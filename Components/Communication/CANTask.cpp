/**
 ********************************************************************************
 * @file    CANTask.cpp
 * @author  Shivam Desai
 * @date    Apr 30, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "CANTask.hpp"
#include "SystemDefines.hpp"
#include "RPBLogs.hpp"
#include "DAQLogs.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
/**
 * @brief Constructor for CANTask
 */
CANTask::CANTask() : Task(TASK_CANBUS_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the CANTask
 *        Do not modify this function aside from adding the task name
 */
void CANTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)CANTask::RunTask,
            (const char*)"CANTask",
            (uint16_t)TASK_CANBUS_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_CANBUS_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

			SOAR_ASSERT(rtValue == pdPASS, "CANTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void CANTask::Run(void * pvParams)
{
	while (1) {
		// TODO Check if all board are connected and skip this check if so
		// Check for join requests
		fcbCAN.CheckCANCommands();

		// Receive heartbeat from daughter boards
		if (fcbCAN.GetTicksSinceLastHeartbeat() > 500) {
			fcbCAN.Heartbeat();
		}

		bool isDataAvailable = false;

//		RPB_AIR_BRAKES_COMMAND openAirBrakes = {true};
		DAQ_AIR_BRAKES_COMMAND airBrakesInstruction{false};
		isDataAvailable = fcbCAN.ReadMessageFromDaughterByLogIndex(
				fcbCAN.GetIDOfBoardWithName(CAN_ROCKET_TARGET_DAQ),
				1,
				(uint8_t*)&airBrakesInstruction,
				sizeof(DAQ_AIR_BRAKES_COMMAND)
		);
		if (isDataAvailable) {
//			HandleCommand();
		}

        // Check for commands from other tasks
		Command cm{};
        bool res = qEvtQueue->Receive(cm);
        if(res) {
            HandleCommand(cm);
        }

        osDelay(10);
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void CANTask::HandleCommand(Command& cm)
{
    switch (cm.GetCommand()) {

    default:
        SOAR_PRINT("CANTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}
