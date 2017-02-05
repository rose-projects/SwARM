#ifndef TERMINAL_H
#define TERMINAL_H

#include "usbcfg.h"

#define SERIAL (BaseSequentialStream *) &SDU1

void initSerial(void);

#endif