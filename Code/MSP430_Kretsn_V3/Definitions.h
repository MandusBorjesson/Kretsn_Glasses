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

struct imageDescriptor {
   unsigned int offset;     // Address offset of an image/animation in flash
   unsigned int size;       // Size of image
   unsigned char period;    // Sensor dependency
   unsigned char mode;      // Sensor dependency
   unsigned char eyes;
};

#define DESC_SIZE 8

struct imageDescriptor img;

/* GPIO Related variables */
unsigned char btnIndex = 0;

/* UART Settings */
#define UART_TIMEOUT 20000/8 // Sets UART timeout in us
#define UART_BUFSIZE 70      // UART buffer size, in bytes
char UART_DOONCE = 0;

unsigned char UART_Available = 0;
unsigned char UART_Buf[UART_BUFSIZE];

/* Display related constants/variables */
#define MODE_FRAME 1
#define MODE_SWEEP 2
#define NUM_SENSORS 8           //Number of sensors
unsigned int  FRAME_CNTR = 0;   // Current frame that is displayed
unsigned int  FRAME_OFFS = 0;   // Current frame offset
unsigned char ROW_CNTR   = 0;     // Row counter for matrix updating

/* Status definitions for CPU wake-ups */
unsigned int STATUS_VEC = 0;
#define STATUS_BTN_1 (1<<0) // Button press timeout
#define STATUS_BTN_2 (1<<1) // Button pressed
#define STATUS_UART  (1<<2) // UART handling requested


/* Pin mappings */

// PORT 1
#define MIC_PIN         BIT0
#define MIC_INCH        INCH_0
#define BTN_PIN         BIT3
#define BTN_INCH        INCH_3
#define PIN_INDICATOR   BIT4

// PORT 2
#define RGHT_R BIT0
#define RGHT_G BIT1
#define RGHT_B BIT2

#define CS_MATR BIT3
#define OE_MATR BIT4
#define CS_ACCL BIT5

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
