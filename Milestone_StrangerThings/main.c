/*
References
1. https://www.embeddedrelated.com/showarticle/420.php
2. TI REsource explorer msp430g2xx3_uscia0_uart_01_9600.c
*/

#include <msp430.h>

/**
 * main.c
 */

volatile int byte = 0, size = 0;
// byte variable declaration for each receiving byte in the packet
// size variable declaration for the size of the packet


void initLED ()
{
    /* Port Layout
        P1.6 = Red LED
        P2.1 = Green LED
        P2.4 = Blue LED */

    P1SEL |= BIT6; //Primary peripheral module function is selected
    P1SEL2 &= ~BIT6; //Primary peripheral module function is selected
    P1DIR |= BIT6; //Sets direction of P1.6 to an output

    P2SEL |= BIT1; //Primary peripheral module function is selected
    P2SEL2 &= ~BIT1; //Primary peripheral module function is selected
    P2DIR |= BIT1; //Sets direction of P2.1 to an output

    P2SEL |= BIT4; //Primary peripheral module function is selected
    P2SEL2 &= ~BIT4; //Primary peripheral module function is selected
    P2DIR |= BIT4; //Sets direction of P2.4 to an output

    P2DIR |= BIT5;
    P2OUT &= ~BIT5;
}

void initTimer ()
{
    TA0CTL = TASSEL_2 + MC_1 + ID_2 + TACLR; //SMLCK, Up-Mode, Input Divider on TimerA0 Control Register
    TA1CTL = TASSEL_2 + MC_1 + ID_2 + TACLR; //SMLCK, Up-Mode, Input Divider on TimerA1 Control Register

    TA0CCR0 = 255; //Max value of a 1 byte integer value, set equal to TimerA0 capture/compare register 0
    TA1CCR0 = 255; //Max value of a 1 byte integer value, set equal to TimerA1 capture/compare register 0
    TA0CCR1 = 0; //Starting value of Red PWM
    TA1CCR1 = 0; //Starting value of Green PWM
    TA1CCR2 = 0; //Starting value of Blue PWM


    TA0CCTL1 = OUTMOD_3; //Output set/reset mode for Capture/Compare Control Register
    TA1CCTL1 = OUTMOD_3; //Output set/reset mode for Capture/Compare Control Register
    TA1CCTL2 = OUTMOD_3; //Output set/reset mode for Capture/Compare Control Register

}

void initUART ()
{
    P1SEL |= BIT1+BIT2;
    P1SEL2 |= BIT1+BIT2;
    UCA0CTL1 |= UCSSEL_2; //Selects SMCLK for USCI_A0 control register

    // Baud Rate calculation
    // 1000000/9600 = 104.1667
    // Fractional portion = 0.1667
    // Use Table 24-5 in Family User Guide
    UCA0BR0 = 104; //sets to specified baud rate of 9600
    UCA0BR1 = 0; //Sets to specified baud rate of 9600
    //Prescalar value equals UCA0BR0 + UCA0BR1*256

    UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST; //Initialize USCI state machine
    UC0IE |= UCA0RXIE; //Enable USCI_A0 RX interrupt
}
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   //Stop watchdog timer
    initLED(); //Initialize LED function setup
    initTimer(); //Initialize Timer function setup
    initUART(); //Initialize UART function setup
    __bis_SR_register(LPM0_bits + GIE); //Low-power mode and Global interrupts enabled

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

    switch (byte) //switch statement for the packet of byte values
    {
    case 0:
        size = UCA0RXBUF; //Variable, size, equals the first byte in the receiving buffer register
        UCA0TXBUF = UCA0RXBUF - 3; //Transmit buffer equals receiving buffer minus the 3 bytes used
        break;
    case 1:
        TA0CCR1 = UCA0RXBUF; //Second byte equals TimerA0 CCR1
        break;
    case 2:
        TA1CCR1 = UCA0RXBUF; //Third byte equals TimerA1 CCR1
        break;
    case 3:
        TA1CCR2 = UCA0RXBUF; //Fourth byte equals TimerA1 CCR2
        break;
    default:
        UCA0TXBUF = UCA0RXBUF; //Default to transmit buffer equals receiving buffer
        break;
    }
    if (byte != size)
    {
        byte++;
    }
    else
    {
        byte = 0;
    }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
     P1OUT |= BIT0;
     UC0IE &= ~UCA0TXIE;
     P1OUT &= ~BIT0;
}
