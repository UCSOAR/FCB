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
template <typename T>
struct CANMessage {
    T first;
    char* boardName;
    DAQ_LogIndexes messageLogIndex;
};

//static std::array<int, 5> numbers = {1, 2, 3, 4, 5};

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
    bool SendCANMessageToDaughter(char* aBoardName, uint8_t aLogIndex, const uint8_t* aMsg) {
    	return fcbCAN.SendMessageToDaughterByLogIndex(fcbCAN.GetIDOfBoardWithName(aBoardName), aLogIndex, aMsg);
    };




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

    void PollForCANMessages();

    template <typename T>
	T ReceiveCANMessage(T x, T y) {
    	bool isDataAvailable = false;
		DAQ_AIR_BRAKES_COMMAND airBrakesInstruction{false};
		isDataAvailable = fcbCAN.ReadMessageFromDaughterByLogIndex(
			fcbCAN.GetIDOfBoardWithName(CAN_ROCKET_TARGET_DAQ),
			DAQ_LogIndexes::_DAQ_AIR_BRAKES_COMMAND_LOGINDEX,
			(uint8_t*)&airBrakesInstruction,
			sizeof(DAQ_AIR_BRAKES_COMMAND)
		);
	}


};


/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CANTASK_HPP_ */
