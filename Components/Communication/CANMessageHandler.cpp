/**
 ********************************************************************************
 * @file    CANMessageHandler.cpp
 * @author  Shivam Desai
 * @date    May 2, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "CANTask.hpp"

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
void CANTask::HandleIncomingCANMessages() {
	HandleRPBAirBrakesCommand();
}

void CANTask::HandleRPBAirBrakesCommand() {
	bool isDataAvailable = false;

	RPB_AIR_BRAKES_COMMAND airBrakesInstruction{false};
	isDataAvailable = fcbCAN.ReadMessageFromDaughterByLogIndex(
		fcbCAN.GetIDOfBoardWithName(CAN_ROCKET_TARGET_RPB),
		RPB_LogIndexes::_RPB_AIR_BRAKES_COMMAND_LOGINDEX,
		(uint8_t*)&airBrakesInstruction,
		sizeof(DAQ_AIR_BRAKES_COMMAND)
	);

	if (isDataAvailable) {
		if (airBrakesInstruction.openAirBrakes) {
			SOAR_PRINT("AIR Brakes Instruction received true");
		} else {
			SOAR_PRINT("AIR Brakes Instruction received false");
		}
	}
}
