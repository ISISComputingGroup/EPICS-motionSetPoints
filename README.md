# EPICS-motionSetPoints
The motionSetPoints Repo is located in the EPICS support directory as a submodule: `Instrument\Apps\EPICS\support\motionSetPoints\master\`

Motion set points allow you to label set positions for any number of axes, though currently db files are only created for single, double or 10 axes. 

## WIKI
Please see the WIKI for more information on motion set points:
- https://github.com/ISISComputingGroup/ibex_developers_manual/wiki/Motion-Set-points

For more information on Motors, please see the linked wiki page below:
- https://github.com/ISISComputingGroup/ibex_developers_manual/wiki/Motor-IOCs

## System Tests
The motion set points system tests are located in the [EPICS-IOC_Test_Frameork](https://github.com/ISISComputingGroup/EPICS-IOC_Test_Framework): `EPICS-IOC_Test_Framework\tests\motion_setpoints.py`

To run the system tests, run the following command from and EPICS Terminal (`C:\Instrument\Apps\EPICS\config_env.bat`): `python run_tests.py -t motion_setpoints`.

Use the `-a` flag if you want to run the motionSetPoints emulator for manual testing.
