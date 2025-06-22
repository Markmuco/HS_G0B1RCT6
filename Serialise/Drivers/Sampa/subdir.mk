################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Sampa/bird.c \
../Drivers/Sampa/sampa.c \
../Drivers/Sampa/sampa_tester.c \
../Drivers/Sampa/spa.c 

OBJS += \
./Drivers/Sampa/bird.o \
./Drivers/Sampa/sampa.o \
./Drivers/Sampa/sampa_tester.o \
./Drivers/Sampa/spa.o 

C_DEPS += \
./Drivers/Sampa/bird.d \
./Drivers/Sampa/sampa.d \
./Drivers/Sampa/sampa_tester.d \
./Drivers/Sampa/spa.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Sampa/%.o Drivers/Sampa/%.su Drivers/Sampa/%.cyclo: ../Drivers/Sampa/%.c Drivers/Sampa/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G0B1xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Sampa -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Sampa

clean-Drivers-2f-Sampa:
	-$(RM) ./Drivers/Sampa/bird.cyclo ./Drivers/Sampa/bird.d ./Drivers/Sampa/bird.o ./Drivers/Sampa/bird.su ./Drivers/Sampa/sampa.cyclo ./Drivers/Sampa/sampa.d ./Drivers/Sampa/sampa.o ./Drivers/Sampa/sampa.su ./Drivers/Sampa/sampa_tester.cyclo ./Drivers/Sampa/sampa_tester.d ./Drivers/Sampa/sampa_tester.o ./Drivers/Sampa/sampa_tester.su ./Drivers/Sampa/spa.cyclo ./Drivers/Sampa/spa.d ./Drivers/Sampa/spa.o ./Drivers/Sampa/spa.su

.PHONY: clean-Drivers-2f-Sampa

