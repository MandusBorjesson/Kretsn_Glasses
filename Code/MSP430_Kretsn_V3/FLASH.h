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

#define FLASH_DYN_SIZE 130  // Dynamic img allocated size (2*8*8 bytes image + 2 byte config)
#define FLASH_STA_SIZE 18  // Static img allocated size (2*8 byte + 2 byte config)
#define FLASH_IMAGE_MEMORY_SIZE 512 // Image memory size, in bytes

const int FLASH_IMAGE_OFFSET[6] = { 0, FLASH_DYN_SIZE, FLASH_DYN_SIZE * 2,FLASH_DYN_SIZE * 3, FLASH_DYN_SIZE * 3 + FLASH_STA_SIZE, FLASH_DYN_SIZE * 3 + FLASH_STA_SIZE * 2 };

#if (FLASH_DYN_SIZE*3 + FLASH_STA_SIZE*3) > FLASH_IMAGE_MEMORY_SIZE
#error "Total image allocated memory too large"
#endif

void Flash_Write_Start(int* startAddr)
{
    char *Flash_ptr;                          // Flash pointer

    Flash_ptr = (char *) startAddr;           // Initialize Flash pointer
    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit
    *Flash_ptr = 0;                        // Dummy write to erase Flash segment

    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
}

void Flash_Write(unsigned int* startAddr, int dataSize, unsigned char* data)
{
    char *Flash_ptr;                          // Flash pointer
    unsigned int i;
    Flash_ptr = (char *) startAddr;           // Initialize Flash pointer

    for (i = 0; i < dataSize; i++)
    {
        *Flash_ptr++ = *data++;               // Write value to flash
    }
}

void Flash_Write_End()
{
    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void Flash_Read(unsigned int* startAddr, int dataSize, unsigned char data[])
{
    char *Flash_ptr;                  // Flash pointer
    unsigned int i;

    Flash_ptr = (char *) startAddr;   // Initialize Flash pointer

    for (i = 0; i < dataSize; i++)
    {
        data[i] = *Flash_ptr++;       // Read value from flash
    }
}

/* FlashImages(image[])
 * Storeas an image to flash
 *
 * image[]: Array with the image to be stored.
 *
 */
void FlashImages(unsigned char image[])
{
    Flash_Write_Start (IMG_MEM_ADDR); // Start writing to address

    char imgIndex = ((image[130]&0x10)>>4)*3; // = 4 if dynamic, 0 if static
    imgIndex += (image[130] & 0x03); // + actual index [0-2]
    unsigned int i;
    for (i = 0; i < 6; i++)
    {
        if (i == imgIndex)
        {
            Flash_Write(IMG_MEM_ADDR + FLASH_IMAGE_OFFSET[i],
                        (i < 3) ? (FLASH_DYN_SIZE) : (FLASH_STA_SIZE), image);
        }
        else
        {
            Flash_Write(IMG_MEM_ADDR + FLASH_IMAGE_OFFSET[i],
                        (i < 3) ? FLASH_DYN_SIZE : FLASH_STA_SIZE,
                        (IMG_MEM_BUFF + FLASH_IMAGE_OFFSET[i]));
        }
    }
    Flash_Write_End();

    Flash_Write_Start (IMG_MEM_BUFF);
    Flash_Write(IMG_MEM_BUFF, FLASH_IMAGE_MEMORY_SIZE, IMG_MEM_ADDR);
    Flash_Write_End();
}
;


/* FlashLoad(index, image[])
 * Reads an image from flash to image[] array
 *
 * index: image index ( 0-2: dynamic, 1-5: static)
 * image[]: Array where the read image is stored.
 */
unsigned char FlashLoad(unsigned int index, unsigned char image[]){
    Flash_Read(IMG_MEM_ADDR + FLASH_IMAGE_OFFSET[index], (index < 3) ? (FLASH_DYN_SIZE) : (FLASH_STA_SIZE), image); // Read dynamic image number 0.

    unsigned char sensor = ((image[129]) >= NUM_SENSORS)?NUM_SENSORS-1:image[129]; // Return sensor dependence
    if(index >= 3) sensor = NUM_SENSORS-1;
    return sensor;
}
#endif /* FLASH_H_ */
