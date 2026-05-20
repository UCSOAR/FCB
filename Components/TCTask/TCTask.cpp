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

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/
extern SPI_HandleTypeDef hspi2;
/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
TCTask::TCTask():Task(TASK_TC_QUEUE_DEPTH_OBJS)
{

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
            (uint16_t)TASK_TC_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_TC_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "TCTask::InitTask() - xTaskCreate() failed");


}

void TCTask::Run(void * pvParams){

	TCDriver1.Init(&hspi2, TC1CS_GPIO_Port, TC1CS_Pin);
	TCDriver2.Init(&hspi2, TC2CS_GPIO_Port, TC2CS_Pin);
	TCDriver3.Init(&hspi2, TC3CS_GPIO_Port, TC3CS_Pin);

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

        osDelay(1000);

		float t1 = TCDriver1.ReadThermocoupleTempC();
		float t2 = TCDriver2.ReadThermocoupleTempC();
		float t3 = TCDriver3.ReadThermocoupleTempC();

    }

}

void TCTask::HandleCommand(Command& cm){

	switch(cm.GetTaskCommand()) {

	}
	cm.Reset();

	return;

}


