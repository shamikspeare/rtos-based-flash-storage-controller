
#ifndef W25Q64_H
#define W25Q64_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* W25Q64FV commands */
#define W25Q64_CMD_JEDEC_ID  0x9FU

/* Read the three JEDEC identification bytes */
HAL_StatusTypeDef W25Q64_ReadJEDECID(uint8_t id[3]);

#endif /* W25Q64_H */
