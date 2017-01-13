
#ifndef USBCONF_H
#define USBCONF_H

#include "hal.h"
#include "chprintf.h"

#define printf(...) \
	do { \
		if(SDU1.config->usbp->state == USB_ACTIVE) \
			chprintf(USBserial, __VA_ARGS__); \
	} while(0)

extern const USBConfig usbcfg;
extern SerialUSBConfig serusbcfg;
extern SerialUSBDriver SDU1;

/* Stream for chprintf */
extern BaseSequentialStream * USBserial;

/* init serial over USB */
void initUSB(void);

#endif
