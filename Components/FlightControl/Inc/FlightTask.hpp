/**
 ********************************************************************************
 * @file    FlightTask.hpp
 * @author  Shivam Desai
 * @date    Apr 25, 2026
 * @brief	Primary flight task, default task for the system.
 ********************************************************************************
 */

#ifndef FLIGHTTASK_HPP_
#define FLIGHTTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "RocketStateMachine.hpp"
#include "RocketStates.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
// TODO NEW
constexpr uint16_t FLIGHT_TASK_BOOTUP_TELE_CYCLES = 20;

/************************************
 * TYPEDEFS
 ************************************/
enum FlightTaskRequests
{
	FT_REQUEST_NONE = 0,
	FT_REQUEST_TRANSMIT_STATE,	// Send the current state over the Radio
};

/************************************
 * CLASS DEFINITIONS
 ************************************/
class FlightTask : public Task
{
public:
    static FlightTask& Inst() {
        static FlightTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { FlightTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    void Run(void * pvParams); // Main run code

    void HandleCommand(Command& cm);

    void SendRocketState();

private:
    // Private Functions
    FlightTask();        // Private constructor
    FlightTask(const FlightTask&);                        // Prevent copy-construction
    FlightTask& operator=(const FlightTask&);            // Prevent assignment

    // Private Variables
    RocketSM* rsm_;
    uint16_t firstStateSent_;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLIGHTTASK_HPP_ */
