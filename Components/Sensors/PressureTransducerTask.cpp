/**
 ********************************************************************************
 * @file    PressureTransducerTask.cpp
 * @author  Shivam Desai
 * @date    May 12, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "PressureTransducerTask.hpp"
#include "main.h"
#include "DebugTask.hpp"
#include "Task.hpp"
#include <time.h>
#include "RadioProtoTask.hpp"
#include "CubeUtils.hpp"
#include "ActualLoggingTask.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
#define MIN(a,b) (a > b ? b : a)

/************************************
 * VARIABLES
 ************************************/
static constexpr uint32_t PT_ADC_POLL_TIMEOUT_MS = 50;
// TODO NEW JUST EXPERIMENT SHOULD BE 3.3V
static constexpr double ADC_VREF = 3.293;
static constexpr double ADC_MAX_16BIT = 65535.0;
// Same scale factor from the old F405 code
static constexpr double PRESSURE_SCALE = 1.5220883534136546;

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
#define BIGDUMPSIZE 10000
/**
 * @brief Default constructor, sets and sets up storage for member variables
 */
PressureTransducerTask::PressureTransducerTask() : Task(TASK_PRESSURE_TRANSDUCER_QUEUE_DEPTH_OBJS)
{
	data = (PressureTransducerData*)soar_malloc(sizeof(PressureTransducerData));
	bigdump = (PTRambufData*)soar_malloc(sizeof(PTRambufData)*BIGDUMPSIZE);
}

/**
 * @brief Creates a task for the FreeRTOS Scheduler
 */
void PressureTransducerTask::InitTask()
{
	// Make sure the task is not already initialized
	SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize PT task twice");
	SOAR_ASSERT(bigdump != nullptr,"could not make big dump!");

	// Start the task
	BaseType_t rtValue =
			xTaskCreate((TaskFunction_t)PressureTransducerTask::RunTask,
					(const char*)"PTTask",
					(uint16_t)TASK_PRESSURE_TRANSDUCER_STACK_DEPTH_WORDS,
					(void*)this,
					(UBaseType_t)TASK_PRESSURE_TRANSDUCER_PRIORITY,
					(TaskHandle_t*)&rtTaskHandle);

	//Ensure creation succeded
	SOAR_ASSERT(rtValue == pdPASS, "PressureTransducerTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief PresssureTransducerTask run loop
 * @param pvParams Currently unused task context
 */
void PressureTransducerTask::Run(void * pvParams)
{
	bool reporteddumpstat = false;
	uint32_t last = HAL_GetTick();

	while (1) {
		Command cm;

		//Wait forever for a command
		bool res = qEvtQueue->Receive(cm);

		//Process the command
		if(res)
			HandleCommand(cm);


		osDelay(MIN(ticksPerFlashLog,300));

		uint32_t th = HAL_GetTick();

		if(th - last > ticksPerFlashLog) {
			last = th;
			if(ticksPerFlashLog != 0) {

				SamplePressureTransducer();
				// under 3ms per log go to ram instead
				if(ticksPerFlashLog < 3 && bigdumpi < BIGDUMPSIZE) {
					bigdump[bigdumpi++] = {*data,HAL_GetTick()};
					if(!reporteddumpstat) {
						SOAR_PRINT("we have switched to big dump, starting at %d!\n",bigdumpi);
						reporteddumpstat = true;
					}
				} else {
					DataBroker::Publish<PressureTransducerData>(data);
				}
			}

			// once we're over 3ms per log again (or disabled at 0ms) we can catch up to flash
			if(bigdumpi > 0 && (ticksPerFlashLog >= 3 || ticksPerFlashLog == 0)) {
				// do a few at once so that we're not flashing so slowly once we go back to slower speed
				for(uint16_t i = 0; i < 5; i++) {
					Command cm = {TASK_SPECIFIC_COMMAND,LOG_FROM_RAMBUF};
					cm.CopyDataToCommand((uint8_t*)&bigdump[--bigdumpi], sizeof(bigdump[0]));
					ActualLoggingTask::Inst().SendCommandReference(cm);
					if(bigdumpi == 0) {
						SOAR_PRINT("done catching up!\n");
						break;
					}
				}
				if(reporteddumpstat) {
					SOAR_PRINT("catching up... (%d)\n",bigdumpi);
					reporteddumpstat = false;
				}
			}
		}
	}
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void PressureTransducerTask::HandleCommand(Command& cm)
{
	//TODO: Since this task will stall for a few milliseconds, we may need a way to eat the whole queue (combine similar eg. REQUEST commands and eat to WDG command etc)
	//TODO: Maybe a HandleEvtQueue instead that takes in the whole queue and eats the whole thing in order of non-blocking to blocking

	//Switch for the GLOBAL_COMMAND
	switch (cm.GetCommand()) {
	case REQUEST_COMMAND: {
		HandleRequestCommand(cm.GetTaskCommand());
		break;
	}
	case TASK_SPECIFIC_COMMAND: {
		if(cm.GetTaskCommand() == PT_SET_FLASH_RATE) {
			ticksPerFlashLog = *(uint32_t*)cm.GetDataPointer();
			SOAR_PRINT("pt rate %d\n",ticksPerFlashLog);
		}
		break;
	}
	default:
		SOAR_PRINT("PressureTransducerTASK - Received Unsupported Command {%d}\n", cm.GetCommand());
		break;
	}

	//No matter what we happens, we must reset allocated data
	cm.Reset();
}

/**
 * @brief Handles a Request Command
 * @param taskCommand The command to handle
 */
void PressureTransducerTask::HandleRequestCommand(uint16_t taskCommand)
{
	//Switch for task specific command within DATA_COMMAND
	switch (taskCommand) {
	case PT_REQUEST_NEW_SAMPLE:
		SamplePressureTransducer();
		break;
	case PT_REQUEST_TRANSMIT:
		SamplePressureTransducer();
		TransmitProtocolPressureData();
		break;
	case PT_REQUEST_DEBUG:
		SOAR_PRINT("|PT_TASK| Pressure (PSI): %d.%d, MCU Timestamp: %u\r\n", data->pressure_1 / 1000, data->pressure_1 % 1000,
				TICKS_TO_MS(xTaskGetTickCount()));
		break;
	default:
		SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
		break;
	}
}

/**
 * @brief Read one blocking ADC sample from an already-configured ADC.
 */
bool PressureTransducerTask::ReadADC(ADC_HandleTypeDef* hadc, uint32_t& adcRaw)
{
	if (HAL_ADC_Start(hadc) != HAL_OK)
	{
		HAL_ADC_Stop(hadc);
		return false;
	}

	if (HAL_ADC_PollForConversion(hadc, PT_ADC_POLL_TIMEOUT_MS) != HAL_OK)
	{
		HAL_ADC_Stop(hadc);
		return false;
	}

	adcRaw = HAL_ADC_GetValue(hadc);

	HAL_ADC_Stop(hadc);
	return true;
}

/**
 * @brief Convert raw 16-bit ADC value to pressure in milli-PSI.
 */
int32_t PressureTransducerTask::ConvertADCToPressure_mPSI(uint32_t adcRaw)
{
	// 2.849V
	double adcVoltage = (ADC_VREF / ADC_MAX_16BIT) * static_cast<double>((adcRaw));

	double transducerVoltage = adcVoltage * PRESSURE_SCALE;

	double pressure_mPSI = (250.0 * transducerVoltage - 125.0) * 1000.0;

	return static_cast<int32_t>(pressure_mPSI);
}

/**
 * @brief This function reads and updates pressure readings
 *          from the pressure transducer.
 */
void PressureTransducerTask::SamplePressureTransducer()
{
	static bool adc1Calibrated = false;

	/* Calibrate ADC once before conversions */
	if (!adc1Calibrated)
	{
		HAL_ADC_Stop(&hadc1);

		if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
		{
			Error_Handler();
		}

		adc1Calibrated = true;
		osDelay(10);
	}

	uint32_t adc1Raw = 0;
	if (ReadADC(&hadc1, adc1Raw))
	{
		data->pressure_1 = ConvertADCToPressure_mPSI(adc1Raw);
	}

	// ENABLE IF WE EVER USE A SECOND PT IN UPPER
	//	uint32_t adc2Raw = 0;
	//	if (ReadADC(&hadc2, adc2Raw))
	//	{
	//		data->pressure_2 = ConvertADCToPressure_mPSI(adc2Raw);
	//	}

	//	DataBroker::Publish<PressureTransducerData>(data);
}

/**
 * @brief Transmits a protocol barometer data sample
 */
void PressureTransducerTask::TransmitProtocolPressureData()
{
	//SOAR_PRINT("Pressure Transducer Transmit...\n");

	Proto::TelemetryMessage msg;
	msg.set_source(Proto::Node::NODE_FCB);
	msg.set_target(Proto::Node::NODE_FSB);
	Proto::FcbPressure pressData;
	pressData.set_upper_pv_pressure(data->pressure_1);
	msg.set_fcbPressure(pressData);

	EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
	msg.serialize(writeBuffer);

	// Send the barometer data
	RadioProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
