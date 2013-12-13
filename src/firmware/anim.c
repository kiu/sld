#include "global.h"
#include "anim.h"

#include <avr/io.h>
#include <string.h>

enum MODE {MODE_MANUAL, MODE_ANIM};
volatile enum MODE mode = MODE_ANIM;

enum ANIM {ANIM_SNAKE, ANIM_COUNT, ANIM_FADE};
volatile enum ANIM anim = ANIM_SNAKE;

const uint8_t LEDCOUNT = 13;
volatile uint8_t pwm_cur[13];
volatile uint8_t pwm_nxt[13];

// -------------------------------------------------------

void sendSPI(uint16_t data) {

        cbi(PORTB, PB2);  // Latch

        SPDR = (uint8_t) ((data & 0xFF00) >> 8);
        while (!(SPSR & (1 << SPIF)));
        SPDR; // Clear SPIF

        SPDR = (uint8_t) (data & 0x00FF);
        while (!(SPSR & (1 << SPIF)));
        SPDR; // Clear SPIF

        sbi(PORTB, PB2);  // Latch
}

// -------------------------------------------------------

volatile uint8_t tock = 0;
volatile uint16_t leds = 0;

void tickPWM() {
    uint8_t i;
    if (tock == 0) {
	for (i = 0; i < LEDCOUNT; i++) {
	    pwm_cur[i] = pwm_nxt[i];
	}
	leds = 0xFFFF;
    }

    for (i = 0; i < LEDCOUNT; i++) {
	if (pwm_cur[i] == tock) {
	    leds &= ~(1 << i);
	}
    }

    tock++;
    sendSPI(leds);
}

// -------------------------------------------------------

const uint8_t snakepwm[13] = {15, 8, 8, 5, 5, 1, 1, 00, 00, 00, 00, 00, 00};
uint8_t snakeidx = 0;
void snake() {
    int i;
    for (i = 0; i < LEDCOUNT; i++) {
	uint8_t target = snakeidx + i;
	if (target > LEDCOUNT) {
	    target = target - (LEDCOUNT + 1);
	}
        pwm_nxt[i] = snakepwm[target];
    }

    snakeidx++;
    if (snakeidx > LEDCOUNT) {
        snakeidx = 0;
    }
}


uint8_t fadeidx = 0;
uint8_t fadedir = 0;
void fade() {
    int i;
    for (i = 0; i < LEDCOUNT; i++) {
        pwm_nxt[i] = fadeidx;
    }
    if (fadedir == 0) {
	fadeidx++;
	if (fadeidx == 0x2F) {
	    fadedir = 1;
	}
    } else {
	fadeidx--;
	if (fadeidx == 0x00) {
	    fadedir = 0;
	}
    }
}


uint16_t countidx = 0;
void count() {
    int i;
    for (i = 0; i < LEDCOUNT; i++) {
	if (countidx & (1 << i)) {
	    pwm_nxt[i] = 0x03;
	} else {
	    pwm_nxt[i] = 0x00;
	}
    }

    countidx++;
    if (countidx > 8191) {
	countidx = 0;
    }
}

// -------------------------------------------------------

volatile uint16_t cycle = 0;

void tickAnim() {
    cycle++;

    switch (anim) {
	case ANIM_SNAKE:
	    if (cycle == 0xFFF) {
		cycle = 0;
		snake();
	    }
	    break;

	case ANIM_FADE:
	    if (cycle == 0x1FF) {
		cycle = 0;
		fade();
	    }
	    break;

	case ANIM_COUNT:
	    if (cycle == 0xFFF) {
		cycle = 0;
		count();
	    }
	    break;
    }

}

// -------------------------------------------------------

void animTick() {
    tickPWM();

    if (mode == MODE_ANIM) {
	tickAnim();
    }
}

void animSetPWM(uint8_t data[]) {
    mode = MODE_MANUAL;

    int i;
    for (i = 0; i < LEDCOUNT; i++) {
        pwm_nxt[i] = data[i];
    }
}

void animNext() {
    mode = MODE_ANIM;

    int i;
    for (i = 0; i < LEDCOUNT; i++) {
        pwm_nxt[i] = 0x00;
    }

    switch (anim) {
	case ANIM_SNAKE:
	    anim = ANIM_FADE;
	    fadeidx = 0;
	    fadedir = 0;
	    break;

	case ANIM_FADE:
	    anim = ANIM_COUNT;
	    countidx = 0;
	    break;

	case ANIM_COUNT:
	    anim = ANIM_SNAKE;
	    snakeidx = 0;
	    break;

	default:
	    anim = ANIM_SNAKE;
	    snakeidx = 0;
	    break;
    }

}
