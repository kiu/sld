#!/bin/bash

MCU=atmega88p
AVRDUDE="avrdude -p $MCU -P /dev/ttyUSB0 -c avr911"

case "$1" in

    fuse)
	echo "Setting fuses: 12.0Mhz crystal, disable /8, 2k bootloader enabled"
	$AVRDUDE -U lfuse:w:0xe7:m -U hfuse:w:0xdf:m -U efuse:w:0xf8:m
	exit $?
	;;

    fusenoboot)
	echo "Setting fuses: 12.0Mhz crystal, disable /8"
	$AVRDUDE -U lfuse:w:0xe7:m -U hfuse:w:0xdf:m -U efuse:w:0xf9:m
	exit $?
	;;

    lock)
	echo "Locking bootloader"
	$AVRDUDE -U lock:w:0x2f:m
	exit $?
	;;

    unlock)
	echo "Unlocking bootloader"
	$AVRDUDE -U lock:w:0x3f:m
	exit $?
	;;

    *)
	echo "usage $0 <fuse|fusenoboot|unlock|lock>"
	exit 0
	;;

esac
