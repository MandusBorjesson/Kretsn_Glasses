/*
 * ADC10.h
 *
 *  ADC10 header file
 *  Functions:
 *      -- Sample controller button
 *      -- Sample microphone using DTC
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef ADC10_H_
#define ADC10_H_

#define BTNSAMPLES 8
unsigned int btnSample[BTNSAMPLES] = {0};     // where the ADC data is put

#define MICSAMPLES 64        // Number of samples taken by the DTC
unsigned int micData[MICSAMPLES] = { 0 };     // Array where the ADC data is put

unsigned char DTC_MODE = 0;  // DTC mode enable

unsigned char index = 0;

#define BUTTON_TOLERANCE 30 // Tolerance when identifying buttons
unsigned int BTN_VAL = 0;   // Analog values for the current button

unsigned char findBtn(unsigned int val)
{
    STATUS_VEC &= ~STATUS_BTNUPD;  // Clear status vector

    unsigned char minIndex = 0;
    unsigned char minDiff = 255;
    unsigned char i;
    for (i = 0; i < N_BTN; i++)
    {
        unsigned char diff = abs(BTN_VOLTS[i] - val);

        if ( diff < minDiff) // Find the button that was pressed
        {
            minDiff = diff;
            minIndex = i;
        }

    }
    return (minIndex>5)? 5:minIndex;

}

unsigned char findVolume()
{
    unsigned int maxVal = 0;
    unsigned int i;
    for (i = 0; i < MICSAMPLES; ++i)
    {
        if (micData[i] > maxVal)
            maxVal = micData[i];
    }

    STATUS_VEC &= ~STATUS_MICUPD;       // Clear status bits
    return (maxVal>230)?1:0;
}

void ADC10_Init_BTN()
{
     ADC10CTL0 &= ~ENC;                  // Disable ADC before configuration.
     ADC10CTL0 = ADC10ON;                // Turn ADC on in single line before configuration.

     ADC10CTL0 |= ADC10SHT_2 + ADC10IE;  // ADC10ON, interrupt enabled
     ADC10CTL1 = BTN_INCH;               // input BTN_INCH
}

void ADC10_Init_MIC()
{
    ADC10CTL0 &= ~ENC;                  // Disable ADC before configuration.
    while (ADC10CTL1 & ADC10BUSY)
        ;      // Make sure the ADC is not running per 22.2.7

    ADC10CTL1 = CONSEQ_2 | MIC_INCH;    // Repeat single channel, A1
    ADC10AE0 = MIC_PIN;                 // MIC_PIN ADC option select

    ADC10DTC1 = MICSAMPLES;             // Only one conversion at a time.
    ADC10SA = (unsigned int) &micData; // Put results at specified place in memory.

    ADC10CTL0 |= ENC | ADC10SC;         // Enable conversion, start conversion
}

void ADC10_Init()
{
    P1DIR &= ~(BTN_PIN|MIC_PIN);
    ADC10_Init_BTN();
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{

    if (DTC_MODE == 1)
    {
        STATUS_VEC |= STATUS_MICUPD;        // Set status bits
        __bic_SR_register_on_exit(CPUOFF);  // Wake up main CPU
    }
    else
    {

        static unsigned char prevSample;
        unsigned char currSample = ADC10MEM >> 2;

        if ((abs(currSample - prevSample) < BUTTON_TOLERANCE) && currSample > 15)
        {
            STATUS_VEC |= STATUS_BTNUPD;       // Set status bits
            BTN_VAL = currSample;               // Set button value
            __bic_SR_register_on_exit(CPUOFF);               // Wake up main CPU
        }

        prevSample = currSample;                // Update "old" value
    }
}

#endif /* ADC10_H_ */
