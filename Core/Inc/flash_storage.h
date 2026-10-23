
#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>
#include <stddef.h>

/* Storage operation results */
typedef enum
{
    FLASH_OK = 0,
    FLASH_ERROR,
    FLASH_INVALID_ADDRESS,
    FLASH_INVALID_LENGTH,
    FLASH_PROGRAM_ERROR,
    FLASH_VERIFY_ERROR
} Flash_Status;

/* Common storage API */
Flash_Status Flash_Init(void);

Flash_Status Flash_Read(uint32_t address,
                        uint8_t *buffer,
                        size_t length);

Flash_Status Flash_Write(uint32_t address,
                         const uint8_t *data,
                         size_t length);

Flash_Status Flash_Erase(uint32_t sector_address);

Flash_Status Flash_Verify(uint32_t address,
                          const uint8_t *data,
                          size_t length);

#endif /* FLASH_STORAGE_H */
