/*
 * FLASH.h
 *
 *  Flash header file
 *  Functions:
 *      -- Read flash to images
 *      -- Write images from display buffer to flash
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */
#ifndef FLASH_H_
#define FLASH_H_

#define FLASH_SEGMENT_SIZE 0x0200 // Smallest erasable partition of memory

unsigned char Flash_Count_Descriptors(void)
{
    char *p = (char*)DESC_MEM_BASE;                          // Flash pointer
    int i;
    for(i= 0; i < MAX_DESCRIPTORS; i++){
        if(p[6] == 0xFF) // If eyes are 0xFF, which they should not be... return index
        {
            return i;
        }
        p += DESC_SIZE;
    }
    return MAX_DESCRIPTORS;
}

void Flash_Erase(unsigned int base)
{
    // Is addressed memory in allowed regions?
     if (!inAllowedMem(base, INFO_SIZE))
         return;

    char *Flash_ptr = (char*)base; // Flash pointer

    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit

    if(base == IMG_MEM_BASE){
    int page;
    for (page = 0; page < IMG_MEM_PAGES; page++)
    {
        *Flash_ptr = 0;                    // Dummy write to erase Flash segment
        Flash_ptr += FLASH_SEGMENT_SIZE; // Increment pointer to next segment base
    }
    } else {
        *Flash_ptr = 0;        // Dummy write to erase Flash segment
    }

    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}


char Flash_Load_Descriptor(struct imageDescriptor *descriptor,
                           unsigned char index)
{
    if (index >= n_descriptors)
        return 0;

    unsigned char* p = (DESC_MEM_BASE + DESC_SIZE * index);

    descriptor->offset = p[1]+(p[0] << 8);
    descriptor->size   = p[3]+(p[2] << 8);
    descriptor->period = p[4];
    descriptor->mode   = p[5];
    descriptor->eyes   = p[6];

    if (descriptor->eyes == 0xFF)
        return 0;

    return 1;
}

void Flash_Erase_Images(void)
{
    char *Flash_ptr = (char*)IMG_MEM_BASE;                          // Flash pointer
//    Flash_ptr = (char *) IMG_MEM_BASE;        // Initialize Flash pointer

    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit

    int page;
    for (page = 0; page < IMG_MEM_PAGES; page++)
    {
        *Flash_ptr = 0;                    // Dummy write to erase Flash segment
        Flash_ptr += FLASH_SEGMENT_SIZE; // Increment pointer to next segment base
    }

    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

char inAllowedMem(unsigned int base, int length)
{
    if ((base >= IMG_MEM_BASE  ) && ((base + length) <= (IMG_MEM_BASE  + IMG_MEM_SIZE  )))
        return 1;

    if ((base >= DESC_MEM_BASE ) && ((base + length) <= (DESC_MEM_BASE + DESC_MEM_SIZE )))
        return 1;

    if ((base >= CFG_MEM_BASE ) && ((base + length) <= (CFG_MEM_BASE + CFG_MEM_SIZE )))
        return 1;

    if ((base >= INFOD_BASE ) && ((base + length) <= (INFOD_BASE + INFOD_SIZE )))
        return 1;

    return 0;
}

void Flash_Write(unsigned int startAddr, unsigned char data[], int dataSize)
{
    // Is addressed memory in allowed regions?
    if (!inAllowedMem(startAddr, dataSize))
        return;

    unsigned char *Flash_ptr;                          // Flash pointer
    unsigned int i;
    Flash_ptr = (unsigned char *) startAddr;           // Initialize Flash pointer

    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
    FCTL3 = FWKEY;                            // Clear Lock bit

    for (i = 0; i < dataSize; i++)
    {
        *(Flash_ptr++) = data[i];               // Write value to flash
    }

    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

#endif /* FLASH_H_ */
