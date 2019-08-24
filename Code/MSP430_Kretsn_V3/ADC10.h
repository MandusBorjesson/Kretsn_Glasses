/*
 * ADC10.h
 *
 *  ADC10 header file
 *  Functions:
 *      -- Sample controller buttons
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef ADC10_H_
#define ADC10_H_

#define BUTTON_TOLERANCE 30 // Tolerance when identifying buttons
char buttonreleased = 1;

void ADC10_Init_BTN()
{
    ADC10CTL0 &= ~ENC;                  // Disable ADC before configuration.
    ADC10CTL0 = ADC10ON;     // Turn ADC on in single line before configuration.

    ADC10CTL0 |= ADC10SHT_2 + ADC10IE;  // ADC10ON, interrupt enabled
    ADC10CTL1 = BTN_INCH;               // input BTN_INCH
}

void ADC10_Init()
{
    P1DIR &= ~(BTN_PIN);
    ADC10_Init_BTN();
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    const char threshold = 15;
    static unsigned char prevSample;
    unsigned char currSample = ADC10MEM >> 2;

    if ((abs(currSample - prevSample) < BUTTON_TOLERANCE) && currSample > threshold && buttonreleased == 1)
    {
        volatile unsigned int BTN_VAL = (currSample + prevSample) >> 1; // Set button value

        if (BTN_VAL < 120)
            STATUS_VEC |= STATUS_BTN_1;       // Set status bits
        else
            STATUS_VEC |= STATUS_BTN_2;       // Set status bits
        buttonreleased = 0;
        __bic_SR_register_on_exit(CPUOFF);       // Wake up main CPU
    }

    if(currSample <= threshold){
        buttonreleased = 1;
    }

    prevSample = currSample;                // Update "old" value
}

#endif /* ADC10_H_ */
