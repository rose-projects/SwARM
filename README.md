# SwARM [![Build Status](https://travis-ci.org/rose-projects/SwARM.svg?branch=master)](https://travis-ci.org/rose-projects/SwARM)

#### Swaggy Artistic Robots Matrix

## What is it ?

SwARM is an artistic project composed of multiple bots which can roll and light up balloons.
It was designed and created during rose 2017 session @ Télécom ParisTech.

## File description

* ```ChibiOS``` contains [ChibiOS](http://chibios.org/dokuwiki/doku.php)
* ```boards``` contains board descriptions for ChibiOS (SwARM board and our discovery boards)
* ```robot``` contains the code for each robot
* ```beacons``` contains the code for the three beacons (to locate robots)
* ```shared``` contains shared code between robot and beacon (especially radio functions)
* ```choregraph``` contains a web tool to design choreographies and send them to the robots
* ```tests``` contains test code for each feature running on discovery boards

## Quick start

first clone the project
* ```git clone https://github.com/rose-projects/SwARM.git --recursive```

build robot code and flash it using gdb (there are macros at the end of Makefile)
* ```cd robot && make```

build beacon code and flash it using gdb (there are macros at the end of Makefile)
* ```cd ..```
* ```cd beacon && make```

use our tool to generate choreographies : read specific instructions
* ```cd ..```
* ```cd choregraph```
* ```open README.md```

### Quoting of our master
```ça marchera pas```
Samuel Tardieu

