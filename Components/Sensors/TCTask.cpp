/*
 * TCTask.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: me
 */


/************************************
 * INCLUDES
 ************************************/
#include "TCTask.hpp"
#include "SystemDefines.hpp"
#include "Command.hpp"

#include "DataBroker.hpp"
#include "Task.hpp"
#include "main.h"
#include "ProtocolTask.hpp"
#include "RadioProtoTask.hpp"
#include "MAX31856_regs.hpp"

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
TCTask::TCTask():Task(TASK_TC_QUEUE_DEPTH_OBJS)
{
    data = (ThermocoupleData*)soar_malloc(sizeof(ThermocoupleData));
}

/**
 * @brief Initialize the TCTask
 *        Do not modify this function aside from adding the task name
 */
void TCTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize TC task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)TCTask::RunTask,
            (const char*)"TCTask",
            (uint16_t)TASK_TC_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_TC_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "TCTask::InitTask() - xTaskCreate() failed");


}

void TCTask::Run(void * pvParams){

	osDelay(100);
	TCDriver1.Init(&hspi2, TC1CS_GPIO_Port, TC1CS_Pin);
	TCDriver2.Init(&hspi2, TC2CS_GPIO_Port, TC2CS_Pin);
	TCDriver3.Init(&hspi2, TC3CS_GPIO_Port, TC3CS_Pin);


	TCDriver1.SetCR1((0b0011<<4) | MAX31856_REG::CR1_TYPE_T); // Pressure Vessel
	TCDriver2.SetCR1((0b0011<<4) | MAX31856_REG::CR1_TYPE_K); // Dip Tube
	TCDriver3.SetCR1((0b0011<<4) | MAX31856_REG::CR1_TYPE_K); // Vent/Heater

	// cont conv mode
	TCDriver1.SetCR0(0b10000000);
	TCDriver2.SetCR0(0b10000000);
	TCDriver3.SetCR0(0b10000000);



    while (1) {
        /* Process commands in blocking mode */

        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){
        	HandleCommand(cm);
        }
//		  For Debugging - Change ReceiveWait to Receive with 1000ms delay
//        SampleTC();
//        SOAR_PRINT("|TC_TASK| \n TC1 (C): %d.%d, \n TC2 (C): %d.%d, \n TC3 (C): %d.%d, \n MCU Timestamp: %u\r\n",
//                		int(data->temp1),abs(int(data->temp1*100-int(data->temp1)*100)),
//        				int(data->temp2),abs(int(data->temp2*100-int(data->temp2)*100)),
//        				int(data->temp3),abs(int(data->temp3*100-int(data->temp3)*100)),
//                		TICKS_TO_MS(xTaskGetTickCount()));
    }

}

void TCTask::HandleCommand(Command& cm){

	switch (cm.GetCommand()) {
	    case REQUEST_COMMAND: {
	        HandleRequestCommand(cm.GetTaskCommand());
	    }
	    case TASK_SPECIFIC_COMMAND: {
	        break;
	    }
	    default:
	        SOAR_PRINT("TCTask - Received Unsupported Command {%d}\n", cm.GetCommand());
	        break;
	    }
	cm.Reset();

	return;

}

/**
 * @brief Handles a Request Command
 * @param taskCommand The command to handle
 */
void TCTask::HandleRequestCommand(uint16_t taskCommand)
{
    //Switch for task specific command within DATA_COMMAND
    switch (taskCommand) {
    case TC_REQUEST_NEW_SAMPLE:
        SampleTC();
        break;
    case TC_REQUEST_TRANSMIT:
    	SampleTC();
    	TransmitProtocolTCData();
        break;
    case TC_REQUEST_DEBUG:

        SOAR_PRINT("|TC_TASK| TC (C): %d.%d, %d.%d, %d.%d, MCU Timestamp: %u\r\n",
        		int(data->temp1),abs(int(data->temp1*100-int(data->temp1)*100)),
				int(data->temp2),abs(int(data->temp2*100-int(data->temp2)*100)),
				int(data->temp3),abs(int(data->temp3*100-int(data->temp3)*100)),
        		TICKS_TO_MS(xTaskGetTickCount()));
        break;
    default:
        SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
        break;
    }
}

void TCTask::SampleTC()
{
	data->temp1 = TCDriver1.ReadThermocoupleTempC();

	data->temp2 = TCDriver2.ReadThermocoupleTempC();

	data->temp3 = TCDriver3.ReadThermocoupleTempC();

}



void TCTask::TransmitProtocolTCData()
{

    Proto::TelemetryMessage msg;
	msg.set_source(Proto::Node::NODE_FCB);
	msg.set_target(Proto::Node::NODE_FSB);
	Proto::FcbTemperature tempData;
//	//      UPPER_PV_TC = 1,
//    VENT_SOLENOID_TC = 2,
//    DIP_TUBE_TC = 3
	tempData.set_upper_pv_tc(data->temp1);
	tempData.set_vent_solenoid_tc(data->temp2);
	tempData.set_dip_tube_tc(data->temp3);

	EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
	msg.serialize(writeBuffer);

    // Send the barometer data
    RadioProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
