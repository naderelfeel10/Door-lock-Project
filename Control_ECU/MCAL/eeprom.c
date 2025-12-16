/*****************************************************************************
 * File: eeprom.c
 * Module: EEPROM
 * Description: Source file for TM4C123GH6PM EEPROM Driver (Register Level)
 * Author: Ahmedhh
 * Date: December 14, 2025
 *****************************************************************************/

#include "eeprom.h"
#include "tm4c123gh6pm.h"

/******************************************************************************
 *                          Private Functions                                  *
 ******************************************************************************/

/*
 * EEPROM_WaitDone
 * Waits for EEPROM operation to complete by polling EEDONE register.
 * Returns: EEPROM_SUCCESS if done, EEPROM_TIMEOUT if timeout occurs
 */
static uint8_t EEPROM_WaitDone(void)
{
    uint32_t timeout = 1000000;  /* Timeout counter */
    
    /* Wait for EEPROM to complete operation */
    while((EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING) && (timeout > 0))
    {
        timeout--;
    }
    
    /* Check for timeout */
    if(timeout == 0)
    {
        return EEPROM_TIMEOUT;
    }
    
    /* Check for errors */
    if(EEPROM_EEDONE_R & (EEPROM_EEDONE_INVPL | EEPROM_EEDONE_NOPERM))
    {
        return EEPROM_ERROR;
    }
    
    return EEPROM_SUCCESS;
}

/******************************************************************************
 *                          Public Functions                                   *
 ******************************************************************************/

/*
 * EEPROM_Init
 * Initializes the EEPROM module.
 */
uint8_t EEPROM_Init(void)
{
    uint32_t timeout = 1000000;
    
    /* Enable EEPROM clock */
    SYSCTL_RCGCEEPROM_R |= 0x01;
    
    /* Wait for EEPROM module to be ready */
    while((SYSCTL_RCGCEEPROM_R & 0x01) == 0 && timeout > 0)
    {
        timeout--;
    }
    
    if(timeout == 0)
    {
        return EEPROM_ERROR;
    }
    
    /* Small delay for stability */
    for(timeout = 0; timeout < 6; timeout++);
    
    /* Wait for EEPROM to be ready */
    timeout = 1000000;
    while((EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING) && (timeout > 0))
    {
        timeout--;
    }
    
    if(timeout == 0)
    {
        return EEPROM_ERROR;
    }
    
    /* Check for errors during initialization */
    if(EEPROM_EESUPP_R & 0x0C)  /* Check PRETRY and ERETRY bits */
    {
        return EEPROM_ERROR;
    }
    
    /* Reset EEPROM to start of first block */
    EEPROM_EEBLOCK_R = 0;
    EEPROM_EEOFFSET_R = 0;
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_WriteWord
 * Writes a 32-bit word to EEPROM.
 */
uint8_t EEPROM_WriteWord(uint32_t block, uint32_t offset, uint32_t data)
{
    /* Validate parameters */
    if(block >= EEPROM_TOTAL_BLOCKS || offset >= EEPROM_BLOCK_SIZE)
    {
        return EEPROM_ERROR;
    }
    
    /* Set block and offset */
    EEPROM_EEBLOCK_R = block;
    EEPROM_EEOFFSET_R = offset;
    
    /* Write data */
    EEPROM_EERDWR_R = data;
    
    /* Wait for write to complete */
    return EEPROM_WaitDone();
}

/*
 * EEPROM_ReadWord
 * Reads a 32-bit word from EEPROM.
 */
uint8_t EEPROM_ReadWord(uint32_t block, uint32_t offset, uint32_t *data)
{
    /* Validate parameters */
    if(block >= EEPROM_TOTAL_BLOCKS || offset >= EEPROM_BLOCK_SIZE || data == 0)
    {
        return EEPROM_ERROR;
    }
    
    /* Set block and offset */
    EEPROM_EEBLOCK_R = block;
    EEPROM_EEOFFSET_R = offset;
    
    /* Read data */
    *data = EEPROM_EERDWR_R;
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_WriteBuffer
 * Writes a buffer of bytes to EEPROM.
 */
uint8_t EEPROM_WriteBuffer(uint32_t block, uint32_t offset, const uint8_t *buffer, uint32_t length)
{
    uint32_t i;
    uint32_t word;
    uint32_t current_block = block;
    uint32_t current_offset = offset;
    uint8_t result;
    
    /* Validate parameters */
    if(buffer == 0 || (length % 4) != 0)
    {
        return EEPROM_ERROR;
    }
    
    /* Write data word by word */
    for(i = 0; i < length; i += 4)
    {
        /* Pack 4 bytes into a 32-bit word (little endian) */
        word = (uint32_t)buffer[i] | 
               ((uint32_t)buffer[i+1] << 8) | 
               ((uint32_t)buffer[i+2] << 16) | 
               ((uint32_t)buffer[i+3] << 24);
        
        /* Write word to EEPROM */
        result = EEPROM_WriteWord(current_block, current_offset, word);
        if(result != EEPROM_SUCCESS)
        {
            return result;
        }
        
        /* Move to next word */
        current_offset++;
        if(current_offset >= EEPROM_BLOCK_SIZE)
        {
            current_offset = 0;
            current_block++;
            if(current_block >= EEPROM_TOTAL_BLOCKS)
            {
                return EEPROM_ERROR;  /* Out of EEPROM space */
            }
        }
    }
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_ReadBuffer
 * Reads a buffer of bytes from EEPROM.
 */
uint8_t EEPROM_ReadBuffer(uint32_t block, uint32_t offset, uint8_t *buffer, uint32_t length)
{
    uint32_t i;
    uint32_t word;
    uint32_t current_block = block;
    uint32_t current_offset = offset;
    uint8_t result;
    
    /* Validate parameters */
    if(buffer == 0 || (length % 4) != 0)
    {
        return EEPROM_ERROR;
    }
    
    /* Read data word by word */
    for(i = 0; i < length; i += 4)
    {
        /* Read word from EEPROM */
        result = EEPROM_ReadWord(current_block, current_offset, &word);
        if(result != EEPROM_SUCCESS)
        {
            return result;
        }
        
        /* Unpack 32-bit word into 4 bytes (little endian) */
        buffer[i]   = (uint8_t)(word & 0xFF);
        buffer[i+1] = (uint8_t)((word >> 8) & 0xFF);
        buffer[i+2] = (uint8_t)((word >> 16) & 0xFF);
        buffer[i+3] = (uint8_t)((word >> 24) & 0xFF);
        
        /* Move to next word */
        current_offset++;
        if(current_offset >= EEPROM_BLOCK_SIZE)
        {
            current_offset = 0;
            current_block++;
            if(current_block >= EEPROM_TOTAL_BLOCKS)
            {
                return EEPROM_ERROR;  /* Out of EEPROM space */
            }
        }
    }
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_MassErase
 * Erases the entire EEPROM.
 */
uint8_t EEPROM_MassErase(void)
{
    uint32_t i, j;
    uint8_t result;
    
    /* Erase all blocks by writing 0xFFFFFFFF to all words */
    for(i = 0; i < EEPROM_TOTAL_BLOCKS; i++)
    {
        for(j = 0; j < EEPROM_BLOCK_SIZE; j++)
        {
            result = EEPROM_WriteWord(i, j, 0xFFFFFFFF);
            if(result != EEPROM_SUCCESS)
            {
                return result;
            }
        }
    }
    
    return EEPROM_SUCCESS;
}
