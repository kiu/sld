// Based on hidtool by Christian Starkjohann
// License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "hiddata.h"
#include "../firmware/usbconfig.h"  /* for device VID, PID, vendor name and product name */

char buf[13];

static char *usbErrorMessage(int errCode) {
    static char buffer[80];

    switch(errCode){
        case USBOPEN_ERR_ACCESS:      return "Access to device denied";
        case USBOPEN_ERR_NOTFOUND:    return "The device was not found";
        case USBOPEN_ERR_IO:          return "Communication error with device";
        default:
            sprintf(buffer, "Unknown USB error %d", errCode);
            return buffer;
    }
    return NULL;    /* not reached */
}

static usbDevice_t  *openDevice(void) {
    usbDevice_t     *dev = NULL;
    unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
    char            vendorName[] = {USB_CFG_VENDOR_NAME, 0}, productName[] = {USB_CFG_DEVICE_NAME, 0};
    int             vid = rawVid[0] + 256 * rawVid[1];
    int             pid = rawPid[0] + 256 * rawPid[1];
    int             err;

    if((err = usbhidOpenDevice(&dev, vid, vendorName, pid, productName, 0)) != 0){
        fprintf(stderr, "error opening %s: %s\n", productName, usbErrorMessage(err));
        return NULL;
    }
    return dev;
}

int main(int argc, char **argv) {
    printf("sldtool - http://github.com/kiu/sld\n");

    int err;
    int i;

    usbDevice_t *dev;
    if((dev = openDevice()) == NULL) {
        exit(1);
    }

    do {

	do {
	    err = read(0, buf, sizeof(buf));
	    if (err == EOF || err <= 0) {
		usbhidCloseDevice(dev);
		exit(0);
	    }
	} while (err != sizeof(buf));

	printf("Sending:");
	for (i = 0 ; i < sizeof(buf); i++) {
	    printf(" %02x", (unsigned char)buf[i]);
	}
	printf("\n");

        if((err = usbhidSetReport(dev, buf, sizeof(buf))) != 0) {
            fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));
	    usbhidCloseDevice(dev);
	    exit(1);
	}

    } while(1);

    return 0;
}
