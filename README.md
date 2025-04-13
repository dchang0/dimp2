# dimp2
DIMP (Desulfator In My Pocket) 2 integrated voltmeter's firmware for ATMEGA48V-10PU AVR or ATMEGA48-20PU.

dimp2.hex used to work with the older versions of avrdude. It does not work with current versions of avrdude. dimp2.c is the source code used to build dimp2.hex.

dimp2.ino_atmega48_1000000L.hex was built with Arduino IDE 1.8.19 with MCUDude's MiniCore 2.1.3 board library and successfully uploaded to the ATMEGA48V-10PU using the built-in avrdude 6.3. dimp2.ino is the source code used to build dimp2.ino_atmega48_1000000L.hex. There are only minor differences between dimp2.ino and dimp2.c with no change in functionality.

dimp2.ino_atmega48_1000000L.hex was also successfully tested on the ATMEGA48-20PU as-is, without having to recompile it, using teh official avrdude 8.0 for Windows.

The command I used for avrdude 8.0 for Windows was:

.\avrdude.exe -p m48 -C .\avrdude.conf -c arduino_as_isp -P COM4 -v -U flash:w:dimp2.ino_atmega48_1000000L.hex:i

I did not have to edit the avrdude.conf file at all. A Seeeduino Nano (same as Arduino Nano but with USB C port) was used as the USB-to-ISP programmer after it was first prepared by downloading the Arduino IDE example sketch "ArduinoISP" to it. To be used as a USB-to-ISP programmer, Arduino Nano requires a 10uF capacitor placed across the RESET and GND pins.
