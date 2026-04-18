/**
 ******************************************************************************
 * @file   SPIFlash.hpp
 * @brief  Wrapper for SPI flash memory operations
 ******************************************************************************
 */
#ifndef SPIFLASH_WRAPPER_HPP_
#define SPIFLASH_WRAPPER_HPP_

/* Includes ------------------------------------------------------------------*/
#include "mx66xx_qspi.hpp"
#include "mx66xxConf.hpp"
#include "SystemDefines.hpp"
#include "Flash.hpp"

/* Constants and Macros -------------------------------------------------------*/
constexpr uint16_t DEFAULT_FLASH_SECTOR_SIZE = 4096; // 4KB - Default If Flash was not Initialized

/* SPI Flash ------------------------------------------------------------------*/
class SPIFlash : public Flash
{
public:
    static SPIFlash &Inst()
    {
        static SPIFlash inst;
        return inst;
    }

    void Init() override
    {
        if (!isInitialized_)
        {
            MX66xxQSPI_Init();
            isInitialized_ = true;
        }
    }

    bool Erase(uint32_t offset) override
    {
        if (offset >= (mx66xx_qspi.SectorSize * mx66xx_qspi.SectorCount))
            return false;

        uint32_t SectorAddr = (offset / GetSectorSize());
        MX66xxQSPI_EraseSector(SectorAddr);
        return true;
    }

    bool Write(uint32_t offset, uint8_t *data, uint32_t len) override
    {
        if (offset + len >= (mx66xx_qspi.SectorSize * mx66xx_qspi.SectorCount) || len > mx66xx_qspi.SectorSize)
            return false;

        uint32_t SectorAddr = (offset / GetSectorSize());
        uint32_t OffsetInSector = offset % GetSectorSize();
        MX66xxQSPI_WriteSector(data, SectorAddr, OffsetInSector, len);
        return true;
    }

    bool Read(uint32_t offset, uint8_t *data, uint32_t len) override
    {
        if (offset + len >= (mx66xx_qspi.SectorSize * mx66xx_qspi.SectorCount) || len > mx66xx_qspi.SectorSize)
            return false;

        uint32_t SectorAddr = (offset / GetSectorSize());
        uint32_t OffsetInSector = offset % GetSectorSize();
        MX66xxQSPI_ReadSector(data, SectorAddr, OffsetInSector, len);
        return true;
    }

    bool EraseChip() override
    {
        MX66xxQSPI_EraseChip();
        return true;
    }

    uint32_t GetSectorSize() override
    {
        if (mx66xx_qspi.SectorSize == 0)
            return DEFAULT_FLASH_SECTOR_SIZE;

        return mx66xx_qspi.SectorSize;
    }

    bool GetInitialized()
    {
        return (isInitialized_ && (mx66xx_qspi.Lock == 0) && (mx66xx_qspi.PageSize != 0));
    }

private:
    SPIFlash()
    {
        isInitialized_ = false;
    }
    SPIFlash(const SPIFlash &);
    SPIFlash &operator=(const SPIFlash &);

    bool isInitialized_ = false;
};

#endif // SPIFLASH_WRAPPER_HPP_
