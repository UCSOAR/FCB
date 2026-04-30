/**
 ********************************************************************************
 * @file    GPIO.hpp
 * @author  Shivam Desai
 * @date    Apr 25, 2026
 * @brief
 ********************************************************************************
 */

#ifndef FCB_GPIO_HPP_
#define FCB_GPIO_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "SystemDefines.hpp"
#include "main.h"
#include "stm32h7xx_hal.h"

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/
namespace GPIO
{
	namespace Heater
	{
		inline void Open() { HAL_GPIO_WritePin(HEATER_GPIO_Port, HEATER_Pin, GPIO_PIN_RESET); }
		inline void Close() { HAL_GPIO_WritePin(HEATER_GPIO_Port, HEATER_Pin, GPIO_PIN_SET); }

		inline bool IsOpen() { return HAL_GPIO_ReadPin(HEATER_GPIO_Port, HEATER_Pin) == GPIO_PIN_RESET; }
	}


	namespace Vent
	{
		inline void Open() { HAL_GPIO_WritePin(VENT_GPIO_Port, VENT_Pin, GPIO_PIN_RESET); }
		inline void Close() { HAL_GPIO_WritePin(VENT_GPIO_Port, VENT_Pin, GPIO_PIN_SET); }

		inline bool IsOpen() { return HAL_GPIO_ReadPin(VENT_GPIO_Port, VENT_Pin) == GPIO_PIN_RESET; }
	}
}

#endif /* FCB_GPIO_HPP_ */
