/*
 * Definitions.h
 *
 *  Definitions header file
 *  Functions:
 *      -- Define pins, variables and constants used for the project
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

const char version_data[78] = {"\n_Kretsn Glasses\nHW Rev. 2.1 by Albin Lejaker\nSW Ver. 3.0 by Mandus Borjesson\n"};
const char flash_regions[116] = {"\nAvailable regions:\n0x1000 - info D\n0x1040 - Configuration memory\n0x1080 - Descriptor Memory\n0xEE00 - Image memory\n"};


struct imageDescriptor {
    unsigned int offset;     // Address offset of an image/animation in flash
    unsigned int size;       // Size of image
    unsigned char period;    // Frame period, in 100ms
    unsigned char mode;      // Animation mode
    unsigned char loops;     // Single shot animation
    unsigned char eyes;      // Eye settings
    unsigned char chain;     // Chaining descriptors
};

#define DESC_SIZE 8

/* GPIO Related variables */
unsigned char btnIndex = 0;

/* UART Settings */
#define UART_TIMEOUT 20000/8 // Sets UART timeout in us
#define UART_BUFSIZE 70      // UART buffer size, in bytes
#define UART_CMD_WRT_FLASH  0x7772 // 'wr'
#define UART_CMD_READ_FLASH 0x7265 // 're'
#define UART_CMD_CLR_FLASH  0x6572 // 'er'
#define UART_CMD_IDENTITY   0x6964 // 'id'

char UART_DOONCE = 0;
unsigned char UART_Available = 0;
unsigned char UART_Buf[UART_BUFSIZE];

/* Display related constants/variables */
#define MODE_FRAME 1
#define MODE_SWEEP_UP 2
#define MODE_SWEEP_DN 3
#define MODE_SINGLE (0x01 << 4)
#define NUM_SENSORS 8           //Number of sensors

unsigned int  FRAME_OFFS = 0;   // Current frame offset

/* Status definitions for CPU wake-ups */
#define STATUS_BTN_1 (1<<0) // Button 1 pressed
#define STATUS_BTN_2 (1<<1) // Button 2 pressed
#define STATUS_UART  (1<<2) // UART handling requested
#define STATUS_FRAME (1<<3) // Frame update requested

unsigned int STATUS_VEC = 0;

/* Flash memory map */
#define IMG_MEM_BASE (unsigned int)0xEE00 // Image data address
#define IMG_MEM_SIZE (unsigned int)0x1000 // Image data size, 256 frames
#define IMG_MEM_PAGES IMG_MEM_SIZE/FLASH_SEGMENT_SIZE // Number of pages in flash occupied by images
#define INFO_SIZE  (unsigned int)0x40
#define INFOD_BASE (unsigned int)0x1000
#define INFOD_SIZE INFO_SIZE // INFO D data size
#define CFG_MEM_BASE (unsigned int)0x1040 // Configuration data address
#define CFG_MEM_SIZE INFO_SIZE // Configuration data size
#define DESC_MEM_BASE (unsigned int)0x1080 // Descriptor data address
#define DESC_MEM_SIZE INFO_SIZE // Descriptor data size
#define MAX_DESCRIPTORS 8 // Number of images allowed to be loaded from flash

unsigned char* cfg_memory   = (CFG_MEM_BASE);

/* General settings */
struct imageDescriptor img;
unsigned char n_descriptors = 0;

/* Pin mappings */

// PORT 1
#define BTN_PIN         BIT3
#define BTN_INCH        INCH_3
#define PIN_INDICATOR   BIT4

// PORT 2
#define RGHT_R BIT0
#define RGHT_G BIT1
#define RGHT_B BIT2

#define CS_MATR BIT3
#define OE_MATR BIT4

// PORT 3
#define ACC_INT1 BIT0
#define ACC_INT2 BIT1

#define LEFT_R BIT2
#define LEFT_G BIT3
#define LEFT_B BIT4

#define ROWSEL_1 BIT5
#define ROWSEL_2 BIT6
#define ROWSEL_3 BIT7


#endif /* DEFINITIONS_H_ */
