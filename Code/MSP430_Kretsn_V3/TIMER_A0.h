/*
 * TIMER_A.h
 *
 *  Timer A header file
 *  Functions:
 *      -- Nothing
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef TIMER_A0_H_
#define TIMER_A0_H_

#define TIMER0_PERIOD 625 // ~1.8kHz

void TIMER_A0_Init()
{
    TA0CCR0 = TIMER0_PERIOD;        // Set up CCR0
    //TA0CCTL0 |= CCIE;               // TACCR0 interrupt enabled

    TA0CTL = TASSEL_1 | MC_1 | ID_0; // SMCLK, Up to CCR0, div by 8
}

// Timer A0 interrupt vector
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR(void)
{

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
