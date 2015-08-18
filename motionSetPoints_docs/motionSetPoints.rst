*****************
Motion Set Points
*****************

The motionSetPoints module uses a text file to map set point names to positions for one or two motors.

-----------
File Format
-----------
The file consists of two or three space separated columns:

#. Position name
#. Coordinate for first motor
#. Coordinate for second motor (optional)

Blank lines and lines starting with a # are ignored.
All the remaining lines must have two columns or they must all have three.

If there are two columns the one motor will be control. If there are three columns then two motors will be controlled.

-----------
Configuring
-----------

The following configures a lookup for two motors::

	epicsEnvSet "LOOKUPFILE3" "$(ICPCONFIGROOT)/motionSetPoints/sample.txt"
	motionSetPointsConfigure("LOOKUPFILE3","LOOKUPFILE3"
	dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPoints.db",
		"P=$(MYPVPREFIX)LKUP:SAMPLE:,TARGET_PV1=$(MYPVPREFIX)MOT:STACK:Y:MTR,
		TARGET_RBV1=$(MYPVPREFIX)MOT:STACK:Y:MTR.RBV,
		TARGET_DONE=$(MYPVPREFIX)MOT:STACK:Y:MTR.DMOV,
		TARGET_PV2=$(MYPVPREFIX)MOT:STACK:ZLO:MTR,
		TARGET_RBV2=$(MYPVPREFIX)MOT:STACK:ZLO:MTR.RBV,
		TARGET_DONE2=$(MYPVPREFIX)MOT:STACK:ZLO:MTR.DMOV,
		TOL=1,LOOKUP=LOOKUPFILE3")

**motionSetPointsConfigure** takes two arguments, a unique identifier for this lookup and the name of the text file.

The macros have the following meanings:

* **P** - The prefix for the PVs created by this lookup
* **TARGET_PV1** - The setpoint for the first motor (typically the .VAL field of a motor record)
* **TARGET_RBV1** - The current coordinate for the first motor (typically the .RBV field of a motor record)
* **TARGET_DONE** - The 'done' indicator for the first motor (typically the .DMOV field of a motor record)
* **TARGET_PV2** - As TARGET_PV1 but for the second motor (optional)
* **TARGET_RBV2** - As TARGET_RBV1 but for the second motor (optional)
* **TARGET_DONE2** - As TARGET_DONE but for the second motor (optional)
* **TOL** - Tolerance used in converting motor positions to named setpoints
* **LOOKUP** - The unique identifier for this lookup

The macros ending in '2' are required if the lookup file has three columns.

---
PVs
---
The following PVs control the lookup:

* **$(P)POSN:SP** - Set to the name of a position to drive there.
* **$(P)RESET** - Putting to this PV causes the lookup file to be reloaded

The following report what is going on:

* **$(P)POSN** - The name of the current position. If not at an exact position, it will show the nearest.
* **$(P)POSN:SP:RBV** - The name of the target position 
* **$(P)COORD1** - The target position for the first motor
* **$(P)COORD1:RBV** - The current position of the first motor
* **$(P)COORD2** - The target position for the second motor
* **$(P)COORD2:RBV** - The current position of the second motor
* **$(P)STATIONARY** - Whether the first motor is in position
* **$(P)STATIONARY2** - Whether the second motor is in position
* **$(P)POSITIONED** - Whether both motors are in position
