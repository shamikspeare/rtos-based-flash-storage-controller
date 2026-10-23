
#include "flash_storage.h"
#include "flash_simulator.h"

/* Initialize storage */
Flash_Status Flash_Init(void)
{
    return FlashSim_Init();
}

/* Read data */
Flash_Status Flash_Read(uint32_t address,
                        uint8_t *buffer,
                        size_t length)
{
    return FlashSim_Read(address, buffer, length);
}

/* Program data */
Flash_Status Flash_Write(uint32_t address,
                         const uint8_t *data,
                         size_t length)
{
    return FlashSim_Write(address, data, length);
}

/* Erase a sector */
Flash_Status Flash_Erase(uint32_t sector_address)
{
    return FlashSim_Erase(sector_address);
}

/* Verify stored data */
Flash_Status Flash_Verify(uint32_t address,
                          const uint8_t *data,
                          size_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return FLASH_INVALID_LENGTH;
    }

    uint8_t read_buffer[32];
    size_t offset = 0;

    while (offset < length)
    {
        size_t chunk = length - offset;

        if (chunk > sizeof(read_buffer))
        {
            chunk = sizeof(read_buffer);
        }

        Flash_Status status =
            Flash_Read(address + (uint32_t)offset,
                       read_buffer,
                       chunk);

        if (status != FLASH_OK)
        {
            return status;
        }

        for (size_t i = 0; i < chunk; i++)
        {
            if (read_buffer[i] != data[offset + i])
            {
                return FLASH_VERIFY_ERROR;
            }
        }

        offset += chunk;
    }

    return FLASH_OK;
}
