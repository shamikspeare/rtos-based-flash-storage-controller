
#ifndef FLASH_SIMULATOR_H
#define FLASH_SIMULATOR_H

#include "flash_storage.h"

/* Simulated flash geometry */
#define SIM_FLASH_SIZE       (32U * 1024U)
#define SIM_FLASH_PAGE_SIZE  256U
#define SIM_FLASH_SECTOR_SIZE 4096U

/* RAM simulator interface */
Flash_Status FlashSim_Init(void);

Flash_Status FlashSim_Read(uint32_t address,
                           uint8_t *buffer,
                           size_t length);

Flash_Status FlashSim_Write(uint32_t address,
                            const uint8_t *data,
                            size_t length);

Flash_Status FlashSim_Erase(uint32_t sector_address);

#endif /* FLASH_SIMULATOR_H */
