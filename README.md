
# Zybo Z7 Software Repository

## Zybo Z7-10 Pcam 5C Demo

For more information on the Zybo Z7, see its [Resource Center](https://reference.digilentinc.com/reference/programmable-logic/zybo-z7/start) on the Digilent Wiki.

For more information on the Zybo Z7-10 Pcam 5C Demo, including setup instructions, visit its [Demo Page](https://reference.digilentinc.com/reference/programmable-logic/zybo-z7/demos/pcam-5c) on the Digilent Wiki.

For instructions on how to use this repository with git, and for additional documentation on the submodule and branch structures used, please visit [Digilent FPGA Demo Git Repositories](https://reference.digilentinc.com/reference/programmable-logic/documents/git) on the Digilent Wiki. Note that use of git is not required to use this demo. Digilent recommends the use of project releases, for which instructions can be found in the demo wiki page, linked above.

Note: If using git, as this demo requires sources for tools other than Vitis, which are provided in other repos, it is recommended to get these sources through the corresponding branch of the Zybo-Z7 repository, which uses submodules to bring in sources for all tools used by this demo. This is described in the Digilent FPGA Demo Git Repositories page, linked above.

## New Vitis workflow

This functionality can be reproduced from Vitis IDE by launching the terminal from
Terminal -> New Terminal which uses the default command line executable from the OS,
or simply manually invoke the native terminal. If this is the choosen method, then it will be necessary to give absolute path to the `checkout.py`
or `checkin.py` file, not relative, or to change current working directory to the wanted sw submodule branch.

> `vitis -s <path-to-scripts-repo>checkout.py`

or

> `vitis -s <path-to-scripts-repo>checkin.py`
