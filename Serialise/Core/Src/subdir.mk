################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/ayct102.c \
../Core/Src/build_display.c \
../Core/Src/crc.c \
../Core/Src/crc16.c \
../Core/Src/e2p.c \
../Core/Src/external_io.c \
../Core/Src/fdcan.c \
../Core/Src/flash.c \
../Core/Src/flash_ee.c \
../Core/Src/gpio.c \
../Core/Src/gps.c \
../Core/Src/gps_decode.c \
../Core/Src/hd44780.c \
../Core/Src/i2c.c \
../Core/Src/iwdg.c \
../Core/Src/machine.c \
../Core/Src/main.c \
../Core/Src/modbus_cmd.c \
../Core/Src/modbus_slave.c \
../Core/Src/motors.c \
../Core/Src/protection.c \
../Core/Src/remote.c \
../Core/Src/rtc.c \
../Core/Src/shell.c \
../Core/Src/stm32g0xx_hal_msp.c \
../Core/Src/stm32g0xx_it.c \
../Core/Src/suncalc.c \
../Core/Src/syscalls.c \
../Core/Src/system_stm32g0xx.c \
../Core/Src/tim.c \
../Core/Src/time.c \
../Core/Src/uart_sci.c \
../Core/Src/usart.c \
../Core/Src/vars.c \
../Core/Src/vector.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/ayct102.o \
./Core/Src/build_display.o \
./Core/Src/crc.o \
./Core/Src/crc16.o \
./Core/Src/e2p.o \
./Core/Src/external_io.o \
./Core/Src/fdcan.o \
./Core/Src/flash.o \
./Core/Src/flash_ee.o \
./Core/Src/gpio.o \
./Core/Src/gps.o \
./Core/Src/gps_decode.o \
./Core/Src/hd44780.o \
./Core/Src/i2c.o \
./Core/Src/iwdg.o \
./Core/Src/machine.o \
./Core/Src/main.o \
./Core/Src/modbus_cmd.o \
./Core/Src/modbus_slave.o \
./Core/Src/motors.o \
./Core/Src/protection.o \
./Core/Src/remote.o \
./Core/Src/rtc.o \
./Core/Src/shell.o \
./Core/Src/stm32g0xx_hal_msp.o \
./Core/Src/stm32g0xx_it.o \
./Core/Src/suncalc.o \
./Core/Src/syscalls.o \
./Core/Src/system_stm32g0xx.o \
./Core/Src/tim.o \
./Core/Src/time.o \
./Core/Src/uart_sci.o \
./Core/Src/usart.o \
./Core/Src/vars.o \
./Core/Src/vector.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/ayct102.d \
./Core/Src/build_display.d \
./Core/Src/crc.d \
./Core/Src/crc16.d \
./Core/Src/e2p.d \
./Core/Src/external_io.d \
./Core/Src/fdcan.d \
./Core/Src/flash.d \
./Core/Src/flash_ee.d \
./Core/Src/gpio.d \
./Core/Src/gps.d \
./Core/Src/gps_decode.d \
./Core/Src/hd44780.d \
./Core/Src/i2c.d \
./Core/Src/iwdg.d \
./Core/Src/machine.d \
./Core/Src/main.d \
./Core/Src/modbus_cmd.d \
./Core/Src/modbus_slave.d \
./Core/Src/motors.d \
./Core/Src/protection.d \
./Core/Src/remote.d \
./Core/Src/rtc.d \
./Core/Src/shell.d \
./Core/Src/stm32g0xx_hal_msp.d \
./Core/Src/stm32g0xx_it.d \
./Core/Src/suncalc.d \
./Core/Src/syscalls.d \
./Core/Src/system_stm32g0xx.d \
./Core/Src/tim.d \
./Core/Src/time.d \
./Core/Src/uart_sci.d \
./Core/Src/usart.d \
./Core/Src/vars.d \
./Core/Src/vector.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G0B1xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Sampa -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/ayct102.cyclo ./Core/Src/ayct102.d ./Core/Src/ayct102.o ./Core/Src/ayct102.su ./Core/Src/build_display.cyclo ./Core/Src/build_display.d ./Core/Src/build_display.o ./Core/Src/build_display.su ./Core/Src/crc.cyclo ./Core/Src/crc.d ./Core/Src/crc.o ./Core/Src/crc.su ./Core/Src/crc16.cyclo ./Core/Src/crc16.d ./Core/Src/crc16.o ./Core/Src/crc16.su ./Core/Src/e2p.cyclo ./Core/Src/e2p.d ./Core/Src/e2p.o ./Core/Src/e2p.su ./Core/Src/external_io.cyclo ./Core/Src/external_io.d ./Core/Src/external_io.o ./Core/Src/external_io.su ./Core/Src/fdcan.cyclo ./Core/Src/fdcan.d ./Core/Src/fdcan.o ./Core/Src/fdcan.su ./Core/Src/flash.cyclo ./Core/Src/flash.d ./Core/Src/flash.o ./Core/Src/flash.su ./Core/Src/flash_ee.cyclo ./Core/Src/flash_ee.d ./Core/Src/flash_ee.o ./Core/Src/flash_ee.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/gps.cyclo ./Core/Src/gps.d ./Core/Src/gps.o ./Core/Src/gps.su ./Core/Src/gps_decode.cyclo ./Core/Src/gps_decode.d ./Core/Src/gps_decode.o ./Core/Src/gps_decode.su ./Core/Src/hd44780.cyclo ./Core/Src/hd44780.d ./Core/Src/hd44780.o ./Core/Src/hd44780.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/iwdg.cyclo ./Core/Src/iwdg.d ./Core/Src/iwdg.o ./Core/Src/iwdg.su ./Core/Src/machine.cyclo ./Core/Src/machine.d ./Core/Src/machine.o ./Core/Src/machine.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/modbus_cmd.cyclo ./Core/Src/modbus_cmd.d ./Core/Src/modbus_cmd.o ./Core/Src/modbus_cmd.su ./Core/Src/modbus_slave.cyclo ./Core/Src/modbus_slave.d ./Core/Src/modbus_slave.o ./Core/Src/modbus_slave.su ./Core/Src/motors.cyclo ./Core/Src/motors.d ./Core/Src/motors.o ./Core/Src/motors.su ./Core/Src/protection.cyclo ./Core/Src/protection.d ./Core/Src/protection.o ./Core/Src/protection.su ./Core/Src/remote.cyclo ./Core/Src/remote.d ./Core/Src/remote.o ./Core/Src/remote.su ./Core/Src/rtc.cyclo ./Core/Src/rtc.d ./Core/Src/rtc.o ./Core/Src/rtc.su ./Core/Src/shell.cyclo ./Core/Src/shell.d ./Core/Src/shell.o ./Core/Src/shell.su ./Core/Src/stm32g0xx_hal_msp.cyclo ./Core/Src/stm32g0xx_hal_msp.d ./Core/Src/stm32g0xx_hal_msp.o ./Core/Src/stm32g0xx_hal_msp.su ./Core/Src/stm32g0xx_it.cyclo ./Core/Src/stm32g0xx_it.d ./Core/Src/stm32g0xx_it.o ./Core/Src/stm32g0xx_it.su ./Core/Src/suncalc.cyclo ./Core/Src/suncalc.d ./Core/Src/suncalc.o ./Core/Src/suncalc.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/system_stm32g0xx.cyclo ./Core/Src/system_stm32g0xx.d ./Core/Src/system_stm32g0xx.o ./Core/Src/system_stm32g0xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/time.cyclo ./Core/Src/time.d ./Core/Src/time.o ./Core/Src/time.su ./Core/Src/uart_sci.cyclo ./Core/Src/uart_sci.d ./Core/Src/uart_sci.o ./Core/Src/uart_sci.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su ./Core/Src/vars.cyclo ./Core/Src/vars.d ./Core/Src/vars.o ./Core/Src/vars.su ./Core/Src/vector.cyclo ./Core/Src/vector.d ./Core/Src/vector.o ./Core/Src/vector.su

.PHONY: clean-Core-2f-Src

