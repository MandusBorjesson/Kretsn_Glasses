/*
 * TIMER_B.h
 *
 *  Timer B header file
 *  Functions:
 *      -- Increment image frame offset
 *      -- Start button samples
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef TIMER_A1_H_
#define TIMER_A1_H_

void TIMER_A1_Init()
{

    TA1CTL = TASSEL_2 | MC_2 | ID_3 | TAIE;  // SMCLK, Continuous Up, div by 8

    TA1CCR0 = 100;                           // TA1CCR0 start value
    TA1CCTL0 |= CCIE;                        // TA1CCR0 interrupt enabled

    TA1CCTL1 |= OUTMOD_4;
    TA1CCTL1 &= ~CCIE;                       // TA1CCR1 interrupt disabled

}

// Timer A0 interrupt vector
#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
    TA1CCR0 += 12500; // add offset, ~10Hz

    static int counter = 0;
    counter++;
    if (counter >= img.period)
    {
        counter = 0;

        STATUS_VEC |= STATUS_FRAME;  // Set Frame update status
        __bic_SR_register_on_exit(CPUOFF);       // Wake up main CPU
    }
    /* Start ADC sampling for the button */
    ADC10CTL0 |= ENC + ADC10SC;
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void T1A1_ISR(void)
{
    switch (TA1IV)
    {
    case 2:
        break;   // CCR1 not used
    case 4:
        break;   // CCR2 not used
    case 10:
        break;   // overflow not used
    }
}

#endif /* TIMER_A1_H_ */
