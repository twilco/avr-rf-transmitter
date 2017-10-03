################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../src/util/ \
../test \
../src


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../src/main.c \
../src/util/avr_util.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
src/main.o \
src/util/avr_util.o

OBJS_AS_ARGS +=  \
src/main.o \
src/util/avr_util.o

C_DEPS +=  \
src/main.d \
src/util/avr_util.d

C_DEPS_AS_ARGS +=  \
src/main.d \
src/util/avr_util.d

OUTPUT_FILE_PATH +=avr-rf-transmitter.elf

OUTPUT_FILE_PATH_AS_ARGS +=avr-rf-transmitter.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler





src/%.o: ../src/%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"E:\7.0\Packs\atmel\ATtiny_DFP\1.0.71\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=attiny2313a -B "E:\7.0\Packs\atmel\ATtiny_DFP\1.0.71\gcc\dev\attiny2313a" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/util/%.o: ../src/util/%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"E:\7.0\Packs\atmel\ATtiny_DFP\1.0.71\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=attiny2313a -B "E:\7.0\Packs\atmel\ATtiny_DFP\1.0.71\gcc\dev\attiny2313a" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 4.9.2
	$(QUOTE)E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="avr-rf-transmitter.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=attiny2313a -B "E:\7.0\Packs\atmel\ATtiny_DFP\1.0.71\gcc\dev\attiny2313a"  
	@echo Finished building target: $@
	"E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "avr-rf-transmitter.elf" "avr-rf-transmitter.hex"
	"E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "avr-rf-transmitter.elf" "avr-rf-transmitter.eep" || exit 0
	"E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "avr-rf-transmitter.elf" > "avr-rf-transmitter.lss"
	"E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "avr-rf-transmitter.elf" "avr-rf-transmitter.srec"
	"E:\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "avr-rf-transmitter.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "avr-rf-transmitter.elf" "avr-rf-transmitter.a" "avr-rf-transmitter.hex" "avr-rf-transmitter.lss" "avr-rf-transmitter.eep" "avr-rf-transmitter.map" "avr-rf-transmitter.srec" "avr-rf-transmitter.usersignatures"
	