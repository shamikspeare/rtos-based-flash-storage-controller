#include "flash_storage.h"
#include "w25q64.h"
/* Initialize storage */
/* Initialize physical flash */
Flash_Status Flash_Init(void)
{
    uint8_t id[3] = {0};

    if (W25Q64_ReadJEDECID(id) != HAL_OK)
    {
        return FLASH_ERROR;
    }

    /* Verify expected Winbond W25Q64FV */
    if ((id[0] != 0xEFU) ||
        (id[1] != 0x40U) ||
        (id[2] != 0x17U))
    {
        return FLASH_ERROR;
    }

    /* Wait until device is ready */
    if (W25Q64_WaitWhileBusy(1000U) != HAL_OK)
    {
        return FLASH_ERROR;
    }

    return FLASH_OK;
}

/* Read data */
/* Read data from physical flash */
Flash_Status Flash_Read(uint32_t address,
                        uint8_t *buffer,
                        size_t length)
{
    /* Validate buffer */
    if (buffer == NULL)
    {
        return FLASH_ERROR;
    }

    /* Validate length */
    if (length == 0U)
    {
        return FLASH_INVALID_LENGTH;
    }

    /* Validate starting address */
    if (address >= W25Q64_FLASH_SIZE)
    {
        return FLASH_INVALID_ADDRESS;
    }

    /* Prevent crossing flash capacity */
    if (length > (size_t)(W25Q64_FLASH_SIZE - address))
    {
        return FLASH_INVALID_LENGTH;
    }

    HAL_StatusTypeDef result =
        W25Q64_Read(address, buffer, (uint32_t)length);

    if (result != HAL_OK)
    {
        return FLASH_ERROR;
    }

    return FLASH_OK;
}

/* Program data */
Flash_Status Flash_Write(uint32_t address,
                         const uint8_t *data,
                         size_t length)
{
    (void)address;
    (void)data;
    (void)length;

    return FLASH_ERROR;
}

Flash_Status Flash_Erase(uint32_t sector_address)
{
    (void)sector_address;

    return FLASH_ERROR;
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
