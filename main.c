#include <msp430.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  int end=0;

  int relay=0;
  int nummer=0;
  int puls=0;
  int puls1=0;
  int reset=0;


  int puls1t=0;
  int ppm=0;

  static int pulscount= 0;

  int ppmReady=0;
  //int aantalcounts10;
  short aantalcounts;
  char aantalcountschar[2];
  char AU;



void uart_stuur_string(char a[]){
    unsigned int i; unsigned int j;
    for(j=0;a[j]!='\0';j++);
    for(i=0; i<j;i++){
        while(!(IFG2&UCA0TXIFG));
        UCA0TXBUF = a[i];
    }
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
  {
    while(1);                               // do not load, trap CPU!!
  }

  //GPIO
  P1DIR |= BIT3;                            //P1.3 output
  P1OUT = 0;
  P2DIR |= (BIT0|BIT1|BIT2);                //P2.1 output, P2.2 output, P2.3 output
  P2OUT = 0;                                //P2 Low

  P1DIR |= 0x01;
  P1DIR |= BIT6;

  //Clock
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;                    // Set DCO
  DCOCTL = CALDCO_16MHZ;

  //UART
  P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 130;                            // 1MHz 9600
  UCA0BR1 = 6;                              // 1MHz 9600
  UCA0MCTL = UCBRS2 +UCBRS1;                // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt


  //Timer A0
  P1DIR &= ~BIT0;                           // P1.1/TA0.1 Input Capture
  P1SEL |= BIT0;                            // TA0.1 option select
  TA0CTL = TASSEL_0 + MC_2;                 // TACLK,
  TA0R=0;                                   // zet de counter op 0


  //Timer A1
  P2SEL |= (BIT6|BIT7); // Set P2.6 and P2.6 SEL for XIN, XOUT
  P2SEL2 &= ~(BIT6|BIT7); // Set P2.6 and P2.7 SEL2 for XIN, XOUT
//  TA1CCTL0 = CCIE;
//  TA1CCR0 = 50000;                                  //CLK = 16M/ 8 = 2M
//  TA1CTL = TASSEL_2 + MC_1 + ID_3;                  // SCLK, Up to CCR0, divider 8


  __enable_interrupt();

  while(1){
      if(reset==1){
          TA0CTL =  MC_0;                 // Turn off timer A0
          TA1CTL =  MC_0;                 // Turn off timer A1
          TA0R=0;
          pulscount = 0;
          P2OUT &= ~(BIT0|BIT1|BIT2);
          P1OUT &= ~BIT3;
          reset=0;
      }
      if(ppmReady==1){
          char bufferC[15];
          unsigned long int buffer = TA0R;
          buffer = buffer+(pulscount*55000);
          ltoa(buffer,bufferC,10);
          uart_stuur_string(bufferC);
          uart_stuur_string("%");
          ppmReady=0;
      }
      if(puls==1&&puls1==2){
          TA0R = 0;      //counter regester to 0
          pulscount = 0;
          int buffer = ((int)aantalcountschar[0]-48)*10;//ACII naar int
          int buffer2 =+ (int)aantalcountschar[1]-48;
          aantalcounts = ((buffer+buffer2)*4);
          aantalcounts = buffer+buffer2;

          //start timer A1
          TA1CCR0 = 50000-1;                            //CLK = 16M/ 8 = 2M -1 want start op 0
          TA1CTL = TASSEL_2 + MC_1 + ID_3;              //SCLK, Up to CCR0, divider 8
          TA1CCTL0 = CCIE;

          //start timer A0
          //TA0CTL = TASSEL_0 + MC_2;                   // Op pin 1
          TA0CCR0 = 55000-1;
          TA0CTL = TASSEL_2+MC_1+ID_3;                 // Turn on timer A0
          TA0CCTL0 = CCIE;

          puls1=0;
          puls=0;
      }
      if(relay==1&&nummer==1){
          if(AU=='A'){
              P2OUT |= BIT0;
              relay=0;
              nummer=0;
              AU='.';
          }
          if(AU=='U'){
              P2OUT &= ~BIT0;
              relay=0;
              nummer=0;
              AU='.';
          }
      }
      if(relay==1&&nummer==2){
          if(AU=='A'){
              P2OUT |= BIT1;
              relay=0;
              nummer=0;
              AU='.';
          }
          if(AU=='U'){
              P2OUT &= ~BIT1;
              relay=0;
              nummer=0;
              AU='.';
          }
      }
      if(relay==1&&nummer==3){
          if(AU=='A'){
              P2OUT |= BIT2;
              relay=0;
              nummer=0;
              AU='.';
          }
          if(AU=='U'){
              P2OUT &= ~BIT2;
              relay=0;
              nummer=0;
              AU='.';
          }
      }
      if(relay==1&&nummer==4){
          if(AU=='A'){
              P1OUT |= BIT3;
              relay=0;
              nummer=0;
              AU='.';
          }
          if(AU=='U'){
              P1OUT &= ~BIT3;
              relay=0;
              nummer=0;
              AU='.';
          }
      }
  }
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    pulscount++;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1(void)
{
    static int timerA1P = 0;
    if(timerA1P==3&&aantalcounts!=0){
        ppmReady =1;
        aantalcounts--;
        timerA1P=0;
        if(aantalcounts==0){
            TA0CTL =  MC_0;                 // Turn off timer A0
            TA1CTL =  MC_0;                 // Turn off timer A1
        }
    }
    else{
        timerA1P ++;
    }
}



#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  char buffer;
  buffer = UCA0RXBUF;

      if(buffer=='S'||buffer=='P'||buffer=='R'||buffer=='0'||buffer=='1'||buffer=='2'||buffer=='3'||buffer=='4'||buffer=='5'||buffer=='6'||buffer=='7'||buffer=='8'||buffer=='9'||buffer=='A'||buffer=='U'){
          if(buffer=='S'){
           reset=1;
          }
          if(buffer=='P'){
              puls=1;
          }
          if(puls==1&&(buffer=='0'||buffer=='1'||buffer=='2'||buffer=='3'||buffer=='4'||buffer=='5'||buffer=='6'||buffer=='7'||buffer=='8'||buffer=='9')){
              aantalcountschar[puls1]=buffer;
              puls1++;
          }
          if(buffer=='R'){
              relay=1;
          }
          if(buffer=='1'||buffer=='2'||buffer=='3'||buffer=='4'){
              if(buffer=='1'){
                  nummer=1;
              }
              if(buffer=='2'){
                  nummer=2;
              }
              if(buffer=='3'){
                  nummer=3;
              }
              if(buffer=='4'){
                  nummer=4;
              }
          }
          if(buffer=='A'||buffer=='U'){
                  AU=buffer;
              }
          }
}
