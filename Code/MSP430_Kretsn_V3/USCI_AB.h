/*
 * USCI_AB.h
 *
 *  USCI A/B header file
 *  Functions:
 *      -- USCI_A0: receive data over UART
 *      -- USCI_B0: Transmit/Receive data over SPI
 *          - Send data to Shift registers
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef USCI_AB_H_
#define USCI_AB_H_

#define UART_CMD_CLR_IM    0x636C
#define UART_CMD_CLR_DESC  0x6465
#define UART_CMD_WRT_FLASH 0x7772
#define UART_CMD_IDENTITY  0x6964

#define UART_RX BIT1
#define UART_TX BIT2
unsigned int UART_BYTE_CNTR = 0;

#define SPIMOSI BIT7
#define SPIMISO BIT6
#define SPICLK  BIT5
#define SPI_PINS SPIMOSI | SPIMISO | SPICLK

unsigned char BYTE_CNTR   = 0;     // Byte counter for matrix updating


void USCI_A0_Init()
{
    P1SEL = UART_RX | UART_TX;                // P1.1 = RXD, P1.2=TXD
    P1SEL2 = UART_RX | UART_TX;               // P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600 Baud
    UCA0BR1 = 0x00;                           // 1MHz 9600 Baud
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE | UCA0TXIE;               // Enable USCI_A0 RX interrupt
}

void USCI_B0_Init()
{
    P1SEL |= SPI_PINS;                          // Assign SPI pins to USCI_B0
    P1SEL2 |= SPI_PINS;                         // Assign SPI pins to USCI_B0

    P2DIR |= CS_MATR | OE_MATR;                 //Set chip selects as outputs
    P3DIR |= ROWSEL_1 | ROWSEL_2 | ROWSEL_3;    // Row select pins
    P3OUT &= ~(ROWSEL_1 | ROWSEL_2 | ROWSEL_3);

    UCB0CTL1 |= UCSWRST;                        // Enable SW reset

    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; // Leading edge phase, MSB first, SPI Master, 3-wire SPI, synchronous mode
    UCB0CTL1 = UCSSEL_2 | UCSWRST;              // Use SMCLK, keep SW reset

    UCB0BR0 = 100;                              // fSCL = SMCLK/255 = ~4kHz <- SPI clock speed
                                                // 128 bits/frame, 4000/128 ~30fps
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset, resume operation
    IE2 |= UCB0RXIE;                            // Enable RX interrupt

    P2OUT &= ~OE_MATR;                          // Turn on matrix
    UCB0TXBUF = 0xFF;                           // Kick off SPI infinity madness

}

void handleUART()
{
    unsigned int UART_CMD = UART_Buf[0] << 8 | UART_Buf[1];

    switch (UART_CMD)
    {
    case UART_CMD_CLR_IM:
    {
        Flash_Erase_Images();
        break;
    }
    case UART_CMD_CLR_DESC:
    {
        Flash_Erase_Descriptors();
        break;
    }
    case UART_CMD_WRT_FLASH:
    {
        unsigned int base = (UART_Buf[2] << 8 | UART_Buf[3]);
        unsigned char length = UART_Buf[4];
        Flash_Write(base, UART_Buf, length);
        break;
    }
    case UART_CMD_IDENTITY:
    {
        UART_Buf[0] = IMG_MEM_SIZE >> 8;
        UART_Buf[1] = IMG_MEM_SIZE & 0xFF;
        UART_Buf[2] = UART_BUFSIZE >> 8;
        UART_Buf[3] = UART_BUFSIZE & 0xFF;

        UART_BYTE_CNTR = 4;

        UCA0TXBUF = 0xFF; // Kick off transmission
        break;
    }
    }
}

void clearUARTbuf()
{
    unsigned int i;
    for (i = UART_BUFSIZE - 1; i >= 0; i--)
    {
        UART_Buf[i] = 0;
    }
    UART_Available = 0;
}
// USCI A/B Transmit done interrupt vector
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    if ((IFG2 & UCA0TXIFG) > 0) //UART Transmit buffer ready
    {
        if (UART_BYTE_CNTR > 0)
        {
            UCA0TXBUF = UART_Buf[--UART_BYTE_CNTR];
        }
        IFG2 &= ~UCA0TXIFG;
    }

    if ((IFG2 & UCB0TXIFG) > 0) //SPI Transmit buffer ready
    {

    }
}

// USCI A/B receive interrupt vector
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{

    if ((IFG2 & UCA0RXIFG) > 0) //UART triggered interrupt
    {
        // Buffer data in receive array
        UART_Buf[UART_Available] = UCA0RXBUF;

        if (UART_Available < UART_BUFSIZE)
        {
            UART_Available++;
        }

        UART_DOONCE = 1;
        WDTCTL = WDT_MDLY_32;            // Set Watchdog Timer interval to ~30ms
        IE1 |= WDTIE;   // Enable WDT interrupt
    }

    if ((IFG2 & UCB0RXIFG) > 0) //SPI triggered RX interrupt, meaning that TX is done
    {

        // Which side was sent in last transfer?
        if ((BYTE_CNTR & 0x01) == 0x01) // Odd counter => Left eye => update matrix and row selection pins
        {
            P2OUT |= OE_MATR;   // Turn off matrix
            P2OUT |= CS_MATR;   // Load shift register storage registers

            P3OUT &= ~(ROWSEL_1 | ROWSEL_2 | ROWSEL_3); // Clear row pins
            P3OUT |= (BYTE_CNTR>>1) << 5;           // Select row to turn on

            P2OUT &= ~(OE_MATR | CS_MATR);  // Turn on matrix
        } else {
        }

        BYTE_CNTR = (++BYTE_CNTR) & 0x0F;   //limit to [0 15] (one "frame")
        char *p = (FRAME_OFFS + BYTE_CNTR); // Find index of current byte to send
        UCB0TXBUF = *(p);               // Send the next byte

        IFG2 &= ~UCB0RXIFG;
    }
}

#endif /* USCI_AB_H_ */
