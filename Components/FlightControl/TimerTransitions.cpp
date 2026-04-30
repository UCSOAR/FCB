/**
 ********************************************************************************
 * @file    TimerTransitions.cpp
 * @author  Shivam Desai
 * @date    Apr 29, 2026
 * @brief	Handles post launch timed transitions
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "TimerTransitions.hpp"
#include "SystemDefines.hpp"
#include "Timer.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
TimerTransitions::TimerTransitions() {
}

void TimerTransitions::Setup() {
	burnCountdown = new Timer(LaunchToBurnCallback);
	burnCountdown->ChangePeriodMs(BURN_TIMER_PERIOD_MS);
	coastCountdown = new Timer(BurnToCoastCallback);
	coastCountdown->ChangePeriodMs(COAST_TIMER_PERIOD_MS);
	brakingCountdown = new Timer(CoastToBrakingCallback);
	brakingCountdown->ChangePeriodMs(BRAKING_TIMER_PERIOD_MS);
	descentCountdown = new Timer(BrakingToDescentCallback);
	descentCountdown->ChangePeriodMs(DESCENT_TIMER_PERIOD_MS);
	touchdownCountdown = new Timer(DescentToTouchdownCallback);
	touchdownCountdown->ChangePeriodMs(TOUCHDOWN_TIMER_PERIOD_MS);
}

void TimerTransitions::BurnSequence() {
	if (!burnCountdown->Start())
		burnCountdown->ResetTimerAndStart();
    return;
}

void TimerTransitions::CoastSequence() {
	if(!coastCountdown->Start())
		coastCountdown->ResetTimerAndStart();
    return;
}

void TimerTransitions::BrakingSequence() {
	if(!brakingCountdown->Start())
		brakingCountdown->ResetTimerAndStart();
    return;
}

void TimerTransitions::DescentSequence() {
	if(!descentCountdown->Start())
		descentCountdown->ResetTimerAndStart();
    return;
}

void TimerTransitions::TouchdownSequence() {
	if(!touchdownCountdown->Start())
		touchdownCountdown->ResetTimerAndStart();
    return;
}
