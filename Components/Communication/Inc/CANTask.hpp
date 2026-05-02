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
    // struct of daughterboards

};


/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CANTASK_HPP_ */
