/*
 * actualflash.cpp
 *
 *  Created on: May 28, 2026
 *      Author: Local user
 */




#include "actualflash.hpp"

// Macronix Command Set
#define CMD_READ_ID              0x9F
#define CMD_READ_STATUS_REG      0x05
#define CMD_WRITE_STATUS_REG     0x01
#define CMD_WRITE_ENABLE         0x06
#define CMD_4BYTE_SECTOR_ERASE   0x21 // 4KB Erase with 4-byte address
#define CMD_4BYTE_PAGE_PROGRAM   0x12 // 1-1-1 Page Program with 4-byte address
#define CMD_4BYTE_QUAD_READ      0x6C // 1-1-4 Quad Output Read with 4-byte address

#include "main.h"

extern QSPI_HandleTypeDef hqspi;

MX66L1G45G::MX66L1G45G() : hqspi_(&hqspi) {}

bool MX66L1G45G::Init() {
	if(inited) {
		return true;
	}
    if (hqspi_ == nullptr) return false;

    HAL_GPIO_WritePin(FLASH_RESET_GPIO_Port, FLASH_RESET_Pin, GPIO_PIN_SET);

    if (!ReadID(&id, &manid)) return false;

    if(id == 0 && manid == 0) return false;

    if (!EnableQuadMode()) return false;

    inited = true;
    return true;
}

bool MX66L1G45G::ReadID(uint8_t* manufacturerID, uint16_t* deviceID) {
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t idData[3];

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_READ_ID;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = 3;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;
    if (HAL_QSPI_Receive(hqspi_, idData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;

    *manufacturerID = idData[0];
    *deviceID = (idData[1] << 8) | idData[2];

    return true;
}

bool MX66L1G45G::WriteEnable() {
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_WRITE_ENABLE;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    return HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) == HAL_OK;
}

bool MX66L1G45G::WaitForReady(uint32_t timeout) {
    QSPI_CommandTypeDef sCommand = {0};
    QSPI_AutoPollingTypeDef sConfig = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_READ_STATUS_REG;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    sConfig.Match           = 0x00;
    sConfig.Mask            = 0x01; // WIP (Write In Progress) bit is Bit 0
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    return HAL_QSPI_AutoPolling(hqspi_, &sCommand, &sConfig, timeout) == HAL_OK;
}

bool MX66L1G45G::EnableQuadMode() {
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t statusReg = 0;

    // 1. Read Status Register
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_READ_STATUS_REG;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = 1;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;
    if (HAL_QSPI_Receive(hqspi_, &statusReg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;

    // Bit 6 is QE (Quad Enable) on Macronix
    if ((statusReg & 0x40) != 0) return true; // Already enabled

    statusReg |= 0x40; // Set QE bit

    // 2. Write Enable
    if (!WriteEnable()) return false;

    // 3. Write Status Register
    sCommand.Instruction       = CMD_WRITE_STATUS_REG;
    if (HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;
    if (HAL_QSPI_Transmit(hqspi_, &statusReg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;

    return WaitForReady();
}

bool MX66L1G45G::EraseSector(uint32_t address) {
    if (!WriteEnable()) return false;

    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_4BYTE_SECTOR_ERASE;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_32_BITS; // 4-Byte Address
    sCommand.Address           = address;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;

    return WaitForReady();
}

bool MX66L1G45G::WritePage(uint32_t address, const uint8_t* data, uint32_t size) {
    if (size == 0 || size > 256) return false;
    if (!WriteEnable()) return false;

    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_4BYTE_PAGE_PROGRAM;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_32_BITS; // 4-Byte Address
    sCommand.Address           = address;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;     // 1-Line Data
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = size;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;
    if (HAL_QSPI_Transmit(hqspi_, (uint8_t*)data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;

    return WaitForReady();
}

bool MX66L1G45G::ReadData(uint32_t address, uint8_t* data, uint32_t size) {
    if (size == 0) return true;

    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_4BYTE_QUAD_READ;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_32_BITS; // 4-Byte Address
    sCommand.Address           = address;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;    // 4-Line Data (Quad)
    sCommand.DummyCycles       = 8;                    // 8 dummy cycles for 0x6C on Macronix
    sCommand.NbData            = size;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi_, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;
    if (HAL_QSPI_Receive(hqspi_, data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return false;

    return true;
}

bool MX66L1G45G::WriteData(uint32_t address, const uint8_t* data, uint32_t size) {
    if (size == 0) return true;

    uint32_t currentAddress = address;
    const uint8_t* currentData = data;
    uint32_t remainingSize = size;

    while (remainingSize > 0) {
        // 1. Calculate how many bytes are left in the current 256-byte page
        uint32_t bytesLeftInPage = 256 - (currentAddress % 256);

        // 2. Write whichever is smaller: the remaining data, or the bytes left in the page
        uint32_t writeSize = (remainingSize < bytesLeftInPage) ? remainingSize : bytesLeftInPage;

        // 3. Commit the write using the page-bound logic
        if (!WritePage(currentAddress, currentData, writeSize)) {
            return false;
        }

        // 4. Advance pointers and decrease counters
        currentAddress += writeSize;
        currentData += writeSize;
        remainingSize -= writeSize;
    }

    return true;
}
