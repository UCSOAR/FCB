/**
 ********************************************************************************
 * @file    TelemetryTask.hpp
 * @author  Shivam Desai
 * @date    May 9, 2026
 * @brief   
 * 
 ********************************************************************************
 */

#ifndef TELEMETRYTASK_HPP_
#define TELEMETRYTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
constexpr uint16_t TELEMETRY_HEARTBEAT_TIMER_PERIOD_MS = 2000; // 2s between heartbeat telemetry
constexpr uint16_t PERIOD_BETWEEN_FLASH_LOGS_MS = 10000; // 10s between logs to flash

constexpr uint32_t TELEMETRY_DEFAULT_LOGGING_RATE_MS = 100; // Default logging delay for telemetry task
constexpr uint32_t TELEMETRY_MINIMUM_LOG_PERIOD_MS = 20; // (1000/20 = 50hz) The minimum log period / max log rate

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class TelemetryTask : public Task
{
public:
    static TelemetryTask& Inst() {
        static TelemetryTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { TelemetryTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

    void RunLogSequence();
    void RequestSample();
    void RequestTransmit();

    void SendVentStatus();

private:
    // Private Functions
    TelemetryTask();        // Private constructor
    TelemetryTask(const TelemetryTask&);                        // Prevent copy-construction
    TelemetryTask& operator=(const TelemetryTask&);            // Prevent assignment

    // Private Variables
    uint32_t loggingDelayMs;
    uint16_t numNonFlashLogs_;
    uint16_t numNonControlLogs_;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* TELEMETRYTASK_HPP_ */
