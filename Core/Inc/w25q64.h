
#ifndef W25Q64_H
#define W25Q64_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

/* Physical flash geometry */
#define W25Q64_FLASH_SIZE       (8UL * 1024UL * 1024UL)
#define W25Q64_PAGE_SIZE        256U
#define W25Q64_SECTOR_SIZE      4096U

/* SPI command codes */
#define W25Q64_CMD_JEDEC_ID     0x9FU
#define W25Q64_CMD_READ_SR1     0x05U
#define W25Q64_CMD_READ_SR2     0x35U
#define W25Q64_CMD_READ_DATA    0x03U

/* Status Register 1 masks */
#define W25Q64_SR1_BUSY         0x01U
#define W25Q64_SR1_WEL          0x02U

/* Device identification */
HAL_StatusTypeDef W25Q64_ReadJEDECID(uint8_t id[3]);

/* Status register operations */
HAL_StatusTypeDef W25Q64_ReadStatus1(uint8_t *status);
HAL_StatusTypeDef W25Q64_ReadStatus2(uint8_t *status);

HAL_StatusTypeDef W25Q64_IsBusy(uint8_t *busy);
HAL_StatusTypeDef W25Q64_WaitWhileBusy(uint32_t timeout_ms);

/* Phase 3.4: Physical memory read */
HAL_StatusTypeDef W25Q64_Read(
    uint32_t address,
    uint8_t *buffer,
    uint32_t length
);

/* Phase 3.5: New commands */
#define W25Q64_CMD_WRITE_ENABLE  0x06U
#define W25Q64_CMD_PAGE_PROGRAM  0x02U
#define W25Q64_CMD_SECTOR_ERASE  0x20U

/* Dedicated destructive test sector */
#define W25Q64_TEST_SECTOR_ADDR  0x7FF000UL

/* Conservative operation timeouts */
#define W25Q64_PROGRAM_TIMEOUT_MS  100U
#define W25Q64_ERASE_TIMEOUT_MS    5000U

HAL_StatusTypeDef W25Q64_WriteEnable(void);

HAL_StatusTypeDef W25Q64_EraseSector(
    uint32_t address
);

HAL_StatusTypeDef W25Q64_PageProgram(
    uint32_t address,
    const uint8_t *data,
    uint32_t length
);

#endif /* W25Q64_H */
