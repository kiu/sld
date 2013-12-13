# Name: Makefile
# Project: hid-data example
# Author: Christian Starkjohann
# Creation Date: 2008-04-11
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

# Please read the definitions below and edit them as appropriate for your
# system:

# Use the following 3 lines on Unix and Mac OS X:
USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`
EXE_SUFFIX=

# Use the following 3 lines on Windows and comment out the 3 above:
#USBFLAGS=
#USBLIBS=    -lhid -lusb -lsetupapi
#EXE_SUFFIX= .exe

CC=				gcc
CFLAGS=			-O -Wall $(USBFLAGS)
LIBS=			$(USBLIBS)

OBJ=		sldtool.o hiddata.o
PROGRAM=	sldtool$(EXE_SUFFIX)

all: $(PROGRAM) cpu_mac

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

cpu_mac: cpu_mac.o hiddata.o
	$(CC) -o cpu_mac cpu_mac.o hiddata.o $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM) cpu_mac

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o
