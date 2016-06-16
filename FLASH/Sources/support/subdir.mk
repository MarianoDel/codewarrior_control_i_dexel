################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../Sources/support/doonstack.asm \

ASM_SRCS_QUOTED += \
"../Sources/support/doonstack.asm" \

OBJS += \
./Sources/support/doonstack_asm.obj \

ASM_DEPS += \
./Sources/support/doonstack_asm.d \

OBJS_QUOTED += \
"./Sources/support/doonstack_asm.obj" \

ASM_DEPS_QUOTED += \
"./Sources/support/doonstack_asm.d" \

OBJS_OS_FORMAT += \
./Sources/support/doonstack_asm.obj \


# Each subdirectory must supply rules for building sources it contributes
Sources/support/doonstack_asm.obj: ../Sources/support/doonstack.asm
	@echo 'Building file: $<'
	@echo 'Executing target #6 $<'
	@echo 'Invoking: HCS08 Assembler'
	"$(HC08ToolsEnv)/ahc08" -ArgFile"Sources/support/doonstack.args" -Objn"Sources/support/doonstack_asm.obj" "$<" -Lm="$(@:%.obj=%.d)" -LmCfg=xilmou
	@echo 'Finished building: $<'
	@echo ' '

Sources/support/%.d: ../Sources/support/%.asm
	@echo 'Regenerating dependency file: $@'
	
	@echo ' '


