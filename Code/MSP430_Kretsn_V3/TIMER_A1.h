/*
 * TIMER_B.h
 *
 *  Timer B header file
 *  Functions:
 *      -- Start controller sample
 *      -- Increment time
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef TIMER_A1_H_
#define TIMER_A1_H_

void TIMER_A1_Init()
{
    TA1CCR0 = 100;                           // TA1CCR0 start value
    TA1CCTL0 |= CCIE;                        // TACCR0 interrupt enabled
    TA1CTL = TASSEL_2 | MC_2 | ID_3;         // SMCLK, Continuous Up, div by 8

    TA1CCTL1 &= ~CCIE;                       // CCR1 interrupt disabled

}

// Timer A0 interrupt vector
#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
    TA1CCR0 += 12500;   // add offset, ~10Hz

    static char counter;
    counter++;
    /* Increase time counters */
    sensorData[0] = (counter) & 0x07;      // counter  % 8
    sensorData[1] = (counter >> 1) & 0x07; // counter /2  % 8
    sensorData[2] = (counter >> 2) & 0x07; // counter /4 % 8


/* UART sensor data out
    if (counter == 0 || counter == 127)
    {
        UART_BYTE_COUNTER = 0;
        UCA0TXBUF = 0x89;   // Dummy byte to start UART transmission
        IFG2 &= ~UCA0TXIFG;
        IE2 |= UCA0TXIE;    // Enable TX interrupt
    }
*/

    /* Start ADC sampling for the button */
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion ready
    DTC_MODE = 0;
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void T1A1_ISR(void)
{
    switch (TA1IV)
    {
    case 2:                                 // CCR1, UART timeout
        break;
    case 4:
        break;                              // CCR2 not used
    case 10:
        break;                              // overflow not used
    }
}

#endif /* TIMER_A1_H_ */
