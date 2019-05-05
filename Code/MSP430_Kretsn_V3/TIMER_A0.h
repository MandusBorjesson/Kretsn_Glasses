/*
 * TIMER_A.h
 *
 *  Timer A header file
 *  Functions:
 *      -- Start SPI Transmission for shift registers
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef TIMER_A0_H_
#define TIMER_A0_H_

#define TIMER0_PERIOD 625 // ~1.8kHz (SMCLK/8/F_desired)

void TIMER_A0_Init()
{
    TA0CCR0 = TIMER0_PERIOD;        // Set up CCR0
    TA0CCTL0 |= CCIE;               // TACCR0 interrupt enabled

    TA0CTL = TASSEL_2 | MC_1 | ID_0; // SMCLK, Up to CCR0, div by 8
}

// Timer A0 interrupt vector
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR(void)
{
    ROW_CNTR++;
    if (ROW_CNTR > 7)
    {
        ROW_CNTR = 0;       // Limit row to [0 7]
    }

    char *p = (char*)(FRAME_OFFS + ROW_CNTR * 2);
    SPI_TX_BUF[0] = *(p + 1); // DISP_BUF[ROW_CNTR*2 + 1 + (sensorData[CURR_SENSOR]*16)]; // Put right side in buffer
    SPI_TX_BUF[1] = *p; // DISP_BUF[ROW_CNTR*2     + (sensorData[CURR_SENSOR]*16)]; // Put left side in buffer

    SPI_BYTE_COUNTER = 0;   // Reset byte counter
    UCB0TXBUF = 0xFF;       // Send dummy Byte to initiate SPI transfer

    IE2 |= UCB0TXIE;        // Enable TX interrupt
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void T0A1_ISR(void)
{
    switch (TA0IV)
    {
    case 2:    // CCR1 not used
        break;
    case 4:
        break; // CCR2 not used
    case 10:
        break; // overflow not used
    }
}

#endif /* TIMER_A0_H_ */
