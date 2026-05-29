/*
 * actualflash.hpp
 *
 *  Created on: May 28, 2026
 *      Author: Local user
 */

#ifndef ACTUALFLASH_ACTUALFLASH_HPP_
#define ACTUALFLASH_ACTUALFLASH_HPP_


#include "stm32h7xx_hal.h"
#include <stdint.h>

class MX66L1G45G {
public:
    // Pass the QSPI handle configured in CubeMX (e.g., &hqspi)
    MX66L1G45G(QSPI_HandleTypeDef* hqspi);

    // Initializes chip, enables Quad mode, and verifies communication
    bool Init();

    // Reads the Manufacturer (0xC2) and Device IDs
    bool ReadID(uint8_t* manufacturerID, uint16_t* deviceID);

    // Erases a 4KB sector. Address must be sector-aligned.
    bool EraseSector(uint32_t address);

    // Writes up to 256 bytes. Cannot cross page boundaries!
    bool WritePage(uint32_t address, const uint8_t* data, uint32_t size);

    // Reads data using 1-1-4 Quad Output Read (4-byte address)
    bool ReadData(uint32_t address, uint8_t* data, uint32_t size);

    // Writes data of any size to any address, automatically crossing page boundaries
    bool WriteData(uint32_t address, const uint8_t* data, uint32_t size);
private:
    QSPI_HandleTypeDef* hqspi_;

    uint8_t id;
    uint16_t manid;

    // Internal helper functions
    bool WriteEnable();
    bool WaitForReady(uint32_t timeout = 5000); // 5 sec timeout for erases
    bool EnableQuadMode();
};

#endif /* ACTUALFLASH_ACTUALFLASH_HPP_ */
