
#ifndef FLASH_TESTS_H
#define FLASH_TESTS_H

/* Existing read-only hardware tests */
void FlashTests_RunAll(void);

/* Destructive test */
void FlashTests_RunWriteErase(void);

/* Read-only persistence check */
void FlashTests_CheckPersistence(void);

#endif /* FLASH_TESTS_H */
