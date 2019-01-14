HEXFILE=gamepad2dendy_i8Mhz.hex
#HEXFILE=gamepad2dendy_e16Mhz.hex

USBASP = avrdude -p m8 -P usb -c usbasp -B 10
COMPILE = avr-gcc -Wall -O3 -g -Werror -mmcu=atmega8 -DF_CPU=8000000L #-DDEBUG_LEVEL=1
#COMPILE = avr-gcc -Wall -O3 -g -Werror -mmcu=atmega8 -DF_CPU=16000000L #-DDEBUG_LEVEL=1
COMMON_OBJS = main.o

#---------------------------------------------------------------------------------------
OBJECTS = main.o

# symbolic targets:
all:	$(HEXFILE)

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@


clean:
	rm -f $(HEXFILE) main.lst main.obj main.cof main.list main.map main.eep.hex main.bin *.o main.s

# file targets:
main.bin:	$(COMMON_OBJS)
	$(COMPILE) -o main.bin $(OBJECTS) -Wl,-Map=main.map

$(HEXFILE):	main.bin
	rm -f $(HEXFILE) main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin $(HEXFILE)
	./checksize main.bin
