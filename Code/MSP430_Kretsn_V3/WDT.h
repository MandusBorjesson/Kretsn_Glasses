/*
 * WDT.h
 *
 *  Created on: 16 Feb 2018
 *      Author: Mandus
 */

#ifndef WDT_H_
#define WDT_H_

void WDT_Init()
{
    UART_DOONCE = 0;
}

// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
    if (UART_DOONCE == 1)
    {
        UART_DOONCE = 0;
    }
    else
    {
        STATUS_VEC |= STATUS_UART;          // Set status bit
        IE1 &= ~WDTIE;                      // Disable WDT interrupt

        __bic_SR_register_on_exit(CPUOFF);                      // Wake up main CPU
    }
}

#endif /* WDT_H_ */
