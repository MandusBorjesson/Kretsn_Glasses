/*
 * GPIO.h
 *
 *  GPIO header file
 *  Functions:
 *      -- Keep track of button presses on remote control
 *
 *  Created on: 3 May 2019
 *      Author: Mandus
 */

#ifndef GPIO_H_
#define GPIO_H_

#define INT_BTN (1<<3) // Pin X.3

void Eyes_Init()
{
    P2DIR |= RGHT_R | RGHT_G | RGHT_B; // Right eye pins as output
    P3DIR |= LEFT_R | LEFT_G | LEFT_B; // Left eye pins as output
}

void Eyes_Set(char colors)
{
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

#define STATUS_LED_OFF 0x00
#define STATUS_LED_RED 0x01
#define STATUS_LED_GRN 0x02

void Status_Set(char color)
{
    switch (color)
    {
    case STATUS_LED_RED:
    {
        P1DIR |= PIN_INDICATOR;
        P1OUT &= ~PIN_INDICATOR;            // Turn on the red led
        break;
    }
    case STATUS_LED_GRN:
    {
        P1DIR |= PIN_INDICATOR;
        P1OUT |= PIN_INDICATOR;            // Turn on the green led
        break;
    }
    default:
    {
        P1DIR &= ~PIN_INDICATOR;
        break;
    }
    }
}

GPIO_BTN_Init()
{
    P1DIR &= ~INT_BTN;
    P1IE |= INT_BTN; // P1.4 interrupt enabled
    P1IES &= ~INT_BTN;// P1.4 Rising edge
    P1IFG &= ~INT_BTN;// P1.4 IFG cleared
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
}

#endif /* GPIO_H_ */
