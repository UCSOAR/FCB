/**
 ********************************************************************************
 * @file    TimerTransitions.hpp
 * @author  Shivam Desai
 * @date    Apr 29, 2026
 * @brief
 ********************************************************************************
 */

#ifndef TIMERTRANSITIONS_HPP_
#define TIMERTRANSITIONS_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "SystemDefines.hpp"
#include "RocketStateMachine.hpp"
#include "Timer.hpp"
#include "FlightTask.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
// TODO NEW : DEFINE THESE CORRECTLY
// ONLY USE TIMER TRANSITIONS IN THE CASE THAT THE DAQ ALGORITHM IS FAILING
constexpr uint32_t BURN_TIMER_PERIOD_MS = 5 * 1000;		// LAUNCH -> BURN
constexpr uint32_t COAST_TIMER_PERIOD_MS = 5 * 1000;	// BURN -> COAST
// TODO NEW : WHEN SWITCHING TO BRAKING, IF DAQ IS FAILING OPEN AIRBRAKES FROM STATE TRANSITION AND SEND MESSAGE
constexpr uint32_t BRAKING_TIMER_PERIOD_MS = 5 * 1000;	// COAST -> BRAKING
constexpr uint32_t DESCENT_TIMER_PERIOD_MS = 5 * 1000;	// BRAKING -> DESCENT
constexpr uint32_t TOUCHDOWN_TIMER_PERIOD_MS = 5 * 1000;	// DESCENT -> TOUCHDOWN

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class TimerTransitions
{
public:
    static TimerTransitions& Inst() {
        static TimerTransitions inst;
        return inst;
    }
    TimerTransitions();

    void Setup();

    void BurnSequence();
    void CoastSequence();
    void BrakingSequence();
    void DescentSequence();
    void TouchdownSequence();

protected:
    inline static void LaunchToBurnCallback(TimerHandle_t rtTimerHandle) {
    	FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, RSC_LAUNCH_TO_BURN));
    }
    inline static void BurnToCoastCallback(TimerHandle_t rtTimerHandle) {
    	FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, RSC_BURN_TO_COAST));
    }
    inline static void CoastToBrakingCallback(TimerHandle_t rtTimerHandle) {
    	FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, RSC_COAST_TO_BRAKING));
    }
    inline static void BrakingToDescentCallback(TimerHandle_t rtTimerHandle) {
		FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, RSC_BRAKING_TO_DESCENT));
	}
    inline static void DescentToTouchdownCallback(TimerHandle_t rtTimerHandle) {
    	FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, RSC_DESCENT_TO_TOUCHDOWN));
    }

private:
    Timer* burnCountdown;
    Timer* coastCountdown;
    Timer* brakingCountdown;
    Timer* descentCountdown;
    Timer* touchdownCountdown;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* TIMERTRANSITIONS_HPP_ */
