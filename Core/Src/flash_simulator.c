
#include "flash_simulator.h"

Flash_Status FlashSim_Init(void)
{
    /* Implementation in Phase 2.2 */
    return FLASH_ERROR;
}

Flash_Status FlashSim_Read(uint32_t address,
                           uint8_t *buffer,
                           size_t length)
{
    (void)address;
    (void)buffer;
    (void)length;

    return FLASH_ERROR;
}

Flash_Status FlashSim_Write(uint32_t address,
                            const uint8_t *data,
                            size_t length)
{
    (void)address;
    (void)data;
    (void)length;

    return FLASH_ERROR;
}

Flash_Status FlashSim_Erase(uint32_t sector_address)
{
    (void)sector_address;

    return FLASH_ERROR;
}
