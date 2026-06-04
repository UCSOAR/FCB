/*
 * TCTask.hpp
 *
 *  Created on: Jan 23, 2026
 *      Author: me
 */

#ifndef TCTASK_TCTASK_HPP_
#define TCTASK_TCTASK_HPP_

#include "Task.hpp"
#include "MAX31856MUD+Driver.hpp"
#include "SensorDataTypes.hpp"
/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/
enum TC_TASK_COMMANDS {
    TC_NONE = 0,
    TC_REQUEST_NEW_SAMPLE,// Get a new tc sample, task will be blocked for polling time
    TC_REQUEST_TRANSMIT,    // Send the current tc data over the Radio
    TC_REQUEST_DEBUG,        // Send the current tc data over the Debug UART
	TC_SET_FLASH_RATE
};


/************************************
 * CLASS DEFINITIONS
 ************************************/
extern SPI_HandleTypeDef hspi2;
/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class TCTask: public Task
{
	public:
		static TCTask& Inst() {
			static TCTask inst;
			return inst;
		}

		void InitTask();

	protected:
		bool RecieveData();
		static void RunTask(void* pvParams) { TCTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
	    void HandleRequestCommand(uint16_t taskCommand);

	    // Sampling
	    void SampleTC();
	    void TransmitProtocolTCData();

	    // Data
	    ThermocoupleData* data;
	    uint32_t timestampTC;

	    uint32_t ticksPerFlashLog = 0;

	private:
		// Private Functions
		TCTask();        // Private constructor
		TCTask(const TCTask&);                        // Prevent copy-construction
		TCTask& operator=(const TCTask&);														// Prevent assignment

		MAX31856Driver TCDriver1;
		MAX31856Driver TCDriver2;
		MAX31856Driver TCDriver3;
};


#endif /* TCTASK_TCTASK_HPP_ */
