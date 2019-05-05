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

/* Flash memory map */
#define IMG_MEM_BASE (unsigned int)0xEE00 // Image data address
#define IMG_MEM_SIZE (unsigned int)0x1000 // Image data size, 256 frames

#define IMG_MEM_PAGES IMG_MEM_SIZE/FLASH_SEGMENT_SIZE // Number of pages in flash occupied by images

#define DESC_MEM_BASE (unsigned int)0x1080 // Configuration data address
#define DESC_MEM_SIZE (unsigned int)0x0040 // Configuration data size

#define MAX_DESCRIPTORS 8 // Number of images allowed to be loaded from flash
unsigned char n_descriptors = 0;

unsigned char Flash_Count_Descriptors(void)
{
    char *p = (char*)DESC_MEM_BASE;                          // Flash pointer
    int i;
    for(i= 0; i < MAX_DESCRIPTORS; i++){
        if(p[5] == 0xFF){
            return i;
        }
        p += DESC_SIZE;
    }
    return MAX_DESCRIPTORS;
}


void Flash_Erase_Descriptors(void)
{
    char *Flash_ptr = (char*)DESC_MEM_BASE;                          // Flash pointer
//    Flash_ptr = ;        // Initialize Flash pointer

    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit

    *Flash_ptr = 0;                        // Dummy write to erase Flash segment

    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

char Flash_Load_Descriptor(struct imageDescriptor *descriptor,
                           unsigned char index)
{
    if (index >= n_descriptors)
        return -1;

    unsigned char* p = (DESC_MEM_BASE + DESC_SIZE * index);

    descriptor->offset = p[0]+(p[1] << 8);
    descriptor->size   = p[2]+(p[3] << 8);
    descriptor->period = p[4];
    descriptor->mode   = p[5];
    descriptor->eyes   = p[6];

    if (descriptor->period == 0xFFFF)
        return -1;

    return 0;
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

char inImgMem(unsigned int* base, int length)
{
    if ((*base < IMG_MEM_BASE )
            || ((*base + length) > (IMG_MEM_BASE + IMG_MEM_SIZE )))
        return 0;
    return 1;
}
char inDescMem(unsigned int* base, int length)
{
    if ((*base < DESC_MEM_BASE )
            || ((*base + length) > (DESC_MEM_BASE + DESC_MEM_SIZE )))
        return 0;
    return 1;
}

void Flash_Write(unsigned int startAddr, unsigned char data[], int dataSize)
{
    // Is addressed memory in allowed regions?
    if (!(inImgMem(startAddr, dataSize) || inDescMem(startAddr, dataSize)))
        return;

    unsigned char *Flash_ptr;                          // Flash pointer
    unsigned int i;
    Flash_ptr = (unsigned char *) startAddr;           // Initialize Flash pointer

    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
    FCTL3 = FWKEY;                            // Clear Lock bit

    for (i = 0; i < dataSize; i++)
    {
        *(Flash_ptr++) = data[i+6];               // Write value to flash
    }

    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

#endif /* FLASH_H_ */
