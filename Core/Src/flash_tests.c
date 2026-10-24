
#include "flash_tests.h"
#include "w25q64.h"

#include <stdio.h>
#include <string.h>

/* USART2 handle is initialized in main.c */
extern UART_HandleTypeDef huart2;

#define TEST_UART_TIMEOUT_MS 1000U
#define FLASH_TEST_LENGTH    16U



static void Test_Print(const char *text)
{
    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)text,
        (uint16_t)strlen(text),
        TEST_UART_TIMEOUT_MS
    );
}

/* Test 1: JEDEC identification */
static uint8_t Test_JEDEC(void)
{
    uint8_t id[3] = {0};
    char message[80];

    Test_Print("\r\n--- JEDEC ID Test ---\r\n");

    if (W25Q64_ReadJEDECID(id) != HAL_OK)
    {
        Test_Print("ERROR: JEDEC SPI transfer\r\n");
        return 0U;
    }

    snprintf(message, sizeof(message),
             "JEDEC ID: %02X %02X %02X\r\n",
             id[0], id[1], id[2]);

    Test_Print(message);

    if ((id[0] == 0xEFU) &&
        (id[1] == 0x40U) &&
        (id[2] == 0x17U))
    {
        Test_Print("JEDEC Test: PASSED\r\n");
        return 1U;
    }

    Test_Print("JEDEC Test: FAILED\r\n");
    return 0U;
}

/* Test 2: Status registers and readiness */
static uint8_t Test_StatusRegisters(void)
{
    uint8_t sr1 = 0U;
    uint8_t sr2 = 0U;
    char message[80];

    Test_Print("\r\n--- Status Register Test ---\r\n");

    if (W25Q64_ReadStatus1(&sr1) != HAL_OK)
    {
        Test_Print("ERROR: SR1 read failed\r\n");
        return 0U;
    }

    if (W25Q64_ReadStatus2(&sr2) != HAL_OK)
    {
        Test_Print("ERROR: SR2 read failed\r\n");
        return 0U;
    }

    snprintf(message, sizeof(message),
             "SR1: 0x%02X  SR2: 0x%02X\r\n",
             sr1, sr2);

    Test_Print(message);

    snprintf(message, sizeof(message),
             "BUSY: %u  WEL: %u\r\n",
             (unsigned)((sr1 & W25Q64_SR1_BUSY) != 0U),
             (unsigned)((sr1 & W25Q64_SR1_WEL) != 0U));

    Test_Print(message);

    HAL_StatusTypeDef result =
        W25Q64_WaitWhileBusy(1000U);

    if (result == HAL_OK)
    {
        Test_Print("Ready Check: PASSED\r\n");
        return 1U;
    }

    Test_Print("Ready Check: FAILED\r\n");
    return 0U;
}

/* Test 3: Read 16 bytes from physical flash */
static uint8_t Test_FlashRead(void)
{
    uint8_t data[FLASH_TEST_LENGTH];
    uint8_t second_read[FLASH_TEST_LENGTH];
    char message[80];

    Test_Print("\r\n--- Physical Flash Read Test ---\r\n");
    Test_Print("Address: 0x000000\r\n");

    HAL_StatusTypeDef result =
        W25Q64_Read(0x000000U,
                    data,
                    sizeof(data));

    if (result != HAL_OK)
    {
        Test_Print("Flash Read: SPI ERROR\r\n");
        return 0U;
    }

    Test_Print("Data:\r\n");

    for (uint32_t i = 0U; i < FLASH_TEST_LENGTH; i++)
    {
        snprintf(message, sizeof(message),
                 "%02X ", data[i]);

        Test_Print(message);

        if ((i + 1U) % 8U == 0U)
        {
            Test_Print("\r\n");
        }
    }

    /* Read same address again */
    result = W25Q64_Read(
        0x000000U,
        second_read,
        sizeof(second_read)
    );

    if (result != HAL_OK)
    {
        Test_Print("Second Read: SPI ERROR\r\n");
        return 0U;
    }

    if (memcmp(data, second_read, sizeof(data)) != 0)
    {
        Test_Print("Repeat Read: MISMATCH\r\n");
        return 0U;
    }

    Test_Print("Repeat Read: MATCH\r\n");
    Test_Print("Flash Read Test: PASSED\r\n");

    return 1U;
}

/* Main test entry point */
void FlashTests_RunAll(void)
{
    Test_Print("\r\n============================\r\n");
    Test_Print("W25Q64FV HARDWARE TESTS\r\n");
    Test_Print("============================\r\n");

    /* Stop if the expected flash is not detected */
    if (Test_JEDEC() == 0U)
    {
        Test_Print("Tests stopped: JEDEC failure\r\n");
        return;
    }

    if (Test_StatusRegisters() == 0U)
    {
        Test_Print("Tests stopped: Status failure\r\n");
        return;
    }

    if (Test_FlashRead() == 0U)
    {
        Test_Print("Tests stopped: Read failure\r\n");
        return;
    }

    Test_Print("\r\nAll basic flash tests PASSED\r\n");
}


/* ==========================================
 * Phase 3.5: Write / Erase / Verify Tests
 * ========================================== */

static const uint8_t test_pattern[4] =
{
    0xA5U, 0x5AU, 0x12U, 0x34U
};

/* Verify the complete 4 KB sector is erased */
static uint8_t Test_VerifyErasedSector(void)
{
    uint8_t buffer[64];

    for (uint32_t offset = 0U;
         offset < W25Q64_SECTOR_SIZE;
         offset += sizeof(buffer))
    {
        HAL_StatusTypeDef result =
            W25Q64_Read(
                W25Q64_TEST_SECTOR_ADDR + offset,
                buffer,
                sizeof(buffer)
            );

        if (result != HAL_OK)
        {
            Test_Print("Erase verify: SPI ERROR\r\n");
            return 0U;
        }

        for (uint32_t i = 0U; i < sizeof(buffer); i++)
        {
            if (buffer[i] != 0xFFU)
            {
                Test_Print("Erase verify: MISMATCH\r\n");
                return 0U;
            }
        }
    }

    Test_Print("Erase verification: PASSED\r\n");
    return 1U;
}


/* Read back and compare programmed data */
void FlashTests_CheckPersistence(void)
{
    uint8_t readback[sizeof(test_pattern)] = {0};
    char message[80];

    Test_Print("\r\n--- Flash Persistence Check ---\r\n");

    HAL_StatusTypeDef result =
        W25Q64_Read(
            W25Q64_TEST_SECTOR_ADDR,
            readback,
            sizeof(readback)
        );

    if (result != HAL_OK)
    {
        Test_Print("Readback: SPI ERROR\r\n");
        return;
    }

    snprintf(message,
             sizeof(message),
             "Readback: %02X %02X %02X %02X\r\n",
             readback[0],
             readback[1],
             readback[2],
             readback[3]);

    Test_Print(message);

    if (memcmp(readback,
               test_pattern,
               sizeof(test_pattern)) == 0)
    {
        Test_Print("Verification: PASSED\r\n");
    }
    else
    {
        Test_Print("Verification: FAILED\r\n");
    }
}


/* Explicitly triggered destructive hardware test */
void FlashTests_RunWriteErase(void)
{
    uint8_t id[3] = {0};
    uint8_t sr1 = 0U;
    uint8_t sr2 = 0U;

    Test_Print("\r\n=== PHASE 3.5 WRITE/ERASE TEST ===\r\n");
    Test_Print("Target: 0x7FF000 - 0x7FFFFF\r\n");

    /* Identify the expected chip before modifying it */
    if (W25Q64_ReadJEDECID(id) != HAL_OK ||
        id[0] != 0xEFU ||
        id[1] != 0x40U ||
        id[2] != 0x17U)
    {
        Test_Print("ERROR: Unexpected flash device\r\n");
        return;
    }

    /* Check protection configuration */
    if (W25Q64_ReadStatus1(&sr1) != HAL_OK ||
        W25Q64_ReadStatus2(&sr2) != HAL_OK)
    {
        Test_Print("ERROR: Status read failed\r\n");
        return;
    }

    /* Reject non-default array protection settings */
    if ((sr1 & 0x7CU) != 0U ||
        (sr2 & 0x40U) != 0U)
    {
        Test_Print("ERROR: Flash protection active\r\n");
        return;
    }

    /* Phase 3.5.1: Verify Write Enable */
    if (W25Q64_WriteEnable() != HAL_OK)
    {
        Test_Print("Write Enable: FAILED\r\n");
        return;
    }

    Test_Print("Write Enable: PASSED (WEL = 1)\r\n");

    /* Phase 3.5.2: Erase the dedicated sector */
    Test_Print("Erasing test sector...\r\n");

    if (W25Q64_EraseSector(
            W25Q64_TEST_SECTOR_ADDR) != HAL_OK)
    {
        Test_Print("Sector Erase: FAILED\r\n");
        return;
    }

    if (Test_VerifyErasedSector() == 0U)
    {
        return;
    }

    /* Phase 3.5.3: Program four bytes */
    Test_Print("Programming: A5 5A 12 34\r\n");

    if (W25Q64_PageProgram(
            W25Q64_TEST_SECTOR_ADDR,
            test_pattern,
            sizeof(test_pattern)) != HAL_OK)
    {
        Test_Print("Page Program: FAILED\r\n");
        return;
    }

    Test_Print("Page Program: Command completed\r\n");

    /* Phase 3.5.4: Verify programmed contents */
    FlashTests_CheckPersistence();
}



