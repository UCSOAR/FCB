/*
 * ActualLoggingTask.hpp
 *
 *  Created on: Jan 23, 2026
 *      Author: jaddina
 */

#ifndef ACTUALLOGGINGTASK_ACTUALLOGGINGTASK_HPP_
#define ACTUALLOGGINGTASK_ACTUALLOGGINGTASK_HPP_

#include "Task.hpp"
#include "actualflash.hpp"
#include "SensorDataTypes.hpp"
/************************************
 * MACROS AND DEFINES
 ************************************/


/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
extern QSPI_HandleTypeDef hqspi;

enum LOGGING_TASK_CMDS {
	LOG_STATE_RECO
};
/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class ActualLoggingTask: public Task
{
	public:
		static ActualLoggingTask& Inst() {
			static ActualLoggingTask inst;
			return inst;
		}

		void InitTask();



	protected:
		bool RecieveData();
		static void RunTask(void* pvParams) { ActualLoggingTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		//uint8_t debugBuffer[LOGGING_RX_BUFFER_SZ_BYTES + 1];

	private:
		// Private Functions
		ActualLoggingTask();        // Private constructor
		ActualLoggingTask(const ActualLoggingTask&);                        // Prevent copy-construction
		ActualLoggingTask& operator=(const ActualLoggingTask&);														// Prevent assignment
		static uint8_t buf[20];

		constexpr static uint32_t TC_DATA_START_ADDR = 0x00000000;
		constexpr static uint32_t TC_DATA_END_ADDR   = 0x003fffff;

		constexpr static uint32_t PT_DATA_START_ADDR = 0x00400000;
		constexpr static uint32_t PT_DATA_END_ADDR   = 0x007fffff;

		uint32_t tcCurrentAddr = 0;
		uint32_t ptCurrentAddr = 0;

		struct __attribute__((packed)) TC_Stored {
			ThermocoupleData data;
			uint32_t timestamp;
			uint32_t check;
		};

		struct __attribute__((packed)) PT_Stored {
			PressureTransducerData data;
			uint32_t timestamp;
			uint32_t check;
		};

//		static uint8_t firstAlt;
};


#endif /* LOGGINGTASK_LOGGINGTASK_HPP_ */
