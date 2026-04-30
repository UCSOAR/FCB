/**
 ********************************************************************************
 * @file    RocketStateMachine.cpp
 * @author  Shivam Desai
 * @date    Apr 25, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "RocketStateMachine.hpp"
#include "RocketStates.hpp"
#include "SystemDefines.hpp"

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
/**
 * @brief Default constructor for Rocket SM, initializes all states
 */
RocketSM::RocketSM(RocketState startingState, bool enterStartingState)
{
    // Setup the internal array of states. Setup in order of enum.
    stateArray[RS_PRELAUNCH] = new PreLaunch();
    stateArray[RS_FILL] = new Fill();
    stateArray[RS_ARM] = new Arm();
    stateArray[RS_IGNITION] = new Ignition();
    stateArray[RS_LAUNCH] = new Launch();
    stateArray[RS_BURN] = new Burn();
    stateArray[RS_COAST] = new Coast();
    stateArray[RS_BRAKING] = new Braking();
    stateArray[RS_DESCENT] = new Descent();
    stateArray[RS_TOUCHDOWN] = new Touchdown();
    stateArray[RS_ABORT] = new Abort();
    stateArray[RS_TEST] = new Test();

    // TODO NEW
    // Verify all states are initialized AND state IDs are consistent
//    HDITask::Inst().SendCommand(Command(REQUEST_COMMAND, RS_ABORT));
//    for (uint8_t i = 0; i < RS_NONE; i++) {
//        SOAR_ASSERT(stateArray[i] != nullptr);
//        SOAR_ASSERT(stateArray[i]->GetStateID() == i);
//    }

    rs_currentState = stateArray[startingState];

    // If we need to run OnEnter for the starting state, do so
    if (enterStartingState) {
        rs_currentState->OnEnter();
    }

    SOAR_PRINT("Rocket State Machine Started in [ %s ] state\n", BaseRocketState::StateToString(rs_currentState->GetStateID()));
    // TODO NEW
    //    HDITask::Inst().SendCommand(Command(REQUEST_COMMAND, rs_currentState->GetStateID()));
}

/**
 * @brief Handles state transitions
 * @param nextState The next state to transition to
 * @return The state after the transition
 */
RocketState RocketSM::TransitionState(RocketState nextState)
{
    // Check if we're already in the next state (TransitionState does not allow entry into the existing state)
    if (nextState == rs_currentState->GetStateID())
        return rs_currentState->GetStateID();

    // Check the next state is valid
    if (nextState >= RS_NONE)
        return rs_currentState->GetStateID();

    RocketState previousState = rs_currentState->GetStateID();

    // Exit the current state
    rs_currentState->OnExit();

    // Set the next state
    rs_currentState = stateArray[nextState];

    // Assert the next state is initalized
    SOAR_ASSERT(rs_currentState != nullptr, "rs_currentState is nullptr in TransitionState");

    // TODO NEW
    //    HDITask::Inst().SendCommand(Command(REQUEST_COMMAND, rs_currentState->GetStateID()));

    // Enter the current state
    rs_currentState->OnEnter();

    SOAR_PRINT("ROCKET STATE TRANSITION [ %s ] --> [ %s ]\n", BaseRocketState::StateToString(previousState), BaseRocketState::StateToString(rs_currentState->GetStateID()));

    // Return the state after the transition
    return rs_currentState->GetStateID();
}

/**
 * @brief Handles current command
 * @param cm The command to handle
 */
void RocketSM::HandleCommand(Command& cm)
{
    SOAR_ASSERT(rs_currentState != nullptr, "Command received before state machine initialized");

    // Handle the command based on the current state
    RocketState nextRocketState = rs_currentState->HandleCommand(cm);

    // Run transition state - if the next state is the current state this does nothing
    if (nextRocketState != rs_currentState->GetStateID())
    {
        //Send new state to FlashTask for storing to SystemStorage
    	// TODO NEW
//        Command cmd(TASK_SPECIFIC_COMMAND, (uint16_t)WRITE_STATE_TO_FLASH);
//        uint8_t state = nextRocketState;
//        cmd.CopyDataToCommand(&state, 1);
//        FlashTask::Inst().GetEventQueue()->Send(cmd);

        TransitionState(nextRocketState);
    }
}

/**
 * @brief Returns a string for the state
 */
const char* BaseRocketState::StateToString(RocketState stateId)
{
    switch(stateId) {
    case RS_PRELAUNCH:
        return "Pre-launch";
    case RS_FILL:
        return "Fill";
    case RS_ARM:
        return "Arm";
    case RS_IGNITION:
        return "Ignition";
    case RS_LAUNCH:
        return "Launch";
    case RS_BURN:
        return "Burn";
    case RS_COAST:
        return "Coast";
    case RS_BRAKING:
    	return "Braking";
    case RS_DESCENT:
        return "Descent";
    case RS_TOUCHDOWN:
        return "Touchdown";
    case RS_ABORT:
        return "Abort";
    case RS_TEST:
        return "Test";
    case RS_NONE:
        return "None";
    default:
        return "WARNING: Invalid";
    }
}

/**
 * @brief Gets the current rocket state as a proto enum
 * @return Current rocket state
 */
// TODO NEW
//Proto::RocketState RocketSM::GetRocketStateAsProto()
//{
//    switch (rs_currentState->GetStateID()) {
//    case RS_PRELAUNCH:
//        return Proto::RocketState::RS_PRELAUNCH;
//    case RS_FILL:
//        return Proto::RocketState::RS_FILL;
//    case RS_ARM:
//        return Proto::RocketState::RS_ARM;
//    case RS_IGNITION:
//        return Proto::RocketState::RS_IGNITION;
//    case RS_LAUNCH:
//        return Proto::RocketState::RS_LAUNCH;
//    case RS_BURN:
//        return Proto::RocketState::RS_BURN;
//    case RS_COAST:
//        return Proto::RocketState::RS_COAST;
//    case RS_DESCENT:
//        return Proto::RocketState::RS_DESCENT;
//    case RS_RECOVERY:
//        return Proto::RocketState::RS_RECOVERY;
//    case RS_ABORT:
//        return Proto::RocketState::RS_ABORT;
//    case RS_TEST:
//        return Proto::RocketState::RS_TEST;
//    default:
//        return Proto::RocketState::RS_NONE;
//    }
//}
