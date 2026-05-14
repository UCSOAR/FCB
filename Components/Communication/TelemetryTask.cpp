/**
 ********************************************************************************
 * @file    ${file_name}
 * @author  ${user}
 * @date    ${date}
 * @brief   This is a template source file to create a new task in our firmware
 * 
 * Setup Steps
 * 1. Define the Task Queue Depth in SystemDefines.hpp
 * 2. Define the Task Stack Depth in SystemDefines.hpp
 * 3. Define the Task Priority in SystemDefines.hpp
 * 4. Replace all placeholders marked with a $ sign
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "TelemetryTask.hpp"
#include "SystemDefines.hpp"
#include "FlightTask.hpp"
#include "RadioProtoTask.hpp"
#include "WatchdogTask.hpp"
#include "GPIO.hpp"
#include "PressureTransducerTask.hpp"

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
 * @brief Constructor for TelemetryTask
 */
TelemetryTask::TelemetryTask() : Task(TELEMETRY_TASK_QUEUE_DEPTH_OBJS)
{
	loggingDelayMs = TELEMETRY_DEFAULT_LOGGING_RATE_MS;
	numNonFlashLogs_ = 0;
	numNonControlLogs_ = 0;
}

/**
 * @brief Initialize the TelemetryTask
 *        Do not modify this function aside from adding the task name
 */
void TelemetryTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize telemetry task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)TelemetryTask::RunTask,
            (const char*)"TelemetryTask",
            (uint16_t)TELEMETRY_TASK_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TELEMETRY_TASK_RTOS_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "TelemetryTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void TelemetryTask::Run(void * pvParams)
{
    while (1) {
        /* Process commands in blocking mode */
    	Command cm;
		while (qEvtQueue->Receive(cm))
			HandleCommand(cm);

        osDelay(2000);
        RunLogSequence();
    }
}

/**
 * @brief Handles a command from the command queue
 * @param cm Command to handle
 */
void TelemetryTask::HandleCommand(Command& cm)
{
    //Switch for the GLOBAL_COMMAND
    switch (cm.GetCommand()) {
    case TELEMETRY_CHANGE_PERIOD: {
        loggingDelayMs = (uint16_t)cm.GetTaskCommand();
    break;
    }
    default:
        SOAR_PRINT("TelemetryTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Runs a full logging sample/send sequence.
 *        can assume this is called with a period of loggingDelayMs
 */
void TelemetryTask::RunLogSequence()
{
    // Flight State
    FlightTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)FT_REQUEST_TRANSMIT_STATE));

    // Heartbeat Status (limited to every 2 seconds)
    if (++numNonControlLogs_ >= (TELEMETRY_HEARTBEAT_TIMER_PERIOD_MS / 4000)) {
        numNonControlLogs_ = 0;
        WatchdogTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, HB_STATUS_SEND));
    }

    // Other Sensors
    RequestSample();
    RequestTransmit();

    SendVentStatus();
}

/**
 * @brief Poll requests to each sensor
 */
void TelemetryTask::RequestSample()
{
}

/**
 * @brief Requests transmit to each sensor
 */
void TelemetryTask::RequestTransmit()
{
	PressureTransducerTask::Inst().SendCommand(Command(REQUEST_COMMAND, PT_REQUEST_TRANSMIT));
}

/**
 * @brief Sends the vent and drain status to the RCU
 */
void TelemetryTask::SendVentStatus()
{
    Proto::TelemetryMessage teleMsg;
    teleMsg.set_source(Proto::Node::NODE_FCB);
    teleMsg.set_target(Proto::Node::NODE_FSB);
    Proto::FcbControlStatus gpioMsg;
    gpioMsg.set_vent_open(GPIO::Vent::IsOpen());
    teleMsg.set_fcbControlStatus(gpioMsg);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    teleMsg.serialize(writeBuffer);

    // Send the control message
    RadioProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
