/*
 * USCI_AB.h
 *
 *  USCI A/B header file
 *  Functions:
 *      -- USCI_A0: receive data over UART
 *      -- USCI_B0: Transmit/Receive data over SPI
 *          - Send data to Shift registers
 *          - Send/receive data to/from accelerometer
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef USCI_AB_H_
#define USCI_AB_H_

#define SPIMOSI BIT7
#define SPIMISO BIT6
#define SPICLK  BIT5
#define SPI_PINS SPIMOSI | SPIMISO | SPICLK
#define SPI_RX_NBYTE 6
#define ACCEL_START_ADDR 0x02
#define ACCEL_DATA_LENGTH 6

#define MATRX (1<<0)
#define ACCEL (1<<1)
#define SPI_TX_BUF_SIZE 2
unsigned char TX_ONGOING = MATRX;
unsigned char SPI_BYTE_COUNTER = 0;
unsigned char SPI_TX_BUF[SPI_TX_BUF_SIZE] = { 0 };
unsigned char accel_rxBuf[ACCEL_DATA_LENGTH] = { 0 };

#define UART_RX BIT1
#define UART_TX BIT2
unsigned int UART_BYTE_COUNTER = 0;

void USCI_A0_Init()
{
    P1SEL = UART_RX | UART_TX;                // P1.1 = RXD, P1.2=TXD
    P1SEL2 = UART_RX | UART_TX;               // P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 0x41;                           // 10MHz 1200 Baud
    UCA0BR1 = 0x03;                           // 10MHz 1200 Baud
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt
}

void USCI_B0_Init()
{
    P1SEL |= SPI_PINS;                        // Assign SPI pins to USCI_B0
    P1SEL2 |= SPI_PINS;                       // Assign SPI pins to USCI_B0

    P2DIR |= CS_MATR | CS_ACCL | OE_MATR;     //Set chip selects as outputs
    P2OUT |= OE_MATR;                         // Turn off matrix
    P3DIR |= ROWSEL_1 | ROWSEL_2 | ROWSEL_3;  // Row select pins
    P3OUT &= ~(ROWSEL_1 | ROWSEL_2 | ROWSEL_3);

    UCB0CTL1 |= UCSWRST;                      // Enable SW reset

    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; // Leading edge phase, MSB first, SPI Master, 3-wore SPI, synchronous mode
    UCB0CTL1 = UCSSEL_2 | UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 1;                              // fSCL = SMCLK/1 = ~1MHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                    // Clear SW reset, resume operation
    IE2 |= UCB0TXIE;                          // Enable RX interrupt
}

void handleUART()
{
    int i;
    if (UART_Available == 2 && (UART_Buf[0] == 0xD3)) // Single byte transmission, 'ID' mode
    {
        for (i = 15; i >=0; i--)
        {
            DISP_BUF[i] = ((UART_Buf[1] & (1 << MY_ID)) > 0) ? 0xFF: 0x00;
        }
        CURR_SENSOR = NUM_SENSORS-1; // Static image
    }

    if (UART_Available == 2 && (UART_Buf[0] == 0xA5
                )) // Single byte transmission, 'BTN' mode
        {
            unsigned int index = (UART_Buf[1]>=N_BTN)?N_BTN-1:UART_Buf[1]; // Limit index to range [0 N_BTN]
            CFG_DATA[index+1] = BTN_VAL;
            BTN_VOLTS[index] = BTN_VAL;

            Flash_Write_Start (CFG_MEM_ADDR);       // Start flash write, erase memory
            Flash_Write(CFG_MEM_ADDR, CFG_MEM_SIZE, CFG_DATA); // Write to flash
            Flash_Write_End();                      // End flash write, lock flash
        }

    if (UART_Available == 4 && UART_Buf[0] == 0xAD) // Flash store, key (0xAD) correct
    {
        if((UART_Buf[3] & 0x10) != 0x00){ // If image is Static
            DISP_BUF[16] = UART_Buf[1]; // Glass eye color
        }

        DISP_BUF[128] = UART_Buf[1]; // Glass eye color
        DISP_BUF[129] = UART_Buf[2]; // Dependence
        DISP_BUF[130] = UART_Buf[3]; // Index
        Eyes_Set(DISP_BUF[128]); // Set eyes to display RED
        FlashImages(DISP_BUF);

    }

    if (UART_Available == 4 && UART_Buf[0] == 0xF5) // Configuration ID update mode, key (0xF5) correct
    {
        CFG_DATA[0] = UART_Buf[1];
        Flash_Write_Start (CFG_MEM_ADDR);       // Start flash write, erase memory
        Flash_Write(CFG_MEM_ADDR, CFG_MEM_SIZE, CFG_DATA); // Write to flash
        Flash_Write_End();                      // End flash write, lock flash
    }

    if (UART_Available == 20)
    {
        char index = UART_Buf[1] * 16;

        for (i = 15; i >= 0; i--)
        {
            DISP_BUF[i + index] = UART_Buf[i + 4];
        }

        CURR_SENSOR = (UART_Buf[1] == 0)?NUM_SENSORS-1:0; // Static if first frame, otherwise animated
    }
    STATUS_VEC &= ~STATUS_UART;
}

void clearUARTbuf()
{
    int i;
    for (i = UART_BUFSIZE-1; i >=0 ; i--)
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

        if (UART_BYTE_COUNTER >= NUM_SENSORS)
        {
            IE2 &= ~UCA0TXIE; // Done transmitting, disable TX interrupt
        }
        else
        {
            UCA0TXBUF = sensorData[UART_BYTE_COUNTER]; // Transmit sensor data
        }

        UART_BYTE_COUNTER++;
        IFG2 &= ~UCA0TXIFG;
    }

    if ((IFG2 & UCB0TXIFG) > 0) //SPI Transmit buffer ready
    {
        if (SPI_BYTE_COUNTER < SPI_TX_BUF_SIZE)
        {
//            P2OUT &= ~CS_MATR;
            UCB0TXBUF = SPI_TX_BUF[SPI_BYTE_COUNTER++];
        }
        else
        {
            P2OUT |= OE_MATR; // Turn off matrix

            P2OUT |= CS_MATR; // Load shift register storage registers

            P3OUT &= ~(ROWSEL_1 | ROWSEL_2 | ROWSEL_3); // Clear row pins
            P3OUT |= ROW_CNTR << 5; // Select row to turn on

            P2OUT &= ~CS_MATR;

            P2OUT &= ~(OE_MATR); // Turn on matrix
            IE2 &= ~(UCB0TXIE); // Done transmitting, disable TX interrupt
        }
    }
}

// USCI A/B receive interrupt vector
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{

    if (IFG2 & UCA0RXIFG > 0) //UART triggered interrupt
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

    if (IFG2 & UCB0RXIFG > 0) //SPI triggered interrupt
    {

    }
}

#endif /* USCI_AB_H_ */
