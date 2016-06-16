################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/MCUinit.c" \
"../Sources/analog.c" \
"../Sources/main.c" \
"../Sources/timer.c" \
"../Sources/utils.c" \

C_SRCS += \
../Sources/MCUinit.c \
../Sources/analog.c \
../Sources/main.c \
../Sources/timer.c \
../Sources/utils.c \

OBJS += \
./Sources/MCUinit_c.obj \
./Sources/analog_c.obj \
./Sources/main_c.obj \
./Sources/timer_c.obj \
./Sources/utils_c.obj \

OBJS_QUOTED += \
"./Sources/MCUinit_c.obj" \
"./Sources/analog_c.obj" \
"./Sources/main_c.obj" \
"./Sources/timer_c.obj" \
"./Sources/utils_c.obj" \

C_DEPS += \
./Sources/MCUinit_c.d \
./Sources/analog_c.d \
./Sources/main_c.d \
./Sources/timer_c.d \
./Sources/utils_c.d \

C_DEPS_QUOTED += \
"./Sources/MCUinit_c.d" \
"./Sources/analog_c.d" \
"./Sources/main_c.d" \
"./Sources/timer_c.d" \
"./Sources/utils_c.d" \

OBJS_OS_FORMAT += \
./Sources/MCUinit_c.obj \
./Sources/analog_c.obj \
./Sources/main_c.obj \
./Sources/timer_c.obj \
./Sources/utils_c.obj \


# Each subdirectory must supply rules for building sources it contributes
Sources/MCUinit_c.obj: ../Sources/MCUinit.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: HCS08 Compiler'
	"$(HC08ToolsEnv)/chc08" -ArgFile"Sources/MCUinit.args" -ObjN="Sources/MCUinit_c.obj" "$<" -Lm="$(@:%.obj=%.d)" -LmCfg=xilmou
	@echo 'Finished building: $<'
	@echo ' '

Sources/%.d: ../Sources/%.c
	@echo 'Regenerating dependency file: $@'
	
	@echo ' '

Sources/analog_c.obj: ../Sources/analog.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: HCS08 Compiler'
	"$(HC08ToolsEnv)/chc08" -ArgFile"Sources/analog.args" -ObjN="Sources/analog_c.obj" "$<" -Lm="$(@:%.obj=%.d)" -LmCfg=xilmou
	@echo 'Finished building: $<'
	@echo ' '

Sources/main_c.obj: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: HCS08 Compiler'
	"$(HC08ToolsEnv)/chc08" -ArgFile"Sources/main.args" -ObjN="Sources/main_c.obj" "$<" -Lm="$(@:%.obj=%.d)" -LmCfg=xilmou
	@echo 'Finished building: $<'
	@echo ' '

Sources/timer_c.obj: ../Sources/timer.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: HCS08 Compiler'
	"$(HC08ToolsEnv)/chc08" -ArgFile"Sources/timer.args" -ObjN="Sources/timer_c.obj" "$<" -Lm="$(@:%.obj=%.d)" -LmCfg=xilmou
	@echo 'Finished building: $<'
	@echo ' '

Sources/utils_c.obj: ../Sources/utils.c
	@echo 'Building file: $<'
	@echo 'Executing target #5 $<'
	@echo 'Invoking: HCS08 Compiler'
	"$(HC08ToolsEnv)/chc08" -ArgFile"Sources/utils.args" -ObjN="Sources/utils_c.obj" "$<" -Lm="$(@:%.obj=%.d)" -LmCfg=xilmou
	@echo 'Finished building: $<'
	@echo ' '


