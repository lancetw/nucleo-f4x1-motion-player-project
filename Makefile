
# MCU name
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -march=armv7e-m -mtune=cortex-m4 -mfloat-abi=softfp -mlittle-endian -mthumb-interwork 

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).
TARGET = stm32_nucleo_f401


# List C source files here. (C dependencies are automatically generated.)
SRC = main.c lcd.c icon.c picojpeg.c picojpeg_disp.c pcf_font.c cfile.c usart.c stm32f4xx_hal_msp.c stm32f4xx_it.c xmodem.c fat.c sd.c HMC5883L.c movie.c sound.c aac.c mp3.c fft.c wm8731.c settings.c lib/Drivers/BSP/STM32F4xx-Nucleo/stm32f4xx_nucleo.c


ASRC = image.s

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
OPT = s

# Debugging format.
DEBUG = #stabs

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = ./lib/Drivers/CMSIS/Include ./lib/Drivers/STM32F4xx_HAL_Driver/Inc ./lib/Drivers/CMSIS/Device/ST/STM32F4xx/Include ./lib/Drivers/BSP/STM32F4xx-Nucleo ./aac ./mp3


# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD =

# Place -D or -U options here
CDEFS = -DBUILD=0x`date '+%Y%m%d'` -DARM -DARM_MATH_CM4 -DSTM32F411xE -DUSE_STM32F4XX_NUCLEO #-DUSE_FULL_ASSERT

# Place -I options here
CINCS =


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
#CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wno-unused-value -Wno-unused-function -Wno-sequence-point -Wno-unused-label -Wno-uninitialized -Wno-unused-variable -Wno-unused-but-set-variable -fno-strict-aliasing#-Wimplicit-function-declaration#-fno-strict-aliasing #-Wall -Wstrict-prototypes
#CFLAGS += -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)


STARTUP = lib/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.o
#STARTUP = lib/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f401xe.o


# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information
#ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 
ASFLAGS = -I ./binaries #-Wa,-gstabs


# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -T STM32F401CE_FLASH.ld
#LDFLAGS = -T stm32_flash.ld 
#LDFLAGS += -Wl,-Map=$(TARGET
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB) $(GCC_LIB) $(patsubst %,-L%,$(DIRLIB)) -lcm4 -lhal -lc -lgcc -ldsp -laac -lmp3

# ---------------------------------------------------------------------------

# Define directories, if needed.
DIRINC = .
#DIRLIB = ./lib/STM32F4xx_StdPeriph_Driver ./lib/CMSIS/ST/STM32F4xx ./lib/CMSIS/DSP_Lib/Source /usr/local/arm/arm-none-eabi/lib/thumb2 /usr/local/arm/lib/gcc/arm-none-eabi/4.4.1/thumb2 ./aac ./mp3
#DIRLIB = lib/Drivers/CMSIS/Device/ST/STM32F4xx lib/Drivers/STM32F4xx_HAL_Driver /usr/local/arm/arm-none-eabi/lib/armv7e-m/softfp  /usr/local/arm/lib/gcc/arm-none-eabi/4.8.3/armv7e-m/softfp
#DIRLIB = lib/Drivers/CMSIS/Device/ST/STM32F4xx lib/Drivers/STM32F4xx_HAL_Driver lib/Drivers/CMSIS/DSP_Lib/Source /usr/local/arm/arm-none-eabi/lib/armv7e-m/softfp /usr/local/arm/lib/gcc/arm-none-eabi/4.8.4/armv7e-m/softfp ./aac ./mp3

DIRLIB = lib/Drivers/CMSIS/Device/ST/STM32F4xx lib/Drivers/STM32F4xx_HAL_Driver lib/Drivers/CMSIS/DSP_Lib/Source /usr/local/arm/arm-none-eabi/lib/thumb/cortex-m4/ /usr/local/arm/lib/gcc/arm-none-eabi/4.6.2/thumb/cortex-m4 ./aac ./mp3
#DIRLIB = ./lib/STM32F4xx_StdPeriph_Driver ./lib/CMSIS/ST/STM32F4xx ./lib/CMSIS/DSP_Lib/Source /usr/local/arm/arm-none-eabi/lib/thumb/cortex-m4/float-abi-hard/fpuv4-sp-d16 /usr/local/arm/lib/gcc/arm-none-eabi/4.6.2/thumb/cortex-m4/float-abi-hard/fpuv4-sp-d16 ./aac ./mp3

# Define programs and commands.
SHELL = sh
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
NM = arm-none-eabi-nm
REMOVE = rm -f
COPY = cp
YACC = bison
LEX = flex

# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.s=.o) $(BINARY:.bin=.o)

# Define all listing files.
LST = $(ASRC:.s=.lst) $(SRC:.c=.lst)

# Compiler flags to generate dependency files.
#GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = $(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = $(MCU) -I. -x assembler-with-cpp $(ASFLAGS)



# Default target.
all: build gccversion sizeafter

build: cm4lib hallib dsplib libaac libmp3 elf hex lss sym 

elf: $(TARGET).elf
hex: $(TARGET).hex
lss: $(TARGET).lss 
sym: $(TARGET).sym


cm4lib:
	$(MAKE) -C lib/Drivers/CMSIS/Device/ST/STM32F4xx

hallib:	lib
	$(MAKE) -C lib/Drivers/STM32F4xx_HAL_Driver

libaac:
	$(MAKE) -C ./aac

libmp3:	
	$(MAKE) -C ./mp3	

dsplib:
	$(MAKE) -C ./lib/Drivers/CMSIS/DSP_Lib/Source

		

# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A -x $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi



# Display compiler version information.
gccversion : 
	$(CC) --version


# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	$(OBJCOPY) -O $(FORMAT) $< $@


# Create extended listing file from ELF output file.
%.lss: %.elf
	$(OBJDUMP) -h -D $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@



# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf : $(OBJ)
	$(LD) $(STARTUP) $(OBJ) -o $@ $(LDFLAGS)


# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.s
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

distclean: clean libclean

# Target: clean project.
clean:
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) .dep/*

libclean:
	$(MAKE) -C lib/Drivers/CMSIS/Device/ST/STM32F4xx clean
	$(MAKE) -C lib/Drivers/STM32F4xx_HAL_Driver clean
	$(MAKE) -C lib/Drivers/CMSIS/DSP_Lib/Source clean
	$(MAKE) -C ./aac clean
	$(MAKE) -C ./mp3 clean

# Include the dependency files.
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all sizebefore sizeafter gccversion \
build elf hex eep lss sym \
clean clean_list program

