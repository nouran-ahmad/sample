################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/alsa.cpp \
../src/program.cpp 

C_SRCS += \
../src/alsa-example.c \
../src/espeak-sample.c \
../src/pulsaudio.c 

OBJS += \
./src/alsa-example.o \
./src/alsa.o \
./src/espeak-sample.o \
./src/program.o \
./src/pulsaudio.o 

CPP_DEPS += \
./src/alsa.d \
./src/program.d 

C_DEPS += \
./src/alsa-example.d \
./src/espeak-sample.d \
./src/pulsaudio.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


