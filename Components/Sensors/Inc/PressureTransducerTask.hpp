/**
 ********************************************************************************
 * @file    PressureTransducerTask.hpp
 * @author  Shivam Desai
 * @date    May 12, 2026
 * @brief
 ********************************************************************************
 */

#ifndef PRESSURETRANSDUCERTASK_HPP_
#define PRESSURETRANSDUCERTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "SensorDataTypes.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
enum PT_TASK_COMMANDS {
    PT_NONE = 0,
    PT_REQUEST_NEW_SAMPLE,// Get a new pressure transducer sample, task will be blocked for polling time
    PT_REQUEST_TRANSMIT,    // Send the current pressure transducer data over the Radio
    PT_REQUEST_DEBUG,        // Send the current pressure transducer data over the Debug UART
	PT_SET_FLASH_RATE
};

/************************************
 * TYPEDEFS
 ************************************/


//ADC Handles
extern ADC_HandleTypeDef hadc1;      // ADC1 - PT1
extern ADC_HandleTypeDef hadc2;      // ADC2 - PT2

/************************************
 * CLASS DEFINITIONS
 ************************************/
class PressureTransducerTask : public Task
{
public:
    static PressureTransducerTask& Inst() {
        static PressureTransducerTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { PressureTransducerTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    void Run(void* pvParams);    // Main run code

    void HandleCommand(Command& cm);
    void HandleRequestCommand(uint16_t taskCommand);

    // Sampling
    void SamplePressureTransducer();
    void TransmitProtocolPressureData();

    // Data
    PressureTransducerData* data;
    uint32_t timestampPT;

    uint32_t ticksPerFlashLog = 0;

    struct __attribute__((packed)) PTRambufData {
    	PressureTransducerData data;
    	uint32_t timestamp;
    };

    PTRambufData* bigdump;
    uint32_t bigdumpi;

private:
    PressureTransducerTask();                                        // Private constructor
    PressureTransducerTask(const PressureTransducerTask&);                    // Prevent copy-construction
    PressureTransducerTask& operator=(const PressureTransducerTask&);            // Prevent assignment

    bool ReadADC(ADC_HandleTypeDef* hadc, uint32_t& adcRaw);
    int32_t ConvertADCToPressure_mPSI(uint32_t adcRaw);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* PRESSURETRANSDUCERTASK_HPP_ */
