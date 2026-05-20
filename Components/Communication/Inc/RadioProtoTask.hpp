/**
 ********************************************************************************
 * @file    RadioProtoTask.hpp
 * @author  Shivam Desai
 * @date    May 8, 2026
 * @brief
 ********************************************************************************
 */

#ifndef RADIOPROTOTASK_HPP_
#define RADIOPROTOTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "ProtocolTask.hpp"
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "UARTTask.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class RadioProtocolTask : public ProtocolTask
{
public:
    static RadioProtocolTask& Inst() {
        static RadioProtocolTask inst;
        return inst;
    }

    void InitTask();

    static void SendProtobufMessage(EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE>& writeBuffer, Proto::MessageID msgId)
    {
        Inst().ProtocolTask::SendProtobufMessage(writeBuffer, msgId);
    }

protected:
    static void RunTask(void* pvParams) { RadioProtocolTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    // These handlers will receive a buffer and size corresponding to a decoded message
    void HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);

    // Member variables

private:
    RadioProtocolTask();        // Private constructor
    RadioProtocolTask(const RadioProtocolTask&);                        // Prevent copy-construction
    RadioProtocolTask& operator=(const RadioProtocolTask&);            // Prevent assignment
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* RADIOPROTOTASK_HPP_ */
