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
