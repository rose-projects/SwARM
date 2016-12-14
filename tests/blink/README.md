[![Build Status](https://travis-ci.org/rose-projects/SwARM.svg?branch=chibi-p407)](https://travis-ci.org/rose-projects/SwARM)

# SwARM
#### Swaggy Artistic Robots Matrix.
[Hackster.io](https://www.hackster.io/perceval/swarm-c362dd)

## ChibiOS stable for Olimex STM32 P407 & E407 rev.E

## About this branch

This branch contains ChibiOS configuration for Olimex STM32 P407 and E407 Rev. E
The makefile is configured for the second one. See the table below for differences.

|Variable|Value for P407|Value for E407|File|
|--------|--------------|--------------|----|
|BOARD|../OLIMEX_P407_JTAG|../OLIMEX_E407_REV_E_JTAG|Makefile|
|STM32_PLLM_VALUE|25|12|mcuconf.h|
|LED|6|13U|main.c|
||GPIOF|GPIOC|main.c|

### What does it do ?

the main is a simple green led blinker :)


## How to clone this branch

```
git clone --recursive git@github.com:rose-projects/SwARM.git --branch chibi-p407
git submodule update --init ChibiOS
```

You can go in ```tests/blink/``` and compile using ```make``` :)


#### ChibiOS is added in the repository as a submodule

to add ChibiOS as a submodule in the current branch and the current directory :
```
git submodule add --branch stable_16.1.x git@github.com:ChibiOS/ChibiOS.git
```

If you want to pull the submodule in an already cloned repository of SwARM :
```
git checkout chibi-p407
git pull
git submodule update --init ChibiOS
```

for more information you can checkout our [wiki page on submodule](https://github.com/rose-projects/SwARM/wiki/GitHelp)