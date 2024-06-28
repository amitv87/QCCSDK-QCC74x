################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../spisync/src/port/spisync_port.c 

OBJS += \
./spisync/src/port/spisync_port.o 

C_DEPS += \
./spisync/src/port/spisync_port.d 


# Each subdirectory must supply rules for building sources it contributes
spisync/src/port/%.o spisync/src/port/%.su spisync/src/port/%.cyclo: ../spisync/src/port/%.c spisync/src/port/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U575xx -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"E:/dev/bouffalo_sdk/bouffalo_sdk_qc/examples/stm32_spi_host/QCC743_SPI_HOST/spisync/include" -I"E:/dev/bouffalo_sdk/bouffalo_sdk_qc/examples/stm32_spi_host/QCC743_SPI_HOST/spisync/src/port" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-spisync-2f-src-2f-port

clean-spisync-2f-src-2f-port:
	-$(RM) ./spisync/src/port/spisync_port.cyclo ./spisync/src/port/spisync_port.d ./spisync/src/port/spisync_port.o ./spisync/src/port/spisync_port.su

.PHONY: clean-spisync-2f-src-2f-port

