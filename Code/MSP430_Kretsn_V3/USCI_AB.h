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

#define UART_RX BIT1
#define UART_TX BIT2
unsigned int UART_BYTE_CNTR = 0;

char* UART_TXBUF;
unsigned char UART_BYTES_TX = 0;
char UART_TXDONE = 1;

#define SPIMOSI BIT7
#define SPIMISO BIT6
#define SPICLK  BIT5
#define SPI_PINS SPIMOSI | SPIMISO | SPICLK

unsigned char BYTE_CNTR   = 0;     // Byte counter for matrix updating

// UART Initialization
void USCI_A0_Init()
{
    P1SEL = UART_RX | UART_TX;                // P1.1 = RXD, P1.2=TXD
    P1SEL2 = UART_RX | UART_TX;               // P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600 Baud
    UCA0BR1 = 0x00;                           // 1MHz 9600 Baud
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE | UCA0TXIE;               // Enable USCI_A0 TX/RX interrupt
}

// SPI Initialization
void USCI_B0_Init()
{
    P1SEL |= SPI_PINS;                          // Assign SPI pins to USCI_B0
    P1SEL2 |= SPI_PINS;                         // Assign SPI pins to USCI_B0

    P2DIR |= CS_MATR | OE_MATR;                 //Set chip select and output enable as outputs
    P3DIR |= ROWSEL_1 | ROWSEL_2 | ROWSEL_3;    // Row select pins as outputs
    P3OUT &= ~(ROWSEL_1 | ROWSEL_2 | ROWSEL_3); // Pull all row select pins LOW

    UCB0CTL1 |= UCSWRST;                        // Enable SW reset

    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; // Leading edge phase, MSB first, SPI Master, 3-wire SPI, synchronous mode
    UCB0CTL1 = UCSSEL_2 | UCSWRST;              // Use SMCLK, keep SW reset

    UCB0BR0 = 100;                              // fSCL = SMCLK/255 = ~4kHz <- SPI clock speed
                                                // 128 bits/frame, 4000/128 ~30fps
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset, resume operation
    IE2 |= UCB0RXIE;                            // Enable RX interrupt

    P2OUT |= OE_MATR;                           // Turn off matrix
    UCB0TXBUF = 0xFF;                           // Kick off SPI infinity madness

}

void uartTransmit(char *data, unsigned char length) {
    UART_TXBUF = data;
    UART_BYTES_TX = length;
    UART_TXDONE = 0;
    UCA0TXBUF = *(UART_TXBUF++);
    UART_BYTES_TX--;
}

void handleUART()
{
    unsigned int UART_CMD = UART_Buf[0] << 8 | UART_Buf[1];
    unsigned int base = (UART_Buf[2] << 8 | UART_Buf[3]);

    switch (UART_CMD)
    {
    case UART_CMD_WRT_FLASH:
    {
        unsigned char length = UART_Buf[4];
        Flash_Write(base, &(UART_Buf[6]), length);
        if(base == DESC_MEM_BASE){
            n_descriptors = Flash_Count_Descriptors();
        }
        break;
    }
    case UART_CMD_CLR_FLASH:
    {
        Flash_Erase(base);
        if(base == DESC_MEM_BASE){
            n_descriptors = 0;
        }
        break;
    }
    case UART_CMD_READ_FLASH:
    {

        if (!inAllowedMem(base, INFO_SIZE))
        {
            uartTransmit(flash_regions, sizeof(flash_regions)); // Transmit flash information over UART
        }
        else
        {
            uartTransmit(base, (base == IMG_MEM_BASE)? IMG_MEM_SIZE : INFO_SIZE); // Transmit memory region
        }
        break;
    }
    case UART_CMD_IDENTITY:
    {
        uartTransmit(version_data, sizeof(version_data)); // Transmit version information over UART
        break;
    }
    }
}

void clearUARTbuf()
{
    memset(UART_Buf, 0, UART_BUFSIZE);
    UART_Available = 0;
}

// USCI A/B Transmit done interrupt vector
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    if ((IFG2 & UCA0TXIFG) > 0) //UART Transmit buffer ready
    {
        if (UART_BYTES_TX > 0)
        {
            UCA0TXBUF = *(UART_TXBUF++);
            UART_BYTES_TX--;
        } else {
            UART_TXDONE = 1;
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

            P2OUT &= ~(OE_MATR | CS_MATR);  // Turn on matrix, CS HIGH
        } else {
        }

        BYTE_CNTR = (++BYTE_CNTR) & 0x0F;   //limit to [0 15] (one "frame")
        char *p = (FRAME_OFFS + BYTE_CNTR); // Find index of current byte to send
        UCB0TXBUF = *(p);               // Send the next byte

        IFG2 &= ~UCB0RXIFG;
    }
}

#endif /* USCI_AB_H_ */
