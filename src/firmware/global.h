#ifndef GLOBAL_H
#define GLOBAL_H

#define cbi(sfr, bit)     (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit)     (_SFR_BYTE(sfr) |= _BV(bit))

#endif
