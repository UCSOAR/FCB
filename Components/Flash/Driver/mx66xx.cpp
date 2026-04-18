#include "mx66xx.hpp"

#include "mx66xxConf.hpp"
#include "SystemDefines.hpp"
#include "main.h"
#include "stm32h7xx_hal.h"

#if (_MX66XX_USE_FREERTOS == 1)
#include "cmsis_os.h"
#define MX66xx_Delay(delay) osDelay(delay)
#else
#define MX66xx_Delay(delay) HAL_Delay(delay)
#endif

#define MX66XX_DUMMY_BYTE 0xA5
#define MX66XX_FASTREAD_DUMMY_CYCLES 8
#define MX66XX_UNIQID_DUMMY_CYCLES 32
#define MX66XX_SFDP_DUMMY_CYCLES 8
#define MX66XX_SFDP_READ_SIZE 256
#define MX66XX_SR1_WIP_BIT 0x01
#define MX66XX_SR1_WEL_BIT 0x02
#define MX66XX_SR1_BP_MASK 0x3C
#define MX66XX_WRITE_POLL_TIMEOUT_MS 10000

mx66xx_t mx66xx;
extern QSPI_HandleTypeDef _MX66XX_SPI;

static void MX66xx_BuildCommand(QSPI_CommandTypeDef *cmd,
                                uint8_t instruction,
                                uint32_t address,
                                uint32_t addressMode,
                                uint32_t dataMode,
                                uint32_t nbData,
                                uint32_t dummyCycles)
{
    *cmd = {0};
    cmd->Instruction = instruction;
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = addressMode;
    cmd->AddressSize = (addressMode == QSPI_ADDRESS_NONE)
                           ? QSPI_ADDRESS_24_BITS
                           : (mx66xx.Addr4Byte ? QSPI_ADDRESS_32_BITS : QSPI_ADDRESS_24_BITS);
    cmd->Address = address;
    cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd->DataMode = dataMode;
    cmd->NbData = nbData;
    cmd->DummyCycles = dummyCycles;
    cmd->DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
}

static bool MX66xx_IsWriteEnabled(void)
{
    return (MX66xx_ReadStatusRegister(1) & MX66XX_SR1_WEL_BIT) != 0;
}

static bool MX66xx_IsAnyBlockProtected(void)
{
    return (MX66xx_ReadStatusRegister(1) & MX66XX_SR1_BP_MASK) != 0;
}

static bool MX66xx_PrepareForErase(void)
{
    MX66xx_WaitForWriteEnd();
    if (MX66xx_IsAnyBlockProtected())
    {
        SOAR_PRINT("MX66xx_PrepareForErase() - Erase blocked: protection bits set (SR1=0x%02X)\n", MX66xx_ReadStatusRegister(1));
        return false;
    }

    MX66xx_WriteEnable();
    if (!MX66xx_IsWriteEnabled())
    {
        SOAR_PRINT("MX66xx_PrepareForErase() - WEL not set after WREN (SR1=0x%02X)\n", MX66xx_ReadStatusRegister(1));
        return false;
    }

    return true;
}

uint32_t MX66xx_ReadID(void)
{
    uint8_t rData[4] = {0, 0, 0};

    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x9F, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 3, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Receive(&_MX66XX_SPI, rData, 1000);

    uint32_t ret = (((uint32_t)rData[0] << 16) | ((uint32_t)rData[1] << 8) | (uint32_t)rData[2]);
    return ret;
}

void MX66xx_ReadUniqID(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x4B, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 8, MX66XX_UNIQID_DUMMY_CYCLES);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Receive(&_MX66XX_SPI, mx66xx.UniqID, 1000);
}

void MX66xx_EN4B(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0xB7, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
    mx66xx.Addr4Byte = 1;
}

void MX66xx_EX4B(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0xE9, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
    mx66xx.Addr4Byte = 0;
}

void MX66xx_RSTQIO(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0xF5, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
}

void MX66xx_RSTEN(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x66, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
}

void MX66xx_RST(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x99, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
}

void MX66xx_WriteEnable(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x06, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
}

void MX66xx_WriteDisable(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x04, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
}

uint8_t MX66xx_ReadStatusRegister(uint8_t SelectStatusRegister_1_2_3)
{
    uint8_t status = 0;
    QSPI_CommandTypeDef cmd = {};
    if (SelectStatusRegister_1_2_3 == 1)
    {
        MX66xx_BuildCommand(&cmd, 0x05, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);
    }
    else if (SelectStatusRegister_1_2_3 == 2)
    {
        MX66xx_BuildCommand(&cmd, 0x35, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);
    }
    else
    {
        MX66xx_BuildCommand(&cmd, 0x15, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);
    }
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Receive(&_MX66XX_SPI, &status, 1000);
    if (SelectStatusRegister_1_2_3 == 1)
    {
        mx66xx.StatusRegister1 = status;
    }
    else if (SelectStatusRegister_1_2_3 == 2)
    {
        mx66xx.StatusRegister2 = status;
    }
    else
    {
        mx66xx.StatusRegister3 = status;
    }
    return status;
}

void MX66xx_WriteStatusRegister(uint8_t SelectStatusRegister_1_2_3, uint8_t Data)
{
    QSPI_CommandTypeDef cmd = {};
    if (SelectStatusRegister_1_2_3 == 1)
    {
        MX66xx_BuildCommand(&cmd, 0x01, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);
        mx66xx.StatusRegister1 = Data;
    }
    else if (SelectStatusRegister_1_2_3 == 2)
    {
        MX66xx_BuildCommand(&cmd, 0x31, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);
        mx66xx.StatusRegister2 = Data;
    }
    else
    {
        MX66xx_BuildCommand(&cmd, 0x11, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);
        mx66xx.StatusRegister3 = Data;
    }
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Transmit(&_MX66XX_SPI, &Data, 1000);
}

void MX66xx_WaitForWriteEnd(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x05, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 1, 0);

    QSPI_AutoPollingTypeDef cfg = {};
    cfg.Match = 0x00;
    cfg.Mask = MX66XX_SR1_WIP_BIT;
    cfg.MatchMode = QSPI_MATCH_MODE_AND;
    cfg.StatusBytesSize = 1;
    cfg.Interval = 0x10;
    cfg.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    const HAL_StatusTypeDef cmdStatus = HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    if (cmdStatus != HAL_OK)
    {
        SOAR_PRINT("MX66xx_WaitForWriteEnd() - HAL_QSPI_Command failed: %d\n", (int)cmdStatus);
        return;
    }

    const HAL_StatusTypeDef pollStatus = HAL_QSPI_AutoPolling(&_MX66XX_SPI, &cmd, &cfg, MX66XX_WRITE_POLL_TIMEOUT_MS);
    if (pollStatus != HAL_OK)
    {
        SOAR_PRINT("MX66xx_WaitForWriteEnd() - AutoPolling timeout/fail: %d SR1=0x%02X\n",
                   (int)pollStatus,
                   MX66xx_ReadStatusRegister(1));
    }
    MX66xx_Delay(1);
}

bool MX66xx_Init(void)
{
    MX66xx_EX4B();
    mx66xx.Lock = 1;
    mx66xx.Addr4Byte = 0;

    while (HAL_GetTick() < 15)
        HAL_Delay(1);

    HAL_Delay(15);

    mx66xx.JedecId = MX66xx_ReadID();
    if (mx66xx.JedecId == 0 || mx66xx.JedecId == 0xFFFFFF)
    {
        mx66xx.Lock = 0;
        return false;
    }

    mx66xx.PageSize = FS_PAGE_SIZE;
    mx66xx.SectorSize = FS_SECTOR_SIZE;
    mx66xx.BlockSize = FS_BLOCK_SIZE;
    mx66xx.CapacityInKiloByte = (FS_TOTAL_SIZE / 1024);
    mx66xx.BlockCount = FS_TOTAL_SIZE / FS_BLOCK_SIZE;
    mx66xx.SectorCount = FS_TOTAL_SIZE / FS_SECTOR_SIZE;
    mx66xx.PageCount = FS_TOTAL_SIZE / FS_PAGE_SIZE;

    // MX66xx_EN4B();

    MX66xx_ReadUniqID();
    MX66xx_ReadStatusRegister(1);
    MX66xx_ReadStatusRegister(2);
    MX66xx_ReadStatusRegister(3);

    mx66xx.Lock = 0;
    return true;
}

void MX66xx_EraseChip(void)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (!MX66xx_PrepareForErase())
    {
        SOAR_PRINT("MX66xx_EraseChip() - Precondition failed, chip erase skipped\n");
        mx66xx.Lock = 0;
        return;
    }

    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0xC7, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    const HAL_StatusTypeDef status = HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    if (status != HAL_OK)
    {
        SOAR_PRINT("MX66xx_EraseChip() - HAL_QSPI_Command failed: %d\n", (int)status);
    }
    MX66xx_WaitForWriteEnd();

    MX66xx_Delay(10);
    mx66xx.Lock = 0;
}

void MX66xx_EraseSector(uint32_t SectorIndex)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (SectorIndex >= mx66xx.SectorCount)
    {
        SOAR_PRINT("MX66xx_EraseSector() - Invalid sector index %lu (max %lu)\n",
                   (uint32_t)SectorIndex,
                   (uint32_t)(mx66xx.SectorCount ? (mx66xx.SectorCount - 1U) : 0U));
        mx66xx.Lock = 0;
        return;
    }

    if (!MX66xx_PrepareForErase())
    {
        SOAR_PRINT("MX66xx_EraseSector() - Precondition failed for sector %lu\n", (uint32_t)SectorIndex);
        mx66xx.Lock = 0;
        return;
    }

    const uint32_t sectorAddr = SectorIndex * mx66xx.SectorSize;
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x20, sectorAddr, QSPI_ADDRESS_1_LINE, QSPI_DATA_NONE, 0, 0);
    const HAL_StatusTypeDef status = HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    if (status != HAL_OK)
    {
        SOAR_PRINT("MX66xx_EraseSector() - HAL_QSPI_Command failed: %d (sector=%lu addr=0x%08lX)\n",
                   (int)status,
                   (uint32_t)SectorIndex,
                   (uint32_t)sectorAddr);
    }
    MX66xx_WaitForWriteEnd();

    MX66xx_Delay(1);
    mx66xx.Lock = 0;
}

void MX66xx_EraseBlock(uint32_t BlockIndex)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (BlockIndex >= mx66xx.BlockCount)
    {
        SOAR_PRINT("MX66xx_EraseBlock() - Invalid block index %lu (max %lu)\n",
                   (uint32_t)BlockIndex,
                   (uint32_t)(mx66xx.BlockCount ? (mx66xx.BlockCount - 1U) : 0U));
        mx66xx.Lock = 0;
        return;
    }

    if (!MX66xx_PrepareForErase())
    {
        SOAR_PRINT("MX66xx_EraseBlock() - Precondition failed for block %lu\n", (uint32_t)BlockIndex);
        mx66xx.Lock = 0;
        return;
    }

    const uint32_t blockAddr = BlockIndex * mx66xx.BlockSize;
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0xD8, blockAddr, QSPI_ADDRESS_1_LINE, QSPI_DATA_NONE, 0, 0);
    const HAL_StatusTypeDef status = HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    if (status != HAL_OK)
    {
        SOAR_PRINT("MX66xx_EraseBlock() - HAL_QSPI_Command failed: %d (block=%lu addr=0x%08lX)\n",
                   (int)status,
                   (uint32_t)BlockIndex,
                   (uint32_t)blockAddr);
    }
    MX66xx_WaitForWriteEnd();

    MX66xx_Delay(1);
    mx66xx.Lock = 0;
}

uint32_t MX66xx_PageToSector(uint32_t PageAddress)
{
    return ((PageAddress * mx66xx.PageSize) / mx66xx.SectorSize);
}

uint32_t MX66xx_PageToBlock(uint32_t PageAddress)
{
    return ((PageAddress * mx66xx.PageSize) / mx66xx.BlockSize);
}

uint32_t MX66xx_SectorToBlock(uint32_t SectorAddress)
{
    return ((SectorAddress * mx66xx.SectorSize) / mx66xx.BlockSize);
}

uint32_t MX66xx_SectorToPage(uint32_t SectorAddress)
{
    return (SectorAddress * mx66xx.SectorSize) / mx66xx.PageSize;
}

uint32_t MX66xx_BlockToPage(uint32_t BlockAddress)
{
    return (BlockAddress * mx66xx.BlockSize) / mx66xx.PageSize;
}

bool MX66xx_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > mx66xx.PageSize) || (NumByteToCheck_up_to_PageSize == 0))
        NumByteToCheck_up_to_PageSize = mx66xx.PageSize - OffsetInByte;

    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < mx66xx.PageSize; i += sizeof(pBuffer))
    {
        WorkAddress = (i + Page_Address * mx66xx.PageSize);
        MX66xx_FastReadBytes(pBuffer, WorkAddress, sizeof(pBuffer));
        for (uint8_t x = 0; x < sizeof(pBuffer); x++)
        {
            if (pBuffer[x] != 0xFF)
            {
                mx66xx.Lock = 0;
                return false;
            }
        }
    }

    mx66xx.Lock = 0;
    return true;
}

bool MX66xx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (((NumByteToCheck_up_to_SectorSize + OffsetInByte) > mx66xx.SectorSize) || (NumByteToCheck_up_to_SectorSize == 0))
        NumByteToCheck_up_to_SectorSize = mx66xx.SectorSize - OffsetInByte;

    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < mx66xx.SectorSize; i += sizeof(pBuffer))
    {
        WorkAddress = (i + Sector_Address * mx66xx.SectorSize);
        MX66xx_FastReadBytes(pBuffer, WorkAddress, sizeof(pBuffer));
        for (uint8_t x = 0; x < sizeof(pBuffer); x++)
        {
            if (pBuffer[x] != 0xFF)
            {
                mx66xx.Lock = 0;
                return false;
            }
        }
    }

    mx66xx.Lock = 0;
    return true;
}

bool MX66xx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (((NumByteToCheck_up_to_BlockSize + OffsetInByte) > mx66xx.BlockSize) || (NumByteToCheck_up_to_BlockSize == 0))
        NumByteToCheck_up_to_BlockSize = mx66xx.BlockSize - OffsetInByte;

    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < mx66xx.BlockSize; i += sizeof(pBuffer))
    {
        WorkAddress = (i + Block_Address * mx66xx.BlockSize);
        MX66xx_FastReadBytes(pBuffer, WorkAddress, sizeof(pBuffer));
        for (uint8_t x = 0; x < sizeof(pBuffer); x++)
        {
            if (pBuffer[x] != 0xFF)
            {
                mx66xx.Lock = 0;
                return false;
            }
        }
    }

    mx66xx.Lock = 0;
    return true;
}

void MX66xx_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address)
{
    MX66xx_WritePage(&pBuffer, Bytes_Address / mx66xx.PageSize, Bytes_Address % mx66xx.PageSize, 1);
}

void MX66xx_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{
    while (mx66xx.Lock == 1)
        MX66xx_Delay(1);
    mx66xx.Lock = 1;

    if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > mx66xx.PageSize) || (NumByteToWrite_up_to_PageSize == 0))
        NumByteToWrite_up_to_PageSize = mx66xx.PageSize - OffsetInByte;

    uint32_t addr = (Page_Address * mx66xx.PageSize) + OffsetInByte;

    MX66xx_WriteEnable();
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x02, addr, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, NumByteToWrite_up_to_PageSize, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Transmit(&_MX66XX_SPI, pBuffer, 1000);

    MX66xx_WaitForWriteEnd();
    mx66xx.Lock = 0;
}

void MX66xx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
    if (((NumByteToWrite_up_to_SectorSize + OffsetInByte) > mx66xx.SectorSize) || (NumByteToWrite_up_to_SectorSize == 0))
        NumByteToWrite_up_to_SectorSize = mx66xx.SectorSize - OffsetInByte;

    uint32_t StartPage = MX66xx_SectorToPage(Sector_Address) + (OffsetInByte / mx66xx.PageSize);
    uint32_t BytesLeft = NumByteToWrite_up_to_SectorSize;
    uint32_t OffsetInPage = OffsetInByte % mx66xx.PageSize;
    uint32_t BufferIndex = 0;

    if (OffsetInPage)
    {
        uint32_t BytesToWrite = mx66xx.PageSize - OffsetInPage;
        if (BytesToWrite > BytesLeft)
            BytesToWrite = BytesLeft;

        MX66xx_WritePage(pBuffer, StartPage, OffsetInPage, BytesToWrite);
        BytesLeft -= BytesToWrite;
        BufferIndex += BytesToWrite;
        StartPage++;
    }

    while (BytesLeft > 0)
    {
        uint32_t BytesToWrite = (BytesLeft > mx66xx.PageSize) ? mx66xx.PageSize : BytesLeft;
        MX66xx_WritePage(&pBuffer[BufferIndex], StartPage, 0, BytesToWrite);
        BytesLeft -= BytesToWrite;
        BufferIndex += BytesToWrite;
        StartPage++;
    }
}

void MX66xx_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize)
{
    if (((NumByteToWrite_up_to_BlockSize + OffsetInByte) > mx66xx.BlockSize) || (NumByteToWrite_up_to_BlockSize == 0))
        NumByteToWrite_up_to_BlockSize = mx66xx.BlockSize - OffsetInByte;

    uint32_t StartPage = MX66xx_BlockToPage(Block_Address) + (OffsetInByte / mx66xx.PageSize);
    uint32_t BytesLeft = NumByteToWrite_up_to_BlockSize;
    uint32_t OffsetInPage = OffsetInByte % mx66xx.PageSize;
    uint32_t BufferIndex = 0;

    if (OffsetInPage)
    {
        uint32_t BytesToWrite = mx66xx.PageSize - OffsetInPage;
        if (BytesToWrite > BytesLeft)
            BytesToWrite = BytesLeft;

        MX66xx_WritePage(pBuffer, StartPage, OffsetInPage, BytesToWrite);
        BytesLeft -= BytesToWrite;
        BufferIndex += BytesToWrite;
        StartPage++;
    }

    while (BytesLeft > 0)
    {
        uint32_t BytesToWrite = (BytesLeft > mx66xx.PageSize) ? mx66xx.PageSize : BytesLeft;
        MX66xx_WritePage(&pBuffer[BufferIndex], StartPage, 0, BytesToWrite);
        BytesLeft -= BytesToWrite;
        BufferIndex += BytesToWrite;
        StartPage++;
    }
}

void MX66xx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address)
{
    MX66xx_ReadBytes(pBuffer, Bytes_Address, 1);
}

void MX66xx_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x03, ReadAddr, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, NumByteToRead, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Receive(&_MX66XX_SPI, pBuffer, 1000);
}

void MX66xx_FastReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0x0B, ReadAddr, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, NumByteToRead, MX66XX_FASTREAD_DUMMY_CYCLES);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Receive(&_MX66XX_SPI, pBuffer, 1000);
}

void MX66xx_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{
    if (((NumByteToRead_up_to_PageSize + OffsetInByte) > mx66xx.PageSize) || (NumByteToRead_up_to_PageSize == 0))
        NumByteToRead_up_to_PageSize = mx66xx.PageSize - OffsetInByte;
    uint32_t addr = (Page_Address * mx66xx.PageSize) + OffsetInByte;
    MX66xx_ReadBytes(pBuffer, addr, NumByteToRead_up_to_PageSize);
}

void MX66xx_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize)
{
    if (((NumByteToRead_up_to_SectorSize + OffsetInByte) > mx66xx.SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
        NumByteToRead_up_to_SectorSize = mx66xx.SectorSize - OffsetInByte;
    uint32_t addr = (Sector_Address * mx66xx.SectorSize) + OffsetInByte;
    MX66xx_ReadBytes(pBuffer, addr, NumByteToRead_up_to_SectorSize);
}

void MX66xx_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize)
{
    if (((NumByteToRead_up_to_BlockSize + OffsetInByte) > mx66xx.BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
        NumByteToRead_up_to_BlockSize = mx66xx.BlockSize - OffsetInByte;
    uint32_t addr = (Block_Address * mx66xx.BlockSize) + OffsetInByte;
    MX66xx_ReadBytes(pBuffer, addr, NumByteToRead_up_to_BlockSize);
}

void MX66xx_ReadSFDP(uint8_t *rData)
{
    if (rData == NULL)
        return;

    QSPI_CommandTypeDef cmd = {};
    uint8_t hdr[1] = {0};

    MX66xx_BuildCommand(&cmd, 0x5A, 0x00000004, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, 1, MX66XX_SFDP_DUMMY_CYCLES);

    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    HAL_QSPI_Receive(&_MX66XX_SPI, hdr, 3000);
}

void MX66xx_ReleaseFromDeepPowerDown(void)
{
    QSPI_CommandTypeDef cmd = {};
    MX66xx_BuildCommand(&cmd, 0xAB, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);
    HAL_QSPI_Command(&_MX66XX_SPI, &cmd, 1000);
    MX66xx_Delay(1);
}

bool MX66xx_IsWriteProtected(void)
{
    return false;
}
