/**
 ********************************************************************************
 * @file    WatchdogTask.hpp
 * @author  Shivam Desai
 * @date    May 9, 2026
 * @brief
 ********************************************************************************
 */

#ifndef WATCHDOGTASK_HPP_
#define WATCHDOGTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "RocketStateMachine.hpp"
#include "Timer.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
enum HEARTBEAT_COMMANDS  {
    RADIOHB_NONE = 0,
    RADIOHB_REQUEST,        // Heartbeat countdown timer is reset when HEARTBEAT_COMMAND is sent
    HB_STATUS_SEND,         // Sends the status of the heartbeat
    RADIOHB_DISABLED
};

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class WatchdogTask : public Task
{
public:
    static WatchdogTask& Inst() {
        static WatchdogTask inst;
        return inst;
    }

    void InitTask();


protected:
    static void RunTask(void* pvParams) { WatchdogTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code

    void SendHeartbeatStatus();

    static void HeartbeatFailureCallback(TimerHandle_t rtTimerHandle);    // Callback for timer which aborts system in case of data ghosting
    void HandleCommand(Command& cm);
    void HandleHeartbeat(uint16_t taskCommand);                        // If it receives a heartbeat then it resets the timer
    Timer* heartbeatTimer;

private:
    // Private Functions
    WatchdogTask();        // Private constructor
    WatchdogTask(const WatchdogTask&);                        // Prevent copy-construction
    WatchdogTask& operator=(const WatchdogTask&);            // Prevent assignment
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* WATCHDOGTASK_HPP_ */
