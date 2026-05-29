/*
 * ActualLoggingTask.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: jaddina
 */


/**
 ********************************************************************************
 * @file    ActualLoggingTask.cpp
 * @author  jaddina
 * @date    Sep 13, 2025
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "ActualLoggingTask.hpp"
#include "DebugTask.hpp"
#include "SystemDefines.hpp"
#include "Command.hpp"
#include "LoggingService.hpp"

#include "DataBroker.hpp"
#include "Task.hpp"
#include "actualflash.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/


uint8_t ActualLoggingTask::buf[20] = {0};
extern CRC_HandleTypeDef hcrc;


/************************************
 * FUNCTION DEFINITIONS
 ************************************/
ActualLoggingTask::ActualLoggingTask():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS)
{

}



/**
 * @brief Initialize the ActualLoggingTask
 *        Do not modify this function aside from adding the task name
 */
void ActualLoggingTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize actual logging task task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)ActualLoggingTask::RunTask,
            (const char*)"ActualLoggingTask",
            (uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_LOGGING_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "ActualLoggingTask::InitTask() - xTaskCreate() failed");

	DataBroker::Subscribe<ThermocoupleData>(this);
	DataBroker::Subscribe<PressureTransducerData>(this);

}

void ActualLoggingTask::Run(void * pvParams){

	osDelay(50);
	awesomeincredibleflashdriver.Init();
	osDelay(200);

	uint32_t i = TC_DATA_START_ADDR;
	while(i < TC_DATA_END_ADDR - sizeof(TC_Stored)) {
		TC_Stored stored;
		awesomeincredibleflashdriver.ReadData(i, (uint8_t*)&stored, sizeof(stored));
		if(HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check)) == stored.check) {
			// valid
		} else {
			tcCurrentAddr = i;
			break;
		}
		i+=sizeof(stored);
	}
	if(tcCurrentAddr == TC_DATA_START_ADDR) {
		awesomeincredibleflashdriver.EraseSector(TC_DATA_START_ADDR);
	}

	i = PT_DATA_START_ADDR;
	while(i < PT_DATA_END_ADDR - sizeof(PT_Stored)) {
		PT_Stored stored;
		awesomeincredibleflashdriver.ReadData(i, (uint8_t*)&stored, sizeof(stored));
		if(HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check)) == stored.check) {
			// valid
		} else {
			ptCurrentAddr = i;
			break;
		}
		i+=sizeof(stored);
	}
	if(ptCurrentAddr == PT_DATA_START_ADDR) {
		awesomeincredibleflashdriver.EraseSector(PT_DATA_START_ADDR);
	}
	SOAR_PRINT("Recovered %d TC logs, starting logging from there\n",tcCurrentAddr/sizeof(TC_Stored));
	SOAR_PRINT("Recovered %d PT logs, starting logging from there\n",(ptCurrentAddr-PT_DATA_START_ADDR)/sizeof(PT_Stored));

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){

        	HandleCommand(cm);
        }
    }

}

void ActualLoggingTask::HandleCommand(Command& cm){

	// SOAR_PRINT("Data In Logging Task\n");
	if(cm.GetCommand() != DATA_BROKER_COMMAND){
		return;
	}

	DataBrokerMessageTypes messageType = DataBroker::getMessageType(cm);

	switch(messageType){


	case DataBrokerMessageTypes:: TC_DATA:
	{

		if(tcCurrentAddr >= TC_DATA_END_ADDR-sizeof(TC_Stored)) {
			break;
		}
		TC_Stored stored;
		stored.data = DataBroker::ExtractData<ThermocoupleData>(cm);

		stored.timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;

		stored.check = HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check));


		uint32_t lastSecctor = tcCurrentAddr / 4096;
		uint32_t currentSector = (tcCurrentAddr+sizeof(stored)) / 4096;
		if(currentSector > lastSecctor) {
			awesomeincredibleflashdriver.EraseSector(currentSector*4096);
		}
		awesomeincredibleflashdriver.WriteData(tcCurrentAddr, (uint8_t*)&stored, sizeof(stored));
		tcCurrentAddr += sizeof(stored);
		SOAR_PRINT("logged at %d\n",tcCurrentAddr);
		break;
	}

	case DataBrokerMessageTypes:: PT_DATA:
	{

		if(ptCurrentAddr >= PT_DATA_END_ADDR-sizeof(PT_Stored)) {
			break;
		}
		PT_Stored stored;
		stored.data = DataBroker::ExtractData<PressureTransducerData>(cm);

		stored.timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;

		stored.check = HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check));


		uint32_t lastSecctor = ptCurrentAddr / 4096;
		uint32_t currentSector = (ptCurrentAddr+sizeof(stored)) / 4096;
		if(currentSector > lastSecctor) {
			awesomeincredibleflashdriver.EraseSector(currentSector*4096);
		}
		awesomeincredibleflashdriver.WriteData(ptCurrentAddr, (uint8_t*)&stored, sizeof(stored));
		ptCurrentAddr += sizeof(stored);
		SOAR_PRINT("logged pt at %d\n",tcCurrentAddr);
		break;
	}

	}

	cm.Reset();

	return;

}


