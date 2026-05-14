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

/************************************
 * MACROS AND DEFINES
 ************************************/
enum PT_TASK_COMMANDS {
    PT_NONE = 0,
    PT_REQUEST_NEW_SAMPLE,// Get a new pressure transducer sample, task will be blocked for polling time
    PT_REQUEST_TRANSMIT,    // Send the current pressure transducer data over the Radio
    PT_REQUEST_DEBUG,        // Send the current pressure transducer data over the Debug UART
};

/************************************
 * TYPEDEFS
 ************************************/
typedef struct
{
    int32_t     pressure_1;
} PressureTransducerData;

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

private:
    PressureTransducerTask();                                        // Private constructor
    PressureTransducerTask(const PressureTransducerTask&);                    // Prevent copy-construction
    PressureTransducerTask& operator=(const PressureTransducerTask&);            // Prevent assignment
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* PRESSURETRANSDUCERTASK_HPP_ */
