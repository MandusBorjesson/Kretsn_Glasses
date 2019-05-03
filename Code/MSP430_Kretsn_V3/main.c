/*
 * Kontrollerkod till kretsn-brillor version 2
 * Skriven till kretsn 7E2 (?)
 *
 * Mandus Börjesson, 2018
 *
 * ------------------|
 *  MSP430G2553      |
 *  QFN package      |
 *  3 ports, 24 GPIO |
 *                   |
 *            P1.0   | < Microphone analog signal
 *            P1.1   | < UART RX
 *            P1.2   | > UART TX (Unused)
 *            P1.3   | < Button analog signal
 *            P1.4   | > Indicator led (back side)
 *            P1.5   | > SPI CLK
 *            P1.6   | < SPI MISO
 *            P1.7   | > SPI MOSI
 *                   |
 *            P2.0   | > Right eye B
 *            P2.1   | > Right eye R
 *            P2.2   | > Right eye G
 *            P2.3   | > Chip select, matrix
 *            P2.4   | > Output enable, matrix
 *            P2.5   | > Chip select, Accelerometer
 *            P2.6   | < XIN   (Crystal oscillator)
 *            P2.7   | < XOUT  (Crystal oscillator)
 *                   |
 *            P3.0   | < Accelerometer interrupt 1
 *            P3.1   | < Accelerometer interrupt 2
 *            P3.2   | > Left eye B
 *            P3.3   | > Left eye R
 *            P3.4   | > Left eye G
 *            P3.5   | > Row select address 1
 *            P3.6   | > Row select address 2
 *            P3.7   | > Row select address 3
 * ------------------|
 */

/* --- Image array format ---
 *
 * Index:   [ Slot contains    ]    Value meaning
 *
 * 0:       [ Image data       ]    Row data to be displayed
 * 1:       [ Image data       ]    Row data to be displayed
 * 2:       [        :         ]
 * 3:                :
 *                   :
 * 128:     [ Eye color        ]    Bits 7-4 : 1-> static, others->dynamic Bits 3-0: image index
 * 129:     [ Image dependence ]    -> currSensor
 */

/* --- sensorData array format ---
 *
 *  All values range from 0 to 7, representing
 *  one slot each in dynamic image arrays.
 *
 * Index:   [ Slot contains   ]    Description
 *
 * 0:       [ Fast time       ]    ~10Hz
 * 1:       [ Medium Time     ]    1/2 Fast time
 * 2:       [ Slow time       ]    1/4 Fast time
 * 3:       [ Volume          ]    Incoming sound volume
 * 4:       [ Accelerometer x ]
 * 5:       [ Accelerometer y ]
 * 6:       [ Accelerometer z ]
 * 7:       [ 0               ]    Used for static images
 */

/* --- Flash structure ---
 *
 * Adress:        [ Slot contains     ]   Size (Bytes)
 *
 * IMG_MEM_ADDR:  [ All images        ]   512
 * CFG_MEM_ADDR:  [ Configuration data]   64
 *
 */

#include <msp430g2553.h>
#include "Definitions.h"

#include "FLASH.h"
#include "ADC10.h"
#include "USCI_AB.h"
#include <TIMER_A0.h>
#include <TIMER_A1.h>
#include "DCO.h"
#include "WDT.h"

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // Stop WDT

    Eyes_Init();
    Eyes_Set(0xFF); // Eyes white

    P2DIR |= CS_MATR | CS_ACCL | OE_MATR;     //Set chip selects as outputs
    P2OUT |= OE_MATR; // Turn off matrix

    unsigned long int i;
    for (i = 0; i < 200000; i++){} // Delay loop

    Flash_Read(CFG_MEM_ADDR, CFG_MEM_SIZE, CFG_DATA); // Read glasses configuration from flash

    MY_ID = CFG_DATA[0];

    for(i = 0; i < N_BTN; i++){
        BTN_VOLTS[i] = CFG_DATA[i+1]; // Load button data
    }

    Eyes_Set(MY_ID | (MY_ID << 4)); // Set eyes to display ID
    for (i = 0; i < 200000; i++){} // Delay loop
    Eyes_Set(0x00); // Eyes off


    DCO_Calib();        // Calibrate DCO
    USCI_A0_Init();     // Initialize UART
    USCI_B0_Init();     // Initialize SPI
    ADC10_Init();       // Initialize ADC

    TIMER_A0_Init();    // Initialize timer 0
    TIMER_A1_Init();    // Initialize timer 1
    WDT_Init();         // Initialize WDT

    P1DIR |= PIN_INDICATOR;
    P1OUT &= ~PIN_INDICATOR;            // Turn on the red led

    while (1)
    {
        if ((STATUS_VEC & STATUS_BTNUPD) > 0)
        {
            index = findBtn(BTN_VAL);
            Status_Set((index > 2) ? 1:0);

            CURR_SENSOR = FlashLoad(index, DISP_BUF);
            if(index < 3){
                Eyes_Set(DISP_BUF[128]); // Set eyes to display RED
            } else {
                Eyes_Set(DISP_BUF[16]); // Set eyes to display RED
            }
        }

        if ((STATUS_VEC & STATUS_MICUPD) > 0)
        {
            sensorData[3] = findVolume();
        }

        if ((STATUS_VEC & STATUS_UART) > 0)
        {
            handleUART();
            clearUARTbuf();
        }
        __bis_SR_register(CPUOFF + GIE);        // Enter LPM0, enable interrupts
    }
}

