/*
 * AXI_VDMA.h
 *
 *  Created on: Sep 2, 2016
 *      Author: Elod
 */

#ifndef AXI_VDMA_H_
#define AXI_VDMA_H_

#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <functional>
#include <iostream>
#include <xil_types.h>

#include "xaxivdma.h"

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

namespace digilent 
{

/*!
 * \brief Driver class for Xilinx AXI VDMA IP. Needs to have stable clocks before
 * instantiation to be able to complete hardware reset.
 */
template <typename IrptCtl>
class AXI_VDMA
{
	typedef struct {
		/* The state variable to keep track if the initialization is done */
		unsigned int init_done;
		/* The XAxiVdma_DmaSetup structure contains all the necessary information to
		 * start a frame write or read. */
		XAxiVdma_DmaSetup ReadCfg;
		XAxiVdma_DmaSetup WriteCfg;
		/* Horizontal size of frame */
		unsigned int hsize;
		/* Vertical size of frame */
		unsigned int vsize;
		/* Buffer address from where read and write will be done by VDMA */
		unsigned int buffer_address;
		/* Flag to tell VDMA to interrupt on frame completion */
		unsigned int enable_frm_cnt_intr;
		/* The counter to tell VDMA on how many frames the interrupt should happen */
		unsigned int number_of_frame_count;
	} vdma_context_t;
public:
	// Utility function to extract function object from CallbackRef and call it.
	// This should call our member function handlers below.
	template <typename Func>
	static void MyCallback(void* CallbackRef, uint32_t mask_or_type)
	{
		auto pfn = static_cast<Func*>(CallbackRef);
		pfn->operator()(mask_or_type);
	}

	AXI_VDMA(uint32_t dev_id,
             UINTPTR frame_buf_base_addr,
             IrptCtl& irpt_ctl,
             uint32_t rd_irpt_id,
             uint32_t wr_irpt_id
             ) :
		rd_handler_(std::bind(&AXI_VDMA::readHandler, this, std::placeholders::_1)),
		wr_handler_(std::bind(&AXI_VDMA::writeHandler, this, std::placeholders::_1)),
		rd_err_handler_(std::bind(&AXI_VDMA::readErrorHandler, this, std::placeholders::_1)),
		wr_err_handler_(std::bind(&AXI_VDMA::writeErrorHandler, this, std::placeholders::_1)),
		context_{},
		frame_buf_base_addr_(frame_buf_base_addr),
		irpt_ctl_(irpt_ctl),
        prevHVSize({0, 0})
	{
		XAxiVdma_Config* psConf;
		XStatus Status;

		psConf = XAxiVdma_LookupConfig(dev_id);
        
		if (!psConf)
        {
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}

		//Initialize driver instance and reset VDMA
		Status = XAxiVdma_CfgInitialize(
            &drv_inst_, 
            psConf, 
            psConf->BaseAddress
        );

		if (Status != XST_SUCCESS)
        {
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}

		//Set error interrupt error handlers, which for some reason need completion handler defined too
		XAxiVdma_SetCallBack(
            &drv_inst_, 
            XAXIVDMA_HANDLER_GENERAL,
            reinterpret_cast<void*>(&MyCallback<decltype(rd_handler_)>), 
            &rd_handler_, 
            XAXIVDMA_READ
        );
		XAxiVdma_SetCallBack(
            &drv_inst_, 
            XAXIVDMA_HANDLER_GENERAL,
            reinterpret_cast<void*>(&MyCallback<decltype(wr_handler_)>), 
            &wr_handler_, 
            XAXIVDMA_WRITE
        );
		XAxiVdma_SetCallBack(
            &drv_inst_, 
            XAXIVDMA_HANDLER_ERROR,
            reinterpret_cast<void*>(&MyCallback<decltype(rd_err_handler_)>), 
            &rd_err_handler_, 
            XAXIVDMA_READ
        );
		XAxiVdma_SetCallBack(
            &drv_inst_, 
            XAXIVDMA_HANDLER_ERROR,
            reinterpret_cast<void*>(&MyCallback<decltype(wr_err_handler_)>), 
            &wr_err_handler_, 
            XAXIVDMA_WRITE
        );

		//Register the IIC handler with the interrupt controller
		irpt_ctl_.registerHandler(rd_irpt_id, &XAxiVdma_ReadIntrHandler, &drv_inst_);
		irpt_ctl_.enableInterrupt(rd_irpt_id);
		irpt_ctl_.registerHandler(wr_irpt_id, &XAxiVdma_WriteIntrHandler, &drv_inst_);
		irpt_ctl_.enableInterrupt(wr_irpt_id);
		irpt_ctl_.enableInterrupts();
	}

	void resetRead()
	{
        //XAxiVdma_ChannelStop(&drv_inst_.ReadChannel);
        //while (XAxiVdma_ChannelIsRunning(&drv_inst_.ReadChannel));

		XAxiVdma_ChannelReset(&drv_inst_.ReadChannel);

		int Polls = RESET_POLL;

		while (Polls && XAxiVdma_ChannelResetNotDone(&drv_inst_.ReadChannel)) 
        {
			--Polls;
		}

		if (!Polls) 
        {
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}
	}

	void resetWrite()
	{
        //XAxiVdma_ChannelStop(&drv_inst_.WriteChannel);
        //while (XAxiVdma_ChannelIsRunning(&drv_inst_.WriteChannel));

		XAxiVdma_ChannelReset(&drv_inst_.WriteChannel);

		int Polls = RESET_POLL;

		while (Polls && XAxiVdma_ChannelResetNotDone(&drv_inst_.WriteChannel)) 
        {
			--Polls;
		}

		if (!Polls) 
        {
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}
	}

	void configureRead(uint16_t h_res, uint16_t v_res)
	{
		XStatus status;

		initMemAxiVdma(h_res, v_res);

        // Set again h/v attr
        context_.ReadCfg.HoriSizeInput = h_res * drv_inst_.ReadChannel.StreamWidth;
        context_.ReadCfg.VertSizeInput = v_res;
		context_.ReadCfg.Stride = context_.ReadCfg.HoriSizeInput;
        // No delays
		context_.ReadCfg.FrameDelay = 0;
		context_.ReadCfg.EnableCircularBuf = 1;
		context_.ReadCfg.EnableSync = 1;
		context_.ReadCfg.PointNum = 0;
		context_.ReadCfg.EnableFrameCounter = 0;
        // Set it on 0 until we sync
		context_.ReadCfg.FixedFrameStoreAddr = 0;

		status = XAxiVdma_DmaConfig(
            &drv_inst_,
            XAXIVDMA_READ,
            &context_.ReadCfg
        );
		
        if (XST_SUCCESS != status)
		{
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}
		
        status = XAxiVdma_DmaSetBufferAddr(
            &drv_inst_,
            XAXIVDMA_READ,
            context_.ReadCfg.FrameStoreStartAddr
        );

		if (XST_SUCCESS != status)
		{
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}

		//Clear errors in SR
		XAxiVdma_ClearChannelErrors(
            &drv_inst_.ReadChannel,
            XAXIVDMA_SR_ERR_ALL_MASK
        );
		//Enable read channel error and frame count interrupts
		XAxiVdma_IntrEnable(
            &drv_inst_,
            XAXIVDMA_IXR_ERROR_MASK,
            XAXIVDMA_READ
        );
	}

	void enableRead()
	{
		XStatus status;
		//Start read channel
		status = XAxiVdma_DmaStart(
            &drv_inst_,
            XAXIVDMA_READ
        );

		if (XST_SUCCESS != status)
		{
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}
	}

	void configureWrite(uint16_t h_res, uint16_t v_res)
	{
		XStatus status;
        
        initMemAxiVdma(h_res, v_res);
		
        // Set again h/v attr
        context_.WriteCfg.HoriSizeInput = h_res * drv_inst_.ReadChannel.StreamWidth;
        context_.WriteCfg.VertSizeInput = v_res;
        context_.WriteCfg.Stride = context_.WriteCfg.HoriSizeInput;
		context_.WriteCfg.FrameDelay = 1;
		context_.WriteCfg.EnableCircularBuf = 1;
        // Gen-Lock
		context_.WriteCfg.EnableSync = 1;
		context_.WriteCfg.PointNum = 0;
		context_.WriteCfg.EnableFrameCounter = 0;
        // Ignored, since we circle through buffers
		context_.WriteCfg.FixedFrameStoreAddr = 0;

        status = XAxiVdma_ClearDmaChannelErrors(
            &drv_inst_,
            XAXIVDMA_WRITE,
            XAXIVDMA_SR_ERR_ALL_MASK
        );
		status = XAxiVdma_DmaConfig(
            &drv_inst_,
            XAXIVDMA_WRITE,
            &context_.WriteCfg
        );

		if (XST_SUCCESS != status)
		{
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}

		status = XAxiVdma_DmaSetBufferAddr(
            &drv_inst_, 
            XAXIVDMA_WRITE, 
            context_.WriteCfg.FrameStoreStartAddr
        );

		if (XST_SUCCESS != status)
		{
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}
		//Clear errors in SR
		XAxiVdma_ClearChannelErrors(
            &drv_inst_.WriteChannel, 
            XAXIVDMA_SR_ERR_ALL_MASK
        );
		//Unmask error interrupts
		XAxiVdma_MaskS2MMErrIntr(
            &drv_inst_, 
            ~XAXIVDMA_S2MM_IRQ_ERR_ALL_MASK, 
            XAXIVDMA_WRITE
        );
		//Enable write channel error and frame count interrupts
		XAxiVdma_IntrEnable(
            &drv_inst_, 
            XAXIVDMA_IXR_ERROR_MASK, 
            XAXIVDMA_WRITE
        );
	}

	void enableWrite()
	{
		XStatus status;
		//Start read channel
		status = XAxiVdma_DmaStart(
            &drv_inst_, 
            XAXIVDMA_WRITE
        );

		if (XST_SUCCESS != status)
		{
			throw std::runtime_error(__FILE__ ":" LINE_STRING);
		}
	}

	void readHandler(uint32_t irq_types)
	{
		std::cout << "VDMA:read complete - " << irq_types << std::endl;
	}

	void writeHandler(uint32_t irq_types)
	{
		std::cout << "VDMA:write complete - " << irq_types << std::endl;
	}

	void readErrorHandler(uint32_t mask)
	{
		std::cout << "VDMA:read error - " << mask << std::endl;
	}

	void writeErrorHandler(uint32_t mask)
	{
		std::cout << "VDMA:write error - " << mask << std::endl;
	}
	
    void initMemAxiVdma(uint16_t hRes, uint16_t vRes)
    {
        if ((hRes != 0 && vRes != 0) &&
            (prevHVSize.hRes != hRes && prevHVSize.vRes != vRes)
            )
        {
            int iFrm = 0;
            // ReadCfg or WriteCfg, free even if alloc mem
            // has not been done. TODO: avoid freeing before
            // alloc mem.
            prevHVSize.hRes = hRes;
            prevHVSize.vRes = vRes;
            // SteamWidth ~ 3
            //context_.ReadCfg.HoriSizeInput = prevHVSize.hRes * drv_inst_.ReadChannel.StreamWidth;
            //context_.ReadCfg.VertSizeInput = prevHVSize.vRes;
            for (iFrm = 0; iFrm < drv_inst_.MaxNumFrames; iFrm++)
            {
                size_t dimFrame = (prevHVSize.hRes * drv_inst_.ReadChannel.StreamWidth) * prevHVSize.vRes;
                // Max nr of bytes set on 0: 5E EC00 ~ 5MB per frame
                //memset((void*)frame_buf_base_addr_, 0, dimFrame);
                context_.ReadCfg.FrameStoreStartAddr[iFrm] = frame_buf_base_addr_;
                context_.WriteCfg.FrameStoreStartAddr[iFrm] = frame_buf_base_addr_;
                xil_printf("VDMA Frame %d Addr: 0x%08x\r\n", iFrm, frame_buf_base_addr_);
                frame_buf_base_addr_ += dimFrame;
            }
        }
    }
    
    ~AXI_VDMA() {}

private:
	XAxiVdma drv_inst_;
	std::function<void(uint32_t)> rd_handler_;
	std::function<void(uint32_t)> wr_handler_;
	std::function<void(uint32_t)> rd_err_handler_;
	std::function<void(uint32_t)> wr_err_handler_;
	vdma_context_t context_;
	UINTPTR frame_buf_base_addr_;
	IrptCtl& irpt_ctl_;
	int const RESET_POLL = 1000;
    struct {
        uint16_t hRes;
        uint16_t vRes;
    } prevHVSize;
};

} //namespace digilent

#endif /* AXI_VDMA_H_ */