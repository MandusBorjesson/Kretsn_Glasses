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

/* Flash memory map */
#define IMG_MEM_ADDR (unsigned int)0xFA00 // Image data address
#define IMG_MEM_BUFF (unsigned int)0xFC00 // Image data buffer
#define CFG_MEM_ADDR (unsigned int)0x1080 // Configuration data address

/* Button configuations */
#define N_BTN 6 // Number of analog buttons on controller
//const unsigned int BTN_VOLTS[N_BTN] = { 880, 730, 580, 430, 280, 130 }; // Analog values for the buttons
unsigned char BTN_VOLTS[N_BTN] = { 220, 183, 145, 108, 70, 33 }; // Analog values for the buttons

/* Glass configurations */
char MY_ID;                  // ID for the glasses [0 - 7]
#define CFG_MEM_SIZE 1 + N_BTN
unsigned char CFG_DATA[CFG_MEM_SIZE];   // Configurations stored in flash

/* UART Settings */
#define UART_TIMEOUT 20000/8 // Sets UART timeout in us
#define UART_BUFSIZE 20      // UART buffer size, in bytes
char UART_DOONCE = 0;

unsigned char UART_Available = 0;
unsigned char UART_Buf[UART_BUFSIZE];

/* Display related constants/variables */
#define NUM_SENSORS 8           //Number of sensors
unsigned char sensorData[NUM_SENSORS] = { 0,0,0,1,2,3,4,0}; // Sensor data array [time, time/2, time/4, volume, acc x,y,z, static]
unsigned char CURR_SENSOR = 0;  // Current sensor listened to
unsigned char ROW_CNTR = 0;     // Row counter for matrix updating

/* Display buffer */
#define DISP_BUF_SIZE 131
unsigned char DISP_BUF[DISP_BUF_SIZE] = {0};

/* Status definitions for CPU wake-ups */
unsigned int STATUS_VEC = 0;
#define STATUS_BTNUPD (1<<0) // Button update requested
#define STATUS_UART   (1<<1) // UART handling requested
#define STATUS_MICUPD (1<<2) // Microphone update requested


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

void Eyes_Init(){
    P2DIR |= RGHT_R | RGHT_G | RGHT_B; // Right eye pins as output
    P3DIR |= LEFT_R | LEFT_G | LEFT_B; // Left eye pins as output
}

void Eyes_Set(char colors){
    // Top nibble (4 bits):    Left eye xBGR
    // Bottom nibble (4 bits): Right eye xBGR
    // color format: xBGRxBGR
    colors = colors & 0x77;
    char temp = (colors << 1) & 0b01100110;
    temp |= (colors & 0b01000100) >> 2;
    colors = temp;

    P2OUT |= (RGHT_R | RGHT_G | RGHT_B);
    P3OUT |= (LEFT_R | LEFT_G | LEFT_B);

    P2OUT &= ~(colors & 0x07);
    P3OUT &= ~((colors & 0x70) >> 2);
}

void Status_Set(char color){
    if(color > 0){
        P1OUT &= ~PIN_INDICATOR;
    }else{
        P1OUT |= PIN_INDICATOR;
    }
}

#endif /* DEFINITIONS_H_ */
