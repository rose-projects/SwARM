[![Build Status](https://travis-ci.org/rose-projects/SwARM.svg?branch=moves)](https://travis-ci.org/rose-projects/SwARM)

# SwARM
#### Swaggy Artistic Robots Matrix.
[Hackster.io](https://www.hackster.io/perceval/swarm-c362dd)

## ChibiOS stable for Olimex STM32 P407 & E407 rev.E

## About this branch

This branch contains ChibiOS configuration for Olimex STM32 P407 

### What does it do ?

The main allows to test the coding wheels connected to the board. The output of
the comparator used in the coding wheels is connected to GPIOA0 TIM5_IC1

## How to clone this branch

```
git clone --recursive git@github.com:rose-projects/SwARM.git --branch moves
git submodule update --init ChibiOS
```

You can go in ```src``` and compile using ```make``` :)


#### ChibiOS is added in the repository as a submodule

to add ChibiOS as a submodule in the current branch and the current directory :
```
git submodule add --branch stable_16.1.x git@github.com:ChibiOS/ChibiOS.git
```

If you want to pull the submodule in an already cloned repository of SwARM :
```
git checkout moves
git pull
git submodule update --init ChibiOS
```

for more information you can checkout our [wiki page on submodule](https://github.com/rose-projects/SwARM/wiki/GitHelp)
