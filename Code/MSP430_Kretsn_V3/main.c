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
#include "GPIO.h"
#include "TIMER_A0.h"
#include "TIMER_A1.h"
#include "DCO.h"
#include "WDT.h"

unsigned int testData[64] = { IMG_MEM_BASE, 32, 5 };

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // Stop WDT

    DCO_Calib();        // Calibrate DCO
    USCI_A0_Init();     // Initialize UART
    USCI_B0_Init();     // Initialize SPI

    TIMER_A0_Init();    // Initialize timer 0
    TIMER_A1_Init();    // Initialize timer 1
    WDT_Init();         // Initialize WDT
    ADC10_Init();

    Eyes_Init();
    Eyes_Set(0x00); // Eyes off
    Status_Set(STATUS_LED_OFF);

    P2DIR |= CS_MATR | CS_ACCL | OE_MATR;     //Set chip selects as outputs
    P2OUT |= OE_MATR; // Turn off matrix


    n_descriptors = Flash_Count_Descriptors();
    Flash_Load_Descriptor(&img, 0);
    Eyes_Set(img.eyes);

    while (1)
    {
        if ((STATUS_VEC & STATUS_BTN_1) > 0)
        {
            Status_Set(STATUS_LED_OFF);
            Flash_Load_Descriptor(&img, btnIndex);
            Eyes_Set(img.eyes);
            FRAME_CNTR = 0;
            FRAME_OFFS = IMG_MEM_BASE + img.offset;
            btnIndex = 0;
        }

        if ((STATUS_VEC & STATUS_BTN_2) > 0)
        {
            Status_Set(1 << ((btnIndex++) & 0x01));
            if (btnIndex >= n_descriptors)
            {
                btnIndex = 0;
                Status_Set(STATUS_LED_OFF);
            }
        }

        if ((STATUS_VEC & STATUS_UART) > 0)
        {
            handleUART();
            clearUARTbuf();
        }

        STATUS_VEC = 0; // Clear status vector
        __bis_SR_register(CPUOFF + GIE);        // Enter LPM0, enable interrupts
    }
}

