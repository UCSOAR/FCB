/**
 ********************************************************************************
 * @file    RadioProtoTask.cpp
 * @author  Shivam Desai
 * @date    May 8, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "RadioProtoTask.hpp"
#include "FlightTask.hpp"
#include "ReadBufferFixedSize.h"

// TODO NEW
//#include "FlashTask.hpp"
//#include "WatchdogTask.hpp"
//#include "TelemetryTask.hpp"

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
 * @brief Initialize the RadioProtocolTask
 */
void RadioProtocolTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize RadioProtocolTask twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)RadioProtocolTask::RunTask,
            (const char*)"ProtocolTask",
            (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_RADIO_PROTOCOL_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "ProtocolTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Default constructor
 */
RadioProtocolTask::RadioProtocolTask() : ProtocolTask(
        Proto::Node::NODE_FCB,
        UART::Radio,
        UART_TASK_COMMAND_SEND_RADIO)
{
}

/**
 * @brief Handle a command message
 */
void RadioProtocolTask::HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::CommandMessage msg;
    msg.deserialize(readBuffer);

    // Verify the source and target nodes, if they aren't as expected, do nothing
    if (msg.get_source() != Proto::Node::NODE_FSB || msg.get_target() != Proto::Node::NODE_FCB)
        return;

    // If the message does not have a FCB command, do nothing
    if (!msg.has_fcb_command())
        return;

    // SOAR_PRINT("PROTO-INFO: Received FCB Command Message\n");

    // Process the db command
    switch (msg.get_fcb_command().get_command_enum())
    {
    case Proto::FcbCommand::Command::RSC_ANY_TO_ABORT:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_ANY_TO_ABORT));
        break;
    case Proto::FcbCommand::Command::RSC_OPEN_VENT:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_OPEN_VENT));
        break;
    case Proto::FcbCommand::Command::RSC_CLOSE_VENT:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_CLOSE_VENT));
        break;
    case Proto::FcbCommand::Command::RSC_GOTO_FILL:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_GOTO_FILL));
        break;
    case Proto::FcbCommand::Command::RSC_ARM_CONFIRM_1:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_ARM_CONFIRM_1));
        break;
    case Proto::FcbCommand::Command::RSC_ARM_CONFIRM_2:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_ARM_CONFIRM_2));
        break;
    case Proto::FcbCommand::Command::RSC_GOTO_ARM:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_GOTO_ARM));
        break;
    case Proto::FcbCommand::Command::RSC_GOTO_PRELAUNCH:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_GOTO_PRELAUNCH));
        break;
    case Proto::FcbCommand::Command::RSC_IGNITION_TO_LAUNCH: // This is the ignition confirmation (we need a button to send this)
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_IGNITION_TO_LAUNCH));
        break;
    case Proto::FcbCommand::Command::RSC_GOTO_TEST:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_GOTO_TEST));
        break;
    default:
        break;
    }

}

/**
 * @brief Handle a control message
 */
void RadioProtocolTask::HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::ControlMessage msg;
    msg.deserialize(readBuffer);

    // Verify the source and target nodes, if they aren't as expected, do nothing
    if (msg.get_source() != Proto::Node::NODE_FSB || msg.get_target() != Proto::Node::NODE_FCB)
        return;

    // Handle based on the message type
    if(msg.has_hb()) {
        // This is a heartbeat message, update the heartbeat
        // SOAR_PRINT("PROTO-INFO: Received Heartbeat Message\n");
    	// TODO NEW
//        WatchdogTask::Inst().SendCommand(Command(HEARTBEAT_COMMAND, (uint16_t)RADIOHB_REQUEST));
    }
    else if(msg.has_ping()) {
        // This is a ping message, respond with an ack
        Proto::ControlMessage ackResponse;
        Proto::AckNack ack;
        ack.set_acking_msg_source(msg.get_source());
        ack.set_acking_sequence_num(msg.get_source_sequence_num());
        ackResponse.set_ack(ack);
        EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuf;
        ackResponse.serialize(writeBuf);
        RadioProtocolTask::SendProtobufMessage(writeBuf, Proto::MessageID::MSG_CONTROL);
    }
    else if(msg.has_sys_ctrl()) {
		// This is a system command, handle it
	    if (msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_FLASH_LOG_ENABLE)
	    {
            // TODO
	    }
        else if(msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_FLASH_LOG_DISABLE)
        {
            // TODO
        }
        else if(msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_RESET)
        {
			// This is a request to reset the system
            SOAR_ASSERT(false, "System reset requested!");
        }
        else if (msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_CRITICAL_FLASH_FULL_ERASE)
        {
            // This is a request that will erase all flash memory, and cause the flash task to stall!
        	// TODO NEW
//        	FlashTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, ERASE_ALL_FLASH));
        }
        else if(msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_LOG_PERIOD_CHANGE)
        {
            uint32_t paramMs = msg.get_sys_ctrl().get_cmd_param();
            paramMs = (paramMs > 0xFFFF) ? 0xFFFE : paramMs;
            // TODO NEW
//          TelemetryTask::Inst().SendCommand(Command(TELEMETRY_CHANGE_PERIOD, paramMs));
        }
    }
}

/**
 * @brief Handle a telemetry message
 */
void RadioProtocolTask::HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{

}
