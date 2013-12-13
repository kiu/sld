// Based on hidtool by Christian Starkjohann
// License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "hiddata.h"
#include "../firmware/usbconfig.h"  /* for device VID, PID, vendor name and product name */

#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <mach/mach_error.h>


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

int maino(int argc, char **argv) {
    printf("sldtool - http://github.com/kiu/sld\n");

    int err;
    usbDevice_t *dev;
    if((dev = openDevice()) == NULL) {
        exit(1);
    }

    do {
	do {
	    err = read(0, buf, sizeof(buf));
	    if (err == EOF || err <= 0) {
		exit(0);
	    }
	} while (err != sizeof(buf));

	printf("Sending:");
	int i = 0;
	for (; i < sizeof(buf); i++) {
	    printf(" %02x", (unsigned char)buf[i]);
	}
	printf("\n");

        if((err = usbhidSetReport(dev, buf, sizeof(buf))) != 0) {   /* add a dummy report ID */
            fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));
	    exit(1);
	}

    } while(1);

    usbhidCloseDevice(dev);

    return 0;
}

int main(int argc, char *argv[])
{
    fprintf(stdout, "cpu_mac\n");
    
    // Open USB device
    usbDevice_t *dev = openDevice();
    if (!dev) {
        fprintf(stderr, "can't open device\n");
        return 1;
    }
    
    // Forever read CPU load and write to USB
    
    // Store previous load info
    host_cpu_load_info_data_t prevInfo;
    int prevValid = 0;
    
    while (1) {
        // Get curent load info
        host_cpu_load_info_data_t info;
        mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
        
        kern_return_t ret = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&info, &count);
        if (ret != KERN_SUCCESS) {
            fprintf(stderr, "error getting statistics: %s\n", mach_error_string(ret));
            break;
        }
                
        if (prevValid) {
            
            // Calculate diff between current and previous load info
            natural_t diffs[CPU_STATE_MAX];
            natural_t total = 0;
            int i;
            for (i = 0; i < CPU_STATE_MAX; ++i) {
                diffs[i] = info.cpu_ticks[i] - prevInfo.cpu_ticks[i];
                total += diffs[i];
            }
            
            // Calculate load
            float cpuLoad = (float)(diffs[CPU_STATE_USER] + diffs[CPU_STATE_SYSTEM]) / (float)(total);
            
            fprintf(stdout, "load: %f%%\n", cpuLoad);

            // Set USB lights dependent on load
            char buf[13];
            for (i = 0; i < 13; ++i) {
                float level = (float)(i + 1) / (float)13;
                if (cpuLoad >= level) {
                    buf[i] = '\x03';
                }
                else {
                    buf[i] = '\x00';
                }
            }
            
            // Write out
            int err = usbhidSetReport(dev, buf, sizeof(buf));
            if (err != 0) {
                fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));
            }
        }

        // Copy current info to previous
        prevInfo = info;
        prevValid = 1;
        
        // Sleep for defined time
        sleep(1);
    }
    
    // Close USB device
    usbhidCloseDevice(dev);
    
    return 0;
}
