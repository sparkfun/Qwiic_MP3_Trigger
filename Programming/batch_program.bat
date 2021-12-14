@echo Programming the SparkFun Qwiic MP3 Trigger.
@pause
:loop

@echo -
@echo Flashing bootloader...
rem The fuse bits are *extremely* important to get correct. Be careful.
rem We program these bits separate from flash so that ATtiny is running at 8MHz internal for max flashing programming speed
rem These were found by using ATtiny core and 'Programming Bootloader' and looking at verbose Arduino IDE output
rem This sets the Brown Out Detect to 2.7V
@avrdude -C avrdude.conf -pattiny84 -cusbtiny -e -Uefuse:w:0xFF:m -Uhfuse:w:0b11010101:m -Ulfuse:w:0xE2:m

@timeout 1

@echo -
@echo Flashing firmware...

rem The -B1 option reduces the bitclock period (1us = 1MHz SPI), decreasing programming time
rem May increase verification errors

@echo Flashing bootloader and  firmware...
@avrdude -C avrdude.conf -pattiny84 -cusbtiny -e -Uflash:w:Qwiic_MP3_Trigger.ino.hex:i

@echo Done programming! Move on to the next board.
@pause

goto loop