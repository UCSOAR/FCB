/**
 ********************************************************************************
 * @file    CANTask.hpp
 * @author  Shivam Desai
 * @date    Apr 30, 2026
 * @brief
 ********************************************************************************
 */

#ifndef CANTASK_HPP_
#define CANTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CanAutoNodeMotherboard.hpp"
#include "RPBLogs.hpp"
#include "DAQLogs.hpp"
#include <array>

/************************************
 * MACROS AND DEFINES
 ************************************/
extern FDCAN_HandleTypeDef hfdcan1;

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class CANTask : public Task
{
public:
    static CANTask& Inst() {
        static CANTask inst;
        return inst;
    }

    void InitTask();

    /**
     * Any Task can use this API to send messages through the CAN Network
     * The Board target names and log index can be found in the CanAutoNode files in the communications repository
     */
    bool SendCANMessageToDaughter(char* aBoardName, uint8_t aLogIndex, const uint8_t* aMsg) {
    	return fcbCAN.SendMessageToDaughterByLogIndex(fcbCAN.GetIDOfBoardWithName(aBoardName), aLogIndex, aMsg);
    };

    /**
     * Check messages in Rx CAN buffers and handle them appropriately
     * Handling for specific messages is implemented locally in CANMessageHandler.cpp
     */
    void HandleIncomingCANMessages();


protected:
    static void RunTask(void* pvParams) { CANTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

private:
    // Private Functions
    CANTask();        // Private constructor
    CANTask(const CANTask&);                        // Prevent copy-construction
    CANTask& operator=(const CANTask&);            // Prevent assignment

    CanAutoNodeMotherboard fcbCAN{&hfdcan1};

    // Handle RPB Messages
    void HandleRPBAirBrakesCommand();

    // Handle DAQ Messages

    // Handle BMB Messages
};


/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CANTASK_HPP_ */
