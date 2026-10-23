
#include "w25q64.h"

extern SPI_HandleTypeDef hspi1;

/* Chip Select: PB6 */
#define FLASH_CS_PORT GPIOB
#define FLASH_CS_PIN  GPIO_PIN_6

#define SPI_TIMEOUT_MS 100U

static void W25Q64_Select(void)
{
    HAL_GPIO_WritePin(
        FLASH_CS_PORT,
        FLASH_CS_PIN,
        GPIO_PIN_RESET
    );
}

static void W25Q64_Deselect(void)
{
    HAL_GPIO_WritePin(
        FLASH_CS_PORT,
        FLASH_CS_PIN,
        GPIO_PIN_SET
    );
}

HAL_StatusTypeDef W25Q64_ReadJEDECID(uint8_t id[3])
{
    if (id == NULL)
    {
        return HAL_ERROR;
    }

    uint8_t command = W25Q64_CMD_JEDEC_ID;

    W25Q64_Select();

    /* Send JEDEC ID command */
    HAL_StatusTypeDef status =
        HAL_SPI_Transmit(
            &hspi1,
            &command,
            1,
            SPI_TIMEOUT_MS
        );

    if (status == HAL_OK)
    {
        /* Clock in three identification bytes */
        status = HAL_SPI_Receive(
            &hspi1,
            id,
            3,
            SPI_TIMEOUT_MS
        );
    }

    W25Q64_Deselect();

    return status;
}
