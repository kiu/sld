// Based on hid-data from Christian Starkjohann
// http://www.obdev.at/products/vusb/index.html

#include "global.h"
#include "anim.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

uint8_t pwm_buf[13];

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x0D,                    //   REPORT_COUNT (13)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};
/* Since we define only one feature report, we don't use report-IDs (which
 * would be the first byte of the report). The entire report consists of 128
 * opaque data bytes.
 */

/* The following variables store the status of the current data transfer */
static uchar    currentAddress;
static uchar    bytesRemaining;

/* ------------------------------------------------------------------------- */

/* usbFunctionRead() is called when the host requests a chunk of data from
 * the device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionRead(uchar *data, uchar len) {
    if(len > bytesRemaining)
        len = bytesRemaining;

    //eeprom_read_block(data, (uchar *)0 + currentAddress, len);
    int i;
    for (i = 0; i < len; i++) {
	*data = pwm_buf[currentAddress + i];
	data++;
    }

    currentAddress += len;
    bytesRemaining -= len;
    return len;
}


/* usbFunctionWrite() is called when the host sends a chunk of data to the
 * device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionWrite(uchar *data, uchar len) {
    if(bytesRemaining == 0) {
        return 1;               /* end of transfer */
    }
    if(len > bytesRemaining)
        len = bytesRemaining;

    //eeprom_write_block(data, (uchar *)0 + currentAddress, len);
    int i;
    for (i = 0; i < len; i++) {
	pwm_buf[currentAddress + i] = *data;
	data++;
    }

    currentAddress += len;
    bytesRemaining -= len;

    if(bytesRemaining == 0) {
	animSetPWM(pwm_buf);
    }

    return bytesRemaining == 0; /* return 1 if this was the last chunk */
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* HID class request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* since we have only one report type, we can ignore the report-ID */
            bytesRemaining = 13;
            currentAddress = 0;
            return USB_NO_MSG;  /* use usbFunctionRead() to obtain data */
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
            /* since we have only one report type, we can ignore the report-ID */
            bytesRemaining = 13;
            currentAddress = 0;
            return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
        }
    }else{
        /* ignore vendor type requests, we don't use any */
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

volatile uint16_t btn = 0;

ISR (INT1_vect) {
    EIMSK &= ~(1 << INT1); // Disable INT1
    btn = 1;
}

void handleButton() {
	if (btn == 0) {
	    return;
	}

	if (btn == 1) {
	    animNext();
	}

	btn++;
	if (btn == 0x2FFF) {
	    btn = 0;
	    EIMSK |= (1 << INT1); // Enable INT1
	}
}

/* ------------------------------------------------------------------------- */

void initHW() {
        // Prepare Ports
        sbi(DDRB, PB1);  // LED Output Enable Out
        sbi(DDRB, PB2);  // LED Latch Out
        sbi(DDRB, PB3);  // LED MOSI Out
        sbi(DDRB, PB5);  // LED SCK Out

        cbi(DDRD, PD3);  // Button In
        sbi(PORTD, PD3);  // Button In Pullup

        // Prepare SPI
        SPCR = (1 << MSTR) | (1 << SPE); // clk/4
        SPSR |= (1 << SPI2X);

        cbi(PORTB, PB1);  // LED Output Enable On

        EIMSK = 1 << INT1; // Enable INT1
}

int main(void) {

    initHW();

    uchar   i;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */

    for(;;){                /* main event loop */
        DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
        usbPoll();
	handleButton();
	animTick();
    }
    return 0;

}

/* ------------------------------------------------------------------------- */
