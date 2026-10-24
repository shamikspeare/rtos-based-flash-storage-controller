
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


/* Read one flash status register */
static HAL_StatusTypeDef W25Q64_ReadStatus(
    uint8_t command,
    uint8_t *status_byte)
{
    if (status_byte == NULL)
    {
        return HAL_ERROR;
    }

    W25Q64_Select();

    HAL_StatusTypeDef result =
        HAL_SPI_Transmit(
            &hspi1,
            &command,
            1,
            SPI_TIMEOUT_MS
        );

    if (result == HAL_OK)
    {
        result = HAL_SPI_Receive(
            &hspi1,
            status_byte,
            1,
            SPI_TIMEOUT_MS
        );
    }

    /* Always release Chip Select */
    W25Q64_Deselect();

    return result;
}

/* Read Status Register 1 */
HAL_StatusTypeDef W25Q64_ReadStatus1(uint8_t *status)
{
    return W25Q64_ReadStatus(
        W25Q64_CMD_READ_SR1,
        status
    );
}

/* Read Status Register 2 */
HAL_StatusTypeDef W25Q64_ReadStatus2(uint8_t *status)
{
    return W25Q64_ReadStatus(
        W25Q64_CMD_READ_SR2,
        status
    );
}

/* Determine whether flash is busy */
HAL_StatusTypeDef W25Q64_IsBusy(uint8_t *busy)
{
    if (busy == NULL)
    {
        return HAL_ERROR;
    }

    uint8_t status = 0;

    HAL_StatusTypeDef result =
        W25Q64_ReadStatus1(&status);

    if (result == HAL_OK)
    {
        *busy = (status & W25Q64_SR1_BUSY) ? 1U : 0U;
    }

    return result;
}

/* Wait until flash finishes its internal operation */
HAL_StatusTypeDef W25Q64_WaitWhileBusy(
    uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (1)
    {
        uint8_t busy = 0;

        HAL_StatusTypeDef result =
            W25Q64_IsBusy(&busy);

        if (result != HAL_OK)
        {
            return result;
        }

        if (busy == 0U)
        {
            return HAL_OK;
        }

        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            return HAL_TIMEOUT;
        }

        HAL_Delay(1);
    }
}


/*
 * Read bytes from physical W25Q64FV flash.
 *
 * address : starting byte address
 * buffer  : destination in STM32 RAM
 * length  : number of bytes to read
 */
HAL_StatusTypeDef W25Q64_Read(
    uint32_t address,
    uint8_t *buffer,
    uint32_t length)
{
    /* Validate arguments */
    if ((buffer == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    /* Prevent invalid addresses and overflow */
    if (address >= W25Q64_FLASH_SIZE)
    {
        return HAL_ERROR;
    }

    if (length > (W25Q64_FLASH_SIZE - address))
    {
        return HAL_ERROR;
    }

    /* Wait until internal flash operations finish */
    HAL_StatusTypeDef result =
        W25Q64_WaitWhileBusy(1000U);

    if (result != HAL_OK)
    {
        return result;
    }

    /* Command + 24-bit address (MSB first) */
    uint8_t header[4] =
    {
        W25Q64_CMD_READ_DATA,
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)(address)
    };

    W25Q64_Select();

    result = HAL_SPI_Transmit(
        &hspi1,
        header,
        sizeof(header),
        SPI_TIMEOUT_MS
    );

    /* Read data in manageable chunks */
    uint32_t offset = 0U;

    while ((result == HAL_OK) && (offset < length))
    {
        uint32_t remaining = length - offset;

        uint16_t chunk = (remaining > 256U)
                       ? 256U
                       : (uint16_t)remaining;

        result = HAL_SPI_Receive(
            &hspi1,
            &buffer[offset],
            chunk,
            SPI_TIMEOUT_MS
        );

        offset += chunk;
    }

    /* Release flash, including on SPI failure */
    W25Q64_Deselect();

    return result;
}


/* ------------------------------------------
 * Phase 3.5.1: Write Enable
 * ------------------------------------------ */

HAL_StatusTypeDef W25Q64_WriteEnable(void)
{
    uint8_t command = W25Q64_CMD_WRITE_ENABLE;
    uint8_t status = 0U;

    /* Flash must be ready first */
    HAL_StatusTypeDef result =
        W25Q64_WaitWhileBusy(1000U);

    if (result != HAL_OK)
    {
        return result;
    }

    W25Q64_Select();

    result = HAL_SPI_Transmit(
        &hspi1,
        &command,
        1U,
        SPI_TIMEOUT_MS
    );

    W25Q64_Deselect();

    if (result != HAL_OK)
    {
        return result;
    }

    /* Verify that WEL was actually set */
    result = W25Q64_ReadStatus1(&status);

    if (result != HAL_OK)
    {
        return result;
    }

    if ((status & W25Q64_SR1_WEL) == 0U)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}


/* ------------------------------------------
 * Phase 3.5.2: Sector Erase
 * ------------------------------------------ */

HAL_StatusTypeDef W25Q64_EraseSector(
    uint32_t address)
{
    /* Phase 3.5 safety restriction */
    if (address != W25Q64_TEST_SECTOR_ADDR)
    {
        return HAL_ERROR;
    }

    /* Sector address must be 4 KB aligned */
    if ((address % W25Q64_SECTOR_SIZE) != 0U)
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef result =
        W25Q64_WriteEnable();

    if (result != HAL_OK)
    {
        return result;
    }

    uint8_t command[4] =
    {
        W25Q64_CMD_SECTOR_ERASE,
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)address
    };

    W25Q64_Select();

    result = HAL_SPI_Transmit(
        &hspi1,
        command,
        sizeof(command),
        SPI_TIMEOUT_MS
    );

    W25Q64_Deselect();

    if (result != HAL_OK)
    {
        return result;
    }

    /* Internal erase continues after SPI ends */
    return W25Q64_WaitWhileBusy(
        W25Q64_ERASE_TIMEOUT_MS
    );
}


/* ------------------------------------------
 * Phase 3.5.3: Page Program
 * ------------------------------------------ */

HAL_StatusTypeDef W25Q64_PageProgram(
    uint32_t address,
    const uint8_t *data,
    uint32_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    /* Maximum one page per command */
    if (length > W25Q64_PAGE_SIZE)
    {
        return HAL_ERROR;
    }

    /* Check physical address range */
    if (address >= W25Q64_FLASH_SIZE)
    {
        return HAL_ERROR;
    }

    if (length > (W25Q64_FLASH_SIZE - address))
    {
        return HAL_ERROR;
    }

    /* Prevent writes crossing a page boundary */
    uint32_t page_offset =
        address % W25Q64_PAGE_SIZE;

    if (length > (W25Q64_PAGE_SIZE - page_offset))
    {
        return HAL_ERROR;
    }

    /* Phase 3.5: permit only the test sector */
    if (address < W25Q64_TEST_SECTOR_ADDR)
    {
        return HAL_ERROR;
    }

    /* Write Enable is required per program */
    HAL_StatusTypeDef result =
        W25Q64_WriteEnable();

    if (result != HAL_OK)
    {
        return result;
    }

    uint8_t header[4] =
    {
        W25Q64_CMD_PAGE_PROGRAM,
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)address
    };

    W25Q64_Select();

    result = HAL_SPI_Transmit(
        &hspi1,
        header,
        sizeof(header),
        SPI_TIMEOUT_MS
    );

    if (result == HAL_OK)
    {
        result = HAL_SPI_Transmit(
            &hspi1,
            (uint8_t *)data,
            (uint16_t)length,
            SPI_TIMEOUT_MS
        );
    }

    W25Q64_Deselect();

    if (result != HAL_OK)
    {
        return result;
    }

    return W25Q64_WaitWhileBusy(
        W25Q64_PROGRAM_TIMEOUT_MS
    );
}


