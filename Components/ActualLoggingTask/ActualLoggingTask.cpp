/*
 * ActualLoggingTask.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: jaddina
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
#include "StateReco.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

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

	osDelay(5);
	MX66L1G45G::Inst().Init();
	osDelay(200);

	uint32_t i = TC_DATA_START_ADDR;
	while(i < TC_DATA_END_ADDR - sizeof(TC_Stored)) {
		TC_Stored stored;
		MX66L1G45G::Inst().ReadData(i, (uint8_t*)&stored, sizeof(stored));
		if(HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check)) == stored.check) {
			// valid
		} else {
			if(stored.check == 0xffffffff && stored.timestamp == 0xffffffff) {
				tcCurrentAddr = i; //safe to continue from here
			} else {
				tcCurrentAddr = (i/4096+1)*4096; //start at next sector
			}
			break;
		}
		if(stored.timestamp > flashTimestampOffset) {
			flashTimestampOffset = stored.timestamp;
		}
		i+=sizeof(stored);
	}

	if((tcCurrentAddr%4096) == 0) {
		MX66L1G45G::Inst().EraseSector(tcCurrentAddr);
	}

	i = PT_DATA_START_ADDR;
	while(i < PT_DATA_END_ADDR - sizeof(PT_Stored)) {
		PT_Stored stored;
		MX66L1G45G::Inst().ReadData(i, (uint8_t*)&stored, sizeof(stored));
		if(HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check)) == stored.check) {
			// valid
		} else {
			if(stored.check == 0xffffffff && stored.timestamp == 0xffffffff) {
				ptCurrentAddr = i; //safe to continue from here
			} else {
				ptCurrentAddr = (i/4096+1)*4096; //start at next sector
			}

			break;
		}
		if(stored.timestamp > flashTimestampOffset) {
			flashTimestampOffset = stored.timestamp;
		}
		i+=sizeof(stored);
	}

	if((ptCurrentAddr%4096) == 0) {
		MX66L1G45G::Inst().EraseSector(ptCurrentAddr);
	}
	SOAR_PRINT("Recovered %d TC logs, starting logging from there\n",tcCurrentAddr/sizeof(TC_Stored));
	SOAR_PRINT("Recovered %d PT logs, starting logging from there\n",(ptCurrentAddr-PT_DATA_START_ADDR)/sizeof(PT_Stored));
	SOAR_PRINT("Also starting from timestamp %lu\n",flashTimestampOffset);

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


	if(cm.GetCommand() == DATA_BROKER_COMMAND) {
		DataBrokerMessageTypes messageType = DataBroker::getMessageType(cm);

		switch(messageType){


		case DataBrokerMessageTypes:: TC_DATA:
		{

			if(tcCurrentAddr >= TC_DATA_END_ADDR-sizeof(TC_Stored)) {
				break;
			}
			TC_Stored stored;
			stored.data = DataBroker::ExtractData<ThermocoupleData>(cm);

			stored.timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS + flashTimestampOffset;

			stored.check = HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check));

			uint32_t lastSecctor = tcCurrentAddr / 4096;
			uint32_t currentSector = (tcCurrentAddr+sizeof(stored)) / 4096;
			if(currentSector > lastSecctor) {
				MX66L1G45G::Inst().EraseSector(currentSector*4096);
			}
			if(MX66L1G45G::Inst().WriteData(tcCurrentAddr, (uint8_t*)&stored, sizeof(stored))) {
				tcCurrentAddr += sizeof(stored);
#ifdef LOGGING_DEBUG
				debugLogged++;
				if(debugLogged > 1000) {
					uint32_t th = HAL_GetTick();
					SOAR_PRINT("log 1000 in %dms\n",th-lastDebugP);
					lastDebugP = th;
					debugLogged = 0;
				}
#endif
			}

			break;
		}

		case DataBrokerMessageTypes:: PT_DATA:
		{

			if(ptCurrentAddr >= PT_DATA_END_ADDR-sizeof(PT_Stored)) {
				break;
			}
			PT_Stored stored;
			stored.data = DataBroker::ExtractData<PressureTransducerData>(cm);

			stored.timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS + flashTimestampOffset;

			stored.check = HAL_CRC_Calculate(&hcrc, (uint32_t*)&stored, sizeof(stored)-sizeof(stored.check));

			uint32_t lastSecctor = ptCurrentAddr / 4096;
			uint32_t currentSector = (ptCurrentAddr+sizeof(stored)) / 4096;
			if(currentSector > lastSecctor) {
				MX66L1G45G::Inst().EraseSector(currentSector*4096);
			}
			if(MX66L1G45G::Inst().WriteData(ptCurrentAddr, (uint8_t*)&stored, sizeof(stored))) {

				ptCurrentAddr += sizeof(stored);
#ifdef LOGGING_DEBUG
				debugLogged++;
				if(debugLogged > 1000) {
					uint32_t th = HAL_GetTick();
					SOAR_PRINT("log 1000 in %dms\n",th-lastDebugP);
					lastDebugP = th;
					debugLogged = 0;
				}
#endif
			}

			break;
		}

		}
	} else if(cm.GetCommand() == TASK_SPECIFIC_COMMAND) {
		switch(cm.GetTaskCommand()) {
		case LOG_STATE_RECO: {
			RocketState state = *(RocketState*)cm.GetDataPointer();
			StateRecoverer::Inst().SaveState(state);
			break;
		}
		case CLEAR_FLASH: {
			SOAR_PRINT("clearing...\n");
			uint32_t i = TC_DATA_START_ADDR;
			while(i < TC_DATA_END_ADDR) {
				uint8_t thisPage[256];
				MX66L1G45G::Inst().ReadData(i, thisPage, sizeof(thisPage));
				bool empty = true;
				for (uint16_t i = 0; i < sizeof(thisPage); i++) {
					if(thisPage[i] != 0xff) {
						empty = false;
						break;
					}
				}
				if(!empty) {
					MX66L1G45G::Inst().EraseSector(i);
					i = (i/4096+1)*4096;
				}
				else {
					i += sizeof(thisPage);
					if((i-TC_DATA_START_ADDR) % 4096 == 0) {
						break;
					}
				}
			}
			SOAR_PRINT("we have destroyed %d sectors of tc\n",(i-TC_DATA_START_ADDR)/4096);
			tcCurrentAddr = TC_DATA_START_ADDR;

			i = PT_DATA_START_ADDR;
			while(i < PT_DATA_END_ADDR) {
				uint8_t thisPage[256];
				MX66L1G45G::Inst().ReadData(i, thisPage, sizeof(thisPage));
				bool empty = true;
				for (uint16_t i = 0; i < sizeof(thisPage); i++) {
					if(thisPage[i] != 0xff) {
						empty = false;
						break;
					}
				}
				if(!empty) {
					MX66L1G45G::Inst().EraseSector(i);
					i = (i/4096+1)*4096;
				}
				else {
					i += sizeof(thisPage);
					if((i-PT_DATA_START_ADDR) % 4096 == 0) {
						break;
					}
				}
			}

			SOAR_PRINT("we have destroyed %d sectors of pt\n",(i-PT_DATA_START_ADDR)/4096);

			ptCurrentAddr = PT_DATA_START_ADDR;

			flashTimestampOffset = 0;
			StateRecoverer::Inst().ClearStates();

			break;
		}

		case DUMP_FLASH: {
			SOAR_PRINT("DUMPING FLASH\n");
			uint32_t i = TC_DATA_START_ADDR;
			uint16_t numInvalidInARow = 0;
			while(i < TC_DATA_END_ADDR) {

				TC_Stored thisTC;
				MX66L1G45G::Inst().ReadData(i, (uint8_t*)&thisTC, sizeof(thisTC));
				if(thisTC.check == HAL_CRC_Calculate(&hcrc, (uint32_t*)&thisTC, sizeof(thisTC)-sizeof(thisTC.check))) {
					numInvalidInARow = 0;
					SOAR_PRINT("TC@%u: %d.%02d, %d.%02d, %d.%02d (%lu)\n",
					        (unsigned int)(i-TC_DATA_START_ADDR)/sizeof(TC_Stored),
					        FPRINT(thisTC.data.temp1),
					        FPRINT(thisTC.data.temp2),
					        FPRINT(thisTC.data.temp3),
					        thisTC.timestamp);
				} else {
					numInvalidInARow++;
				}

				if(numInvalidInARow >= 4096/sizeof(thisTC)) {
					break;
				}
				i += sizeof(thisTC);
			}

			i = PT_DATA_START_ADDR;
			numInvalidInARow = 0;
			while(i < PT_DATA_END_ADDR) {

				PT_Stored thisPT;
				MX66L1G45G::Inst().ReadData(i, (uint8_t*)&thisPT, sizeof(thisPT));
				if(thisPT.check == HAL_CRC_Calculate(&hcrc, (uint32_t*)&thisPT, sizeof(thisPT)-sizeof(thisPT.check))) {
					numInvalidInARow = 0;
					SOAR_PRINT("PT@%u: %d.%02d, (%lu)\n",(unsigned int)(i-PT_DATA_START_ADDR)/sizeof(PT_Stored),
							FPRINT((thisPT.data.pressure_1/1000)), thisPT.timestamp);
				} else {
					numInvalidInARow++;
				}

				if(numInvalidInARow >= 4096/sizeof(thisPT)) {
					break;
				}
				i += sizeof(thisPT);
			}

			SOAR_PRINT("DONE!!!!!!!!!!!!\n");
			break;

		}
		}
	}

	cm.Reset();

	return;

}


