/**
 ********************************************************************************
 * @file    RocketStates.hpp
 * @author  Shivam Desai
 * @date    Apr 25, 2026
 * @brief
 ********************************************************************************
 */

#ifndef ROCKETSTATES_HPP_
#define ROCKETSTATES_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "RocketStateMachine.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
/**
 * @brief PreLaunch state, waiting for commands to proceeding sequences
 */
class PreLaunch : public BaseRocketState
{
public:
    PreLaunch();

    // Base class
    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;

    // Non-ignition
    static RocketState HandleNonIgnitionCommands(RocketControlCommands rcAction, RocketState currentState);
};

/**
 * @brief Fill state, N2 Prefill/Purge/Leak-check/Load-cell Tare check sub-sequences, full control of valves (except MEV) allowed
 */
class Fill : public BaseRocketState
{
public:
    Fill();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;

private:
    bool arrArmConfirmFlags[2]; // we have three arm confirm flags
};

/**
 * @brief Arm state, we don't allow fill etc. 1-2 minutes before launch : Cannot fill rocket with N2 etc. unless you return to FILL
 */
class Arm : public BaseRocketState
{
public:
    Arm();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;

private:
};

/**
 * @brief Ignition state, ignition of the ignitors
 */
class Ignition : public BaseRocketState
{
public:
    Ignition();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Launch state, launch triggered by confirmation of ignition (from ignitor) is nominal : MEV Open Sequence
 */
class Launch : public BaseRocketState
{
public:
    Launch();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Burn state, main burn (vents closed MEV open) - 5-6 seconds (TBD) :
 */
class Burn : public BaseRocketState
{
public:
    Burn();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Coast state, coasting (MEV closed, vents closed) - 30 seconds (TBD) ^ Vents closed applies here too, in part. Includes APOGEE
 */
class Coast : public BaseRocketState
{
public:
    Coast();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Braking state, peripheral states same as Coast. Signals the opening of Air Brakes
 */
class Braking : public BaseRocketState
{
public:
	Braking();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Descent state, vents open (well into the descent)
 */
class Descent : public BaseRocketState
{
public:
    Descent();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Touchdown state, All valves open. Allow all control
 */
class Touchdown : public BaseRocketState
{
public:
	Touchdown();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Abort state, abort sequence, vents open, MEV closed, ignitors off
 */
class Abort : public BaseRocketState
{
public:
    Abort();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/**
 * @brief Test state, between ABORT and PRE-LAUNCH, has control over critical components
 *        such as MEV_ENABLE pins
 */
class Test : public BaseRocketState
{
public:
    Test();

    RocketState HandleCommand(Command& cm) override;
    RocketState OnEnter() override;
    RocketState OnExit() override;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* ROCKETSTATES_HPP_ */
