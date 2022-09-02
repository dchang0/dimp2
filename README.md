# dimp2
DIMP (Desulfator In My Pocket) 2 integrated voltmeter's firmware for ATMEGA48V-10PU AVR

dimp2.hex used to work with the older versions of avrdude. It does not work with current versions of avrdude. dimp2.c is the source code used to build dimp2.hex.

dimp2.ino_atmega48_1000000L.hex was built with Arduino IDE 1.8.19 with MCUDude's MiniCore 2.1.3 board library and successfully uploaded to the ATMega48V-10PU using the built-in avrdude 6.3. dimp2.ino is the source code used to build dimp2.ino_atmega48_1000000L.hex. There are only minor differences between dimp2.ino and dimp2.c with no change in functionality.
