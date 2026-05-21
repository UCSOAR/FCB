/**
 ******************************************************************************
 * File Name          : main_system.cpp
 * Description        : This file acts as an interface supporting CubeIDE Codegen
    while having a clean interface for development.
 ******************************************************************************
*/
/* Includes -----------------------------------------------------------------*/
#include "SystemDefines.hpp"
#include "UARTDriver.hpp"

// Tasks
#include "UARTTask.hpp"
#include "CubeTask.hpp"
#include "LoggingTask.hpp"
#include "FlashTask.hpp"
#include "DebugTask.hpp"
#include "FlightTask.hpp"
#include "CANTask.hpp"
#include "WatchdogTask.hpp"
#include "RadioProtoTask.hpp"
#include "TelemetryTask.hpp"
#include "PressureTransducerTask.hpp"

/* Drivers ------------------------------------------------------------------*/
namespace Driver {
    UARTDriver usart1(USART1);
    UARTDriver usart2(USART2);
}

/* Interface Functions ------------------------------------------------------------*/
/**
 * @brief Main function interface, called inside main.cpp before os initialization takes place.
*/
void run_main() {
    // Init Tasks
	WatchdogTask::Inst().InitTask();
	UARTTask::Inst().InitTask();
	CubeTask::Inst().InitTask();
	FlightTask::Inst().InitTask();
	DebugTask::Inst().InitTask();
	CANTask::Inst().InitTask();
	RadioProtocolTask::Inst().InitTask();
	TelemetryTask::Inst().InitTask();
	PressureTransducerTask::Inst().InitTask();

	// PT
	// TC
	// Flash -- State Recovery
	// HDITask
	// Hardware WTDG?
	// Future Work - USB C

	//FlashTask::Inst().InitTask();

    // Print System Boot Info : Warning, don't queue more than 10 prints before scheduler starts
    SOAR_PRINT("\n-- CUBE SYSTEM --\n");
    SOAR_PRINT("System Reset Reason: [TODO]\n"); //TODO: System reset reason can be implemented via. Flash storage
    SOAR_PRINT("Current System Free Heap: %d Bytes\n", xPortGetFreeHeapSize());
//    SOAR_PRINT("Lowest Ever Free Heap: %d Bytes\n\n", xPortGetMinimumEverFreeHeapSize());
    // Start the Scheduler
    // Guidelines:
    // - Be CAREFUL with race conditions after osKernelStart
    // - All uses of new and delete should be closely monitored after this point


    osKernelStart();

    // Should never reach here
    SOAR_ASSERT(false, "osKernelStart() failed");

    while (1)
    {
        osDelay(100);
        HAL_NVIC_SystemReset();
    }
}

