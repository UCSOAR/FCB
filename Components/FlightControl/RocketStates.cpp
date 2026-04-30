/**
 ********************************************************************************
 * @file    RocketStates.cpp
 * @author  Shivam Desai
 * @date    Apr 25, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "RocketStates.hpp"
#include "SystemDefines.hpp"
#include "GPIO.hpp"
#include "TimerTransitions.hpp"

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

/* PreLaunch State ------------------------------------------------------------------*/
/**
 * @brief PreLaunch state constructor
 */
PreLaunch::PreLaunch()
{
    rsStateID = RS_PRELAUNCH;
}

/**
 * @brief Entry to PreLaunch state
 * @return The state we're entering
 */
RocketState PreLaunch::OnEnter()
{
    GPIO::Vent::Close();

    // TODO NEW
    // PBBRxProtocolTask::SendFastLogCommand(Proto::FastLog::FastLogCommand::FL_RESET);
    // PBBRxProtocolTask::SendFastLogCommand(Proto::FastLog::FastLogCommand::FL_RESET);

    return rsStateID;
}

/**
 * @brief Exit from PreLaunch state
 * @return The state we're exiting
 */
RocketState PreLaunch::OnExit()
{
    // We don't do anything upon exiting prelaunch

	//TODO: Test code for testing hardfault, make sure commented out / removed after tests
    //uint8_t* crash = NULL;
    //*crash = 5;

    //if(&crash) {
        //return RS_ABORT;
    //}

    return rsStateID;
}

/**
 * @brief Handles control actions generally, can be used for derived states that allow full vent control
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
RocketState PreLaunch::HandleNonIgnitionCommands(RocketControlCommands rcAction, RocketState currentState)
{
    switch (rcAction) {
    case RSC_ANY_TO_ABORT:
        // Transition to abort state
        return RS_ABORT;
    case RSC_OPEN_VENT:
        GPIO::Vent::Open();
        // SOAR_PRINT("Vents were opened in [ %s ] state\n", StateToString(currentState));
        break;
    case RSC_CLOSE_VENT:
        GPIO::Vent::Close();
        // SOAR_PRINT("Vents were closed in [ %s ] state\n", StateToString(currentState));
        break;
    default:
        break;
    }
    return currentState ;
}

/**
 * @brief HandleCommand for PreLaunch state
 * @return The rocket state to transition or stay in
 */
RocketState PreLaunch::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    //// Handle general commands - these do not support state transitions
    //HandleGeneralStateCommands(cm.GetCommand());

    // Switch for the given command
    switch(cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_GOTO_FILL:
            // Transition to fill state
            nextStateID = RS_FILL;
            break;
        case RSC_GOTO_TEST:
            nextStateID = RS_TEST;
            break;
        default:
            // Handle as a general control action
            nextStateID = PreLaunch::HandleNonIgnitionCommands((RocketControlCommands)cm.GetTaskCommand(), GetStateID());
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}



/* Fill State ------------------------------------------------------------------*/
/**
 * @brief Fill state constructor
 */
Fill::Fill()
{
    rsStateID = RS_FILL;

    // Clear the arm flags
    for (uint8_t i = 0; i < 2; i++)
        arrArmConfirmFlags[i] = false;
}

/**
 * @brief Entry to Fill state
 * @return The state we're entering
 */
RocketState Fill::OnEnter()
{
    GPIO::Vent::Close();

    // Clear the arm flags
    for (uint8_t i = 0; i < 2; i++)
        arrArmConfirmFlags[i] = false;

    // TODO: Consider automatically beginning fill sequence (since we've already explicitly entered the fill state)

    return rsStateID;
}

/**
 * @brief Exit from Fill state
 * @return The state we're exiting
 */
RocketState Fill::OnExit()
{
    // Clear the arm flags
    for(uint8_t i = 0; i < 2; i++)
        arrArmConfirmFlags[i] = false;

    return rsStateID;
}

/**
 * @brief HandleCommand for Fill state
 * @return The rocket state to transition or stay in
 */
RocketState Fill::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    //// Handle general commands - these do not support state transitions
    //HandleGeneralStateCommands(cm.GetCommand());

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_ARM_CONFIRM_1:
            arrArmConfirmFlags[0] = true;
            break;
        case RSC_ARM_CONFIRM_2:
            arrArmConfirmFlags[1] = true;
            break;
        case RSC_GOTO_ARM:
            // Check if all arm confirmations have been received
            if (arrArmConfirmFlags[0] && arrArmConfirmFlags[1]) {
                // Transition to arm state
                nextStateID = RS_ARM;
            }
            break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            // Handle as a general control action
            nextStateID = PreLaunch::HandleNonIgnitionCommands((RocketControlCommands)cm.GetTaskCommand(), GetStateID());
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Arm State ------------------------------------------------------------------*/
/**
 * @brief Arm state constructor
 */
Arm::Arm()
{
    rsStateID = RS_ARM;
}

/**
 * @brief Entry to Arm state
 * @return The state we're entering
 */
RocketState Arm::OnEnter()
{
    // Assert the vent and drain are closed
    GPIO::Vent::Close();

    return rsStateID;
}

/**
 * @brief Exit from Arm state
 * @return The state we're exiting
 */
RocketState Arm::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Arm state
 * @return The rocket state to transition or stay in
 */
RocketState Arm::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_GOTO_IGNITION:
            // Transition to ready for ignition state
            nextStateID = RS_IGNITION;
            break;
        case RSC_GOTO_FILL:
            nextStateID = RS_FILL;
            break;
        default:
            // If manual override is enabled, handle the command as a non ignition command
            nextStateID = PreLaunch::HandleNonIgnitionCommands((RocketControlCommands)cm.GetTaskCommand(), GetStateID());
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Ignition State ------------------------------------------------------------------*/
/**
 * @brief Ignition state constructor
 */
Ignition::Ignition()
{
    rsStateID = RS_IGNITION;
}

/**
 * @brief Entry to Ignition state
 * @return The state we're entering
 */
RocketState Ignition::OnEnter()
{
    // Assert vent and drain closed
    GPIO::Vent::Close();
    
    // TODO NEW
    // PBBRxProtocolTask::SendFastLogCommand(Proto::FastLog::FastLogCommand::FL_PEND);
    // PBBRxProtocolTask::SendFastLogCommand(Proto::FastLog::FastLogCommand::FL_PEND);

    return rsStateID;
}

/**
 * @brief Exit from Ignition state
 * @return The state we're exiting
 */
RocketState Ignition::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Ignition state
 * @return The rocket state to transition or stay in
 */
RocketState Ignition::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_IGNITION_TO_LAUNCH:
            nextStateID = RS_LAUNCH;
            break;
        case RSC_GOTO_ARM:
            nextStateID = RS_ARM;
            break;
        //case RSC_MANUAL_IGNITION_CONFIRMED:
        //    TimerTransitions::Inst().ManualLaunch();
        //    break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Launch State ------------------------------------------------------------------*/
/**
 * @brief Launch state constructor
 */
Launch::Launch()
{
    rsStateID = RS_LAUNCH;
}

/**
 * @brief Entry to Launch state
 * @return The state we're entering
 */
RocketState Launch::OnEnter()
{
    // Assert vent and drain closed
    GPIO::Vent::Close();

    // TODO NEW
    TimerTransitions::Inst().BurnSequence();

    // TODO NEW
    // PBBRxProtocolTask::SendFastLogCommand(Proto::FastLog::FastLogCommand::FL_START);
    return rsStateID;
}

/**
 * @brief Exit from Launch state
 * @return The state we're exiting
 */
RocketState Launch::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Launch state
 * @return The rocket state to transition or stay in
 */
RocketState Launch::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_LAUNCH_TO_BURN:
            nextStateID = RS_BURN;
            break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Burn State ------------------------------------------------------------------*/
/**
 * @brief Burn state constructor
 */
Burn::Burn()
{
    rsStateID = RS_BURN;
}

/**
 * @brief Entry to Burn state
 * @return The state we're entering
 */
RocketState Burn::OnEnter()
{
    // TODO: Debug print - can remove in final versions
    if (GPIO::Vent::IsOpen()) {
        // SOAR_PRINT("Vents were not closed in [ %s ] state\n", StateToString(rsStateID));
    }

    // Assert vent/drain state
    GPIO::Vent::Close();

    // Start the coast transition timer (7 seconds - TBD based on sims)
    TimerTransitions::Inst().CoastSequence();

    return rsStateID;
}

/**
 * @brief Exit from Burn state
 * @return The state we're exiting
 */
RocketState Burn::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Burn state
 * @return The rocket state to transition or stay in
 */
RocketState Burn::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_BURN_TO_COAST:
            nextStateID = RS_COAST;
            break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Coast State ------------------------------------------------------------------*/
/**
 * @brief Coast state constructor
 */
Coast::Coast()
{
    rsStateID = RS_COAST;
}

/**
 * @brief Entry to Coast state
 * @return The state we're entering
 */
RocketState Coast::OnEnter()
{
    // Assert vent/drain state
    GPIO::Vent::Close();

	TimerTransitions::Inst().BrakingSequence();
    return rsStateID;
}

/**
 * @brief Exit from Coast state
 * @return The state we're exiting
 */
RocketState Coast::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Coast state
 * @return The rocket state to transition or stay in
 */
RocketState Coast::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_COAST_TO_BRAKING:
            nextStateID = RS_BRAKING;
            break;
        case RSC_COAST_TO_DESCENT:
            nextStateID = RS_DESCENT;
            break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Braking State ------------------------------------------------------------------*/
/**
 * @brief Coast state constructor
 */
Braking::Braking()
{
    rsStateID = RS_BRAKING;
}

/**
 * @brief Entry to Coast state
 * @return The state we're entering
 */
RocketState Braking::OnEnter()
{
    // Assert vent/drain state
    GPIO::Vent::Close();

	TimerTransitions::Inst().DescentSequence();
    return rsStateID;
}

/**
 * @brief Exit from Coast state
 * @return The state we're exiting
 */
RocketState Braking::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Coast state
 * @return The rocket state to transition or stay in
 */
RocketState Braking::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_BRAKING_TO_DESCENT:
            nextStateID = RS_DESCENT;
            break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Descent State ------------------------------------------------------------------*/
/**
 * @brief Descent state constructor
 */
Descent::Descent()
{
    rsStateID = RS_DESCENT;
}

/**
 * @brief Entry to Descent state
 * @return The state we're entering
 */
RocketState Descent::OnEnter()
{
    // Open the vent and drain
    GPIO::Vent::Open();

    // SOAR_PRINT("Vents were opened in [ %s ] state\n", StateToString(rsStateID));

    // Start Recovery Transition Timer (~300 seconds) : Should be well into / after descent
    TimerTransitions::Inst().TouchdownSequence();
    return rsStateID;
}

/**
 * @brief Exit from Descent state
 * @return The state we're exiting
 */
RocketState Descent::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Descent state
 * @return The rocket state to transition or stay in
 */
RocketState Descent::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_DESCENT_TO_TOUCHDOWN:
            nextStateID = RS_TOUCHDOWN;
            break;
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            break;
        }
        break;
    }
    default:
        // Do nothing
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}


/* Recovery State ------------------------------------------------------------------*/
/**
 * @brief Recovery state constructor
 */
Touchdown::Touchdown()
{
    rsStateID = RS_TOUCHDOWN;
}

/**
 * @brief Entry to Recovery state
 * @return The state we're entering
 */
RocketState Touchdown::OnEnter()
{
    // Assert vent and drain are open
    GPIO::Vent::Open();
    
    //TODO: Consider adding periodic AUTO-VENT timers every 100 seconds to make sure they're open)
    //TODO: Send out GPS and GPIO Status (actually should be happening always anyway)
    //TODO: Decrease log rate to 1 Hz - StorageManager should automatically stop logging after it gets near full

    return rsStateID;
}

/**
 * @brief Exit from Recovery state
 * @return The state we're exiting
 */
RocketState Touchdown::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Recovery state
 * @return The rocket state to transition or stay in
 */
RocketState Touchdown::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        default:
            // Handle as a general control action
            nextStateID = PreLaunch::HandleNonIgnitionCommands((RocketControlCommands)cm.GetTaskCommand(), GetStateID());
            break;
        }
        break;
    }
    default:
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Abort State ------------------------------------------------------------------*/
/**
 * @brief Abort state constructor
 */
Abort::Abort()
{
    rsStateID = RS_ABORT;
}

/**
 * @brief Entry to Abort state
 * @return The state we're entering
 */
RocketState Abort::OnEnter()
{
    // Make sure the MEV closed and vents are open
	GPIO::Vent::Open();

    return rsStateID;
}

/**
 * @brief Exit from Abort state
 * @return The state we're exiting
 */
RocketState Abort::OnExit()
{
	// TODO NEW
//    WatchdogTask::Inst().SendCommand(Command(HEARTBEAT_COMMAND, RADIOHB_REQUEST));
    return rsStateID;
}

/**
 * @brief HandleCommand for Abort state
 * @return The rocket state to transition or stay in
 */
RocketState Abort::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        case RSC_GOTO_TEST:
            nextStateID = RS_TEST;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}

/* Test State ------------------------------------------------------------------*/
/**
 * @brief Test state constructor
 */
Test::Test()
{
    rsStateID = RS_TEST;
}

/**
 * @brief Entry to Test state
 * @return The state we're entering
 */
RocketState Test::OnEnter()
{
	// Test does not have any entry actions
    return rsStateID;
}

/**
 * @brief Exit from Test state
 * @return The state we're exiting
 */
RocketState Test::OnExit()
{
    return rsStateID;
}

/**
 * @brief HandleCommand for Test state
 * @return The rocket state to transition or stay in
 */
RocketState Test::HandleCommand(Command& cm)
{
    RocketState nextStateID = GetStateID();

    // Switch for the given command
    switch (cm.GetCommand()) {
    case CONTROL_ACTION: {
        switch (cm.GetTaskCommand()) {
        case RSC_GOTO_PRELAUNCH:
            nextStateID = RS_PRELAUNCH;
            break;
        default:
            nextStateID = PreLaunch::HandleNonIgnitionCommands((RocketControlCommands)cm.GetTaskCommand(), GetStateID());
            break;
        }
        break;
    }
    default:
        break;
    }

    // Make sure to reset the command, and return the next state
    cm.Reset();
    return nextStateID;
}
