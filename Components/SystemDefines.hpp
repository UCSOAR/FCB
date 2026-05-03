/**
  ******************************************************************************
  * @file           : SystemDefines.hpp
  * @brief          : Macros and wrappers
  ******************************************************************************
  *
  * Contains system wide macros, defines, and wrappers
  *
  ******************************************************************************
  */
#ifndef CUBE_MAIN_SYSTEM_DEFINES_H
#define CUBE_MAIN_SYSTEM_DEFINES_H

/* Environment Defines ------------------------------------------------------------------*/
//#define COMPUTER_ENVIRONMENT        // Define this if we're in Windows, Linux or Mac (not when flashing on DMB)

#ifdef COMPUTER_ENVIRONMENT
#define __CC_ARM
#endif

/* System Wide Includes ------------------------------------------------------------------*/
#include "main_system.hpp" // C++ Main File Header
#include "UARTDriver.hpp"

/* Cube++ Required Configuration ------------------------------------------------------------------*/
#include "CubeDefines.hpp"
constexpr UARTDriver* const DEFAULT_DEBUG_UART_DRIVER = UART::Debug;    // UART Handle that ASSERT messages are sent over

//Barometer constants
constexpr int TEMP_LOW = 2000;
constexpr int TEMP_VERY_LOW = -1500;
constexpr int CMD_SIZE = 1;
constexpr int CMD_TIMEOUT = 150;

enum GLOBAL_COMMANDS : uint8_t
{
  COMMAND_NONE = 0,        // No command, packet can probably be ignored
  TASK_SPECIFIC_COMMAND,    // Runs a task specific command when given this object
  DATA_COMMAND,// Data command, used to send data to a task. Target is stored in taskCommand
  CONTROL_ACTION,            // Control actions, used in Rocket State Machine, direct translation to RCU<->DMB Protocol
  REQUEST_COMMAND,            // Request command
  HEARTBEAT_COMMAND,            // Control actions for heartbeat commands
  PROTOCOL_COMMAND,           // Protocol command, used for commands to the Protocol Task
	DATA_BROKER_COMMAND,
	GPS_COMMAND,
};

/* Cube++ Optional Code Configuration ------------------------------------------------------------------*/


/* Task Parameter Definitions ------------------------------------------------------------------*/
/* - Lower priority number means lower priority task ---------------------------------*/

// FLIGHT PHASE
constexpr uint8_t FLIGHT_TASK_RTOS_PRIORITY = 2;            // Priority of the flight task
constexpr uint8_t FLIGHT_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the flight task queue
constexpr uint16_t FLIGHT_TASK_STACK_DEPTH_WORDS = 512;        // Size of the flight task stack

// UART TASK
constexpr uint8_t UART_TASK_RTOS_PRIORITY = 2;            // Priority of the uart task
constexpr uint8_t UART_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the uart task queue
constexpr uint16_t UART_TASK_STACK_DEPTH_WORDS = 512;    // Size of the uart task stack

// DEBUG TASK
constexpr uint8_t TASK_DEBUG_PRIORITY = 2;            // Priority of the debug task
constexpr uint8_t TASK_DEBUG_QUEUE_DEPTH_OBJS = 10;        // Size of the debug task queue
constexpr uint16_t TASK_DEBUG_STACK_DEPTH_WORDS = 512;        // Size of the debug task stack

// CAN BUS TASK
constexpr uint8_t TASK_CANBUS_PRIORITY = 2;
constexpr uint8_t TASK_CANBUS_QUEUE_DEPTH_OBJS = 10;
constexpr uint16_t TASK_CANBUS_STACK_DEPTH_WORDS = 512;

// LOGGING TASK
constexpr uint8_t TASK_LOGGING_PRIORITY = 3;
constexpr uint8_t TASK_LOGGING_QUEUE_DEPTH_OBJS = 10;
constexpr uint16_t TASK_LOGGING_QUEUE_DEPTH_WORDS = 512;

// FLASH TASK
constexpr uint8_t TASK_FLASH_TASK_PRIORITY = 3;         // Priority of the flash task
constexpr uint8_t TASK_FLASH_QUEUE_DEPTH_OBJS = 8;      // Size of the flash task queue
constexpr uint16_t TASK_FLASH_STACK_DEPTH_WORDS = 1024; // Size of the flash task stack

#endif // CUBE_MAIN_SYSTEM_DEFINES_H

