SLDongle (SLD) provides an inexpensive entry into the world of microcontroller development.
http://schoar.de/tinkering/sld

Control LEDs from host:
    1. Plug SLD into USB port
    2. Pipe 13 bytes to /src/usbhost/sldtool

Compile own application:
    - MCU = atmega88p
    - F_CPU = 12000000
    - Max size = 8192 bytes (flash) - 2048 bytes (bootloader) = 6144 bytes

Flash own application:
    1. Hold button
    2. Plug SLD into USB port
    3. Wait until host has enumarated the emulated USBasp
    4. Release button
    5. Flash your hex file (avrdude -p atmega88p -c usbasp -U flash:w:my.hex)

Directory structure:
    /pcb		Schematic and board as Eagle CAD files
    /pcb/img		Schematic and board as PNG files
    /pcb/gerber		Board as Gerber files

    /src/bootloader	Bootloader running on the device
    /src/contrib	Third party contributions
    /src/firmware	Main app running on the device
    /src/helper		Helper scripts for mass flashing
    /src/simple		A simple example app
    /src/usbhost	Host tool to control LEDs

