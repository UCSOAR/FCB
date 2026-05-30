/*
 * actualflash.hpp
 *
 *  Created on: May 28, 2026
 *      Author: Local user
 */

// mostly cooked up by ai

#ifndef ACTUALFLASH_ACTUALFLASH_HPP_
#define ACTUALFLASH_ACTUALFLASH_HPP_


#include "stm32h7xx_hal.h"
#include <stdint.h>

class MX66L1G45G {
public:

    MX66L1G45G();

    static MX66L1G45G& Inst() {
    	static MX66L1G45G e;
    	e.Init();
    	return e;
    }

    bool Init();

    bool ReadID(uint8_t* manufacturerID, uint16_t* deviceID);

    bool EraseSector(uint32_t address);

    // Writes up to 256 bytes. Cannot cross page boundaries!
    bool WritePage(uint32_t address, const uint8_t* data, uint32_t size);

    bool ReadData(uint32_t address, uint8_t* data, uint32_t size);

    bool WriteData(uint32_t address, const uint8_t* data, uint32_t size);
private:
    QSPI_HandleTypeDef* hqspi_;

    uint8_t id;
    uint16_t manid;

    bool inited = false;

    bool WriteEnable();
    bool WaitForReady(uint32_t timeout = 5000);
    bool EnableQuadMode();
};

#endif /* ACTUALFLASH_ACTUALFLASH_HPP_ */
