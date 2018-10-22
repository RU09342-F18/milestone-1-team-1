#include <msp430.h>
int count = 0;
unsigned char Length = 0;

extern void LEDSetup();
extern void TimerASetup();
extern void resetColor();
extern void UartSetup();

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                   //Stop WDT
    UartSetup();                                //Uart setup
    LEDSetup();                                 //LED setup
    TimerSetup();                               //Timer setup
    __bis_SR_register(LPM0_bits + GIE); //Set Low Power Mode and Global Interrupt
    __no_operation();
}

void LEDSetup()                       //Output for LED signal and connects timer
{
    P1SEL |= BIT2, BIT3, BIT4;        //Sets port 1.2 1.3 and 1.4 to TimerA CCR1
    P1DIR |= BIT2, BIT3, BIT4;             //Sets port 1.2 1.3 and 1.4 to output
}

void TimerSetup()
{
    TA0CTL = TASSEL_2 + MC_1 + OUTMOD_7; //Uses Small Clock, Puts Timer in Up , Outmode reset/set

    TA0CCTL1 = OUTMOD_2;                        //Sets TACCR1 to toggle
    TA0CCTL2 = OUTMOD_2;                        //Sets TACCR2 to toggle
    TA0CCTL3 = OUTMOD_2;                        //Sets TACCR3 to toggle
    TA0CCR0 = 0xFF;                             //Sets TA0CCR0
}

void UartSetup()
{
    P4SEL |= BIT4 + BIT5;                       //P4.4,5 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;                      //Reset
    UCA1CTL1 |= UCSSEL_2;                     //SMCLK
    UCA1BR0 = 104;                            //1MHz 9600 Bald Rate
    UCA1BR1 = 0;                              //1MHz 9600
    UCA1MCTL |= UCBRS_1 + UCBRF_0;            //Modulation UCBRSx=1, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                    //**Initialize USCI state machine**
    UCA1IE |= UCRXIE;                         //Enable USCI_A1 RX interrupt
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__) //Check for TXBuf and RXBuf readiness
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(UCA1IV, 4))
    {
    case 0:
        break;                                 //No interrupt
    case 2:                                       //RXIFG
        switch (count)
        {
        case 0:
            UCA1TXBUF = UCA1RXBUF - 3; //Detects length bit, subtracts 3 and sends
            Length = UCA1RXBUF;             //Number of expected bytes
            count++;                           //Set to correct byte for machine
            break;
        case 1:                                 //Red
            TA0CCR1 = 255 - UCA1RXBUF;           //Set red PWM
            count++;
            break;
        case 2:                                 //Green
            TA0CCR2 = 255 - UCA1RXBUF;           //Set green PWM
            count++;
            break;
        case 3:                                //Blue
            TA0CCR3 = 255 - UCA1RXBUF;            //Set blue PWM
            count++;
            break;
        default:                       //Case for sending rest of bytes on TXBuf
            if (count <= Length) //Checks to see if remaining bytes are less than length
            {
                UCA1TXBUF = UCA1RXBUF;          //Sends remaining bytes
                count++;
            }
            else                      //If remaining bytes is longer than length
            {
                count = 1;                      //Reset count & restarts
                UCA1TXBUF = UCA1RXBUF - 3;     //Subtract 3 from the length byte
                Length = UCA1RXBUF;             //Store number of expected byte
                TA0CCR1 = 0x00; //Sets original RGB in LED (currently is opposite to expected value and I cannot figure out how to fix it)
                TA0CCR2 = 0x00;
                TA0CCR3 = 0x00;
            }
            break;
        }
        break;
    case 4:
        break;                                 //TXIFG
    default:
        break;
    }
}
