#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=UserConsole.c LEDFade.c sht3x.c AutoDuctTestMain.c ValveMotionControl.c FanControl.c DeviceControl.c TimeKeeper.c Config.c BTComCallbacksApp.c RTC_RV3129.c ../Common/BTCom.c ../Common/CircBuffer.c ../Common/Delay.c ../Common/M24512.c ../Common/NVMem.c ../Common/TaskScheduler.c ../Common/uart1.c ../Common/uart2.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/UserConsole.o ${OBJECTDIR}/LEDFade.o ${OBJECTDIR}/sht3x.o ${OBJECTDIR}/AutoDuctTestMain.o ${OBJECTDIR}/ValveMotionControl.o ${OBJECTDIR}/FanControl.o ${OBJECTDIR}/DeviceControl.o ${OBJECTDIR}/TimeKeeper.o ${OBJECTDIR}/Config.o ${OBJECTDIR}/BTComCallbacksApp.o ${OBJECTDIR}/RTC_RV3129.o ${OBJECTDIR}/_ext/2108356922/BTCom.o ${OBJECTDIR}/_ext/2108356922/CircBuffer.o ${OBJECTDIR}/_ext/2108356922/Delay.o ${OBJECTDIR}/_ext/2108356922/M24512.o ${OBJECTDIR}/_ext/2108356922/NVMem.o ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o ${OBJECTDIR}/_ext/2108356922/uart1.o ${OBJECTDIR}/_ext/2108356922/uart2.o
POSSIBLE_DEPFILES=${OBJECTDIR}/UserConsole.o.d ${OBJECTDIR}/LEDFade.o.d ${OBJECTDIR}/sht3x.o.d ${OBJECTDIR}/AutoDuctTestMain.o.d ${OBJECTDIR}/ValveMotionControl.o.d ${OBJECTDIR}/FanControl.o.d ${OBJECTDIR}/DeviceControl.o.d ${OBJECTDIR}/TimeKeeper.o.d ${OBJECTDIR}/Config.o.d ${OBJECTDIR}/BTComCallbacksApp.o.d ${OBJECTDIR}/RTC_RV3129.o.d ${OBJECTDIR}/_ext/2108356922/BTCom.o.d ${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d ${OBJECTDIR}/_ext/2108356922/Delay.o.d ${OBJECTDIR}/_ext/2108356922/M24512.o.d ${OBJECTDIR}/_ext/2108356922/NVMem.o.d ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d ${OBJECTDIR}/_ext/2108356922/uart1.o.d ${OBJECTDIR}/_ext/2108356922/uart2.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/UserConsole.o ${OBJECTDIR}/LEDFade.o ${OBJECTDIR}/sht3x.o ${OBJECTDIR}/AutoDuctTestMain.o ${OBJECTDIR}/ValveMotionControl.o ${OBJECTDIR}/FanControl.o ${OBJECTDIR}/DeviceControl.o ${OBJECTDIR}/TimeKeeper.o ${OBJECTDIR}/Config.o ${OBJECTDIR}/BTComCallbacksApp.o ${OBJECTDIR}/RTC_RV3129.o ${OBJECTDIR}/_ext/2108356922/BTCom.o ${OBJECTDIR}/_ext/2108356922/CircBuffer.o ${OBJECTDIR}/_ext/2108356922/Delay.o ${OBJECTDIR}/_ext/2108356922/M24512.o ${OBJECTDIR}/_ext/2108356922/NVMem.o ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o ${OBJECTDIR}/_ext/2108356922/uart1.o ${OBJECTDIR}/_ext/2108356922/uart2.o

# Source Files
SOURCEFILES=UserConsole.c LEDFade.c sht3x.c AutoDuctTestMain.c ValveMotionControl.c FanControl.c DeviceControl.c TimeKeeper.c Config.c BTComCallbacksApp.c RTC_RV3129.c ../Common/BTCom.c ../Common/CircBuffer.c ../Common/Delay.c ../Common/M24512.c ../Common/NVMem.c ../Common/TaskScheduler.c ../Common/uart1.c ../Common/uart2.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX150F128B
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/UserConsole.o: UserConsole.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/UserConsole.o.d 
	@${RM} ${OBJECTDIR}/UserConsole.o 
	@${FIXDEPS} "${OBJECTDIR}/UserConsole.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/UserConsole.o.d" -o ${OBJECTDIR}/UserConsole.o UserConsole.c  
	
${OBJECTDIR}/LEDFade.o: LEDFade.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LEDFade.o.d 
	@${RM} ${OBJECTDIR}/LEDFade.o 
	@${FIXDEPS} "${OBJECTDIR}/LEDFade.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/LEDFade.o.d" -o ${OBJECTDIR}/LEDFade.o LEDFade.c  
	
${OBJECTDIR}/sht3x.o: sht3x.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/sht3x.o.d 
	@${RM} ${OBJECTDIR}/sht3x.o 
	@${FIXDEPS} "${OBJECTDIR}/sht3x.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/sht3x.o.d" -o ${OBJECTDIR}/sht3x.o sht3x.c  
	
${OBJECTDIR}/AutoDuctTestMain.o: AutoDuctTestMain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/AutoDuctTestMain.o.d 
	@${RM} ${OBJECTDIR}/AutoDuctTestMain.o 
	@${FIXDEPS} "${OBJECTDIR}/AutoDuctTestMain.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/AutoDuctTestMain.o.d" -o ${OBJECTDIR}/AutoDuctTestMain.o AutoDuctTestMain.c  
	
${OBJECTDIR}/ValveMotionControl.o: ValveMotionControl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ValveMotionControl.o.d 
	@${RM} ${OBJECTDIR}/ValveMotionControl.o 
	@${FIXDEPS} "${OBJECTDIR}/ValveMotionControl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/ValveMotionControl.o.d" -o ${OBJECTDIR}/ValveMotionControl.o ValveMotionControl.c  
	
${OBJECTDIR}/FanControl.o: FanControl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FanControl.o.d 
	@${RM} ${OBJECTDIR}/FanControl.o 
	@${FIXDEPS} "${OBJECTDIR}/FanControl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/FanControl.o.d" -o ${OBJECTDIR}/FanControl.o FanControl.c  
	
${OBJECTDIR}/DeviceControl.o: DeviceControl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/DeviceControl.o.d 
	@${RM} ${OBJECTDIR}/DeviceControl.o 
	@${FIXDEPS} "${OBJECTDIR}/DeviceControl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/DeviceControl.o.d" -o ${OBJECTDIR}/DeviceControl.o DeviceControl.c  
	
${OBJECTDIR}/TimeKeeper.o: TimeKeeper.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/TimeKeeper.o.d 
	@${RM} ${OBJECTDIR}/TimeKeeper.o 
	@${FIXDEPS} "${OBJECTDIR}/TimeKeeper.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/TimeKeeper.o.d" -o ${OBJECTDIR}/TimeKeeper.o TimeKeeper.c  
	
${OBJECTDIR}/Config.o: Config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Config.o.d 
	@${RM} ${OBJECTDIR}/Config.o 
	@${FIXDEPS} "${OBJECTDIR}/Config.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/Config.o.d" -o ${OBJECTDIR}/Config.o Config.c  
	
${OBJECTDIR}/BTComCallbacksApp.o: BTComCallbacksApp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/BTComCallbacksApp.o.d 
	@${RM} ${OBJECTDIR}/BTComCallbacksApp.o 
	@${FIXDEPS} "${OBJECTDIR}/BTComCallbacksApp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/BTComCallbacksApp.o.d" -o ${OBJECTDIR}/BTComCallbacksApp.o BTComCallbacksApp.c  
	
${OBJECTDIR}/RTC_RV3129.o: RTC_RV3129.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RTC_RV3129.o.d 
	@${RM} ${OBJECTDIR}/RTC_RV3129.o 
	@${FIXDEPS} "${OBJECTDIR}/RTC_RV3129.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/RTC_RV3129.o.d" -o ${OBJECTDIR}/RTC_RV3129.o RTC_RV3129.c  
	
${OBJECTDIR}/_ext/2108356922/BTCom.o: ../Common/BTCom.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/BTCom.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/BTCom.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/BTCom.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/BTCom.o.d" -o ${OBJECTDIR}/_ext/2108356922/BTCom.o ../Common/BTCom.c  
	
${OBJECTDIR}/_ext/2108356922/CircBuffer.o: ../Common/CircBuffer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/CircBuffer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d" -o ${OBJECTDIR}/_ext/2108356922/CircBuffer.o ../Common/CircBuffer.c  
	
${OBJECTDIR}/_ext/2108356922/Delay.o: ../Common/Delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/Delay.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/Delay.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/Delay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/Delay.o.d" -o ${OBJECTDIR}/_ext/2108356922/Delay.o ../Common/Delay.c  
	
${OBJECTDIR}/_ext/2108356922/M24512.o: ../Common/M24512.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/M24512.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/M24512.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/M24512.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/M24512.o.d" -o ${OBJECTDIR}/_ext/2108356922/M24512.o ../Common/M24512.c  
	
${OBJECTDIR}/_ext/2108356922/NVMem.o: ../Common/NVMem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/NVMem.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/NVMem.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/NVMem.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/NVMem.o.d" -o ${OBJECTDIR}/_ext/2108356922/NVMem.o ../Common/NVMem.c  
	
${OBJECTDIR}/_ext/2108356922/TaskScheduler.o: ../Common/TaskScheduler.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d" -o ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o ../Common/TaskScheduler.c  
	
${OBJECTDIR}/_ext/2108356922/uart1.o: ../Common/uart1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart1.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/uart1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/uart1.o.d" -o ${OBJECTDIR}/_ext/2108356922/uart1.o ../Common/uart1.c  
	
${OBJECTDIR}/_ext/2108356922/uart2.o: ../Common/uart2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart2.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart2.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/uart2.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/uart2.o.d" -o ${OBJECTDIR}/_ext/2108356922/uart2.o ../Common/uart2.c  
	
else
${OBJECTDIR}/UserConsole.o: UserConsole.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/UserConsole.o.d 
	@${RM} ${OBJECTDIR}/UserConsole.o 
	@${FIXDEPS} "${OBJECTDIR}/UserConsole.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/UserConsole.o.d" -o ${OBJECTDIR}/UserConsole.o UserConsole.c  
	
${OBJECTDIR}/LEDFade.o: LEDFade.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LEDFade.o.d 
	@${RM} ${OBJECTDIR}/LEDFade.o 
	@${FIXDEPS} "${OBJECTDIR}/LEDFade.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/LEDFade.o.d" -o ${OBJECTDIR}/LEDFade.o LEDFade.c  
	
${OBJECTDIR}/sht3x.o: sht3x.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/sht3x.o.d 
	@${RM} ${OBJECTDIR}/sht3x.o 
	@${FIXDEPS} "${OBJECTDIR}/sht3x.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/sht3x.o.d" -o ${OBJECTDIR}/sht3x.o sht3x.c  
	
${OBJECTDIR}/AutoDuctTestMain.o: AutoDuctTestMain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/AutoDuctTestMain.o.d 
	@${RM} ${OBJECTDIR}/AutoDuctTestMain.o 
	@${FIXDEPS} "${OBJECTDIR}/AutoDuctTestMain.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/AutoDuctTestMain.o.d" -o ${OBJECTDIR}/AutoDuctTestMain.o AutoDuctTestMain.c  
	
${OBJECTDIR}/ValveMotionControl.o: ValveMotionControl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ValveMotionControl.o.d 
	@${RM} ${OBJECTDIR}/ValveMotionControl.o 
	@${FIXDEPS} "${OBJECTDIR}/ValveMotionControl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/ValveMotionControl.o.d" -o ${OBJECTDIR}/ValveMotionControl.o ValveMotionControl.c  
	
${OBJECTDIR}/FanControl.o: FanControl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FanControl.o.d 
	@${RM} ${OBJECTDIR}/FanControl.o 
	@${FIXDEPS} "${OBJECTDIR}/FanControl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/FanControl.o.d" -o ${OBJECTDIR}/FanControl.o FanControl.c  
	
${OBJECTDIR}/DeviceControl.o: DeviceControl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/DeviceControl.o.d 
	@${RM} ${OBJECTDIR}/DeviceControl.o 
	@${FIXDEPS} "${OBJECTDIR}/DeviceControl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/DeviceControl.o.d" -o ${OBJECTDIR}/DeviceControl.o DeviceControl.c  
	
${OBJECTDIR}/TimeKeeper.o: TimeKeeper.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/TimeKeeper.o.d 
	@${RM} ${OBJECTDIR}/TimeKeeper.o 
	@${FIXDEPS} "${OBJECTDIR}/TimeKeeper.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/TimeKeeper.o.d" -o ${OBJECTDIR}/TimeKeeper.o TimeKeeper.c  
	
${OBJECTDIR}/Config.o: Config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Config.o.d 
	@${RM} ${OBJECTDIR}/Config.o 
	@${FIXDEPS} "${OBJECTDIR}/Config.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/Config.o.d" -o ${OBJECTDIR}/Config.o Config.c  
	
${OBJECTDIR}/BTComCallbacksApp.o: BTComCallbacksApp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/BTComCallbacksApp.o.d 
	@${RM} ${OBJECTDIR}/BTComCallbacksApp.o 
	@${FIXDEPS} "${OBJECTDIR}/BTComCallbacksApp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/BTComCallbacksApp.o.d" -o ${OBJECTDIR}/BTComCallbacksApp.o BTComCallbacksApp.c  
	
${OBJECTDIR}/RTC_RV3129.o: RTC_RV3129.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RTC_RV3129.o.d 
	@${RM} ${OBJECTDIR}/RTC_RV3129.o 
	@${FIXDEPS} "${OBJECTDIR}/RTC_RV3129.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/RTC_RV3129.o.d" -o ${OBJECTDIR}/RTC_RV3129.o RTC_RV3129.c  
	
${OBJECTDIR}/_ext/2108356922/BTCom.o: ../Common/BTCom.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/BTCom.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/BTCom.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/BTCom.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/BTCom.o.d" -o ${OBJECTDIR}/_ext/2108356922/BTCom.o ../Common/BTCom.c  
	
${OBJECTDIR}/_ext/2108356922/CircBuffer.o: ../Common/CircBuffer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/CircBuffer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/CircBuffer.o.d" -o ${OBJECTDIR}/_ext/2108356922/CircBuffer.o ../Common/CircBuffer.c  
	
${OBJECTDIR}/_ext/2108356922/Delay.o: ../Common/Delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/Delay.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/Delay.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/Delay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/Delay.o.d" -o ${OBJECTDIR}/_ext/2108356922/Delay.o ../Common/Delay.c  
	
${OBJECTDIR}/_ext/2108356922/M24512.o: ../Common/M24512.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/M24512.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/M24512.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/M24512.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/M24512.o.d" -o ${OBJECTDIR}/_ext/2108356922/M24512.o ../Common/M24512.c  
	
${OBJECTDIR}/_ext/2108356922/NVMem.o: ../Common/NVMem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/NVMem.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/NVMem.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/NVMem.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/NVMem.o.d" -o ${OBJECTDIR}/_ext/2108356922/NVMem.o ../Common/NVMem.c  
	
${OBJECTDIR}/_ext/2108356922/TaskScheduler.o: ../Common/TaskScheduler.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/TaskScheduler.o.d" -o ${OBJECTDIR}/_ext/2108356922/TaskScheduler.o ../Common/TaskScheduler.c  
	
${OBJECTDIR}/_ext/2108356922/uart1.o: ../Common/uart1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart1.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/uart1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/uart1.o.d" -o ${OBJECTDIR}/_ext/2108356922/uart1.o ../Common/uart1.c  
	
${OBJECTDIR}/_ext/2108356922/uart2.o: ../Common/uart2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart2.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/uart2.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2108356922/uart2.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"../../../microchip_solutions_v2013-06-15/Microchip/Include" -I"." -I"../Common" -Os -MMD -MF "${OBJECTDIR}/_ext/2108356922/uart2.o.d" -o ${OBJECTDIR}/_ext/2108356922/uart2.o ../Common/uart2.c  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_ICD3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}       -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem 
else
dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}       -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem
	${MP_CC_DIR}\\pic32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/AutoDuctMotionControllerTest.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
