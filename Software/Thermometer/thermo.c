/*
  Thermometer.c

  Created: 01.11.2019 14:00:00
  Author:  Uwe Liedtke und Joerg Wunsch
  Projekt: Temperaturanzeige und Wuerfel
  fuer Atmega 168A
 */


#include "thermo.h"


int main(void)
{
    uint8_t  templ = 0,temph = 0;
    uint16_t temperaturdaten = 0;
    LED_HELLIGKEIT = 0;
	drehcounter = 0;
    PORTs_init();                       // Init der Ein und Ausgabeports
    TIMER_init();                       // Timer Init
    UART_init();                        // INIT 485
    sei();                              // INTERRUPTS GLOBAL AN
	LED_TASK[1][0]=1;					// LED 1 AN nach INIT
    //DEBUG
    //LED_TASK[1][0]=1; // LED 1 AN
    //LED_TASK[3][0]=1; // LED 3 AN
    /*
    for (uint8_t step = 0; step <= LED_ANZAHL ; step++)
    {

            LED_TASK[step][0]=1;

    }
    */
    
	putstring("Tempanzeige und Wuerfel Ver 0.2");         // Ausgabe Versionstext Text
	
    while(1)
    {

        usart_getc_intr();                      // CHECK und Verarbeitung des UART Buffers
        if(qbi(CONTROLLWORD_VOLL,UFLAGS))       // Daten im Puffer > Run
            {
            cbi(CONTROLLWORD_VOLL,UFLAGS);      // Clear FLAG "RUN"
            if(CONTROLLWORD[1] == ADR485)       // Check Richtige Adresse
                {
                switch(CONTROLLWORD[2])         // Abarbeitung switch
                    {
                    case 0:                     // Ausgabe Software Version
                    if(CONTROLLWORD[4] == ADR485) // CRC CHECK 
                        {
                        putstring("Version 0.2");   // Ausgabe Versionstext Text
                        UART_SendByte(10);               // Ausgabe Return
                        }                                // DEBUG
                    break; //END CASE0

                    case 1:
                    if(CONTROLLWORD[4] == ADR485)       // CRC CHECK
                        {
                        putstring("Zeiger:");            // Ausgabe Text
                        errorcodeu(CONTROLLWORD[3]);	// Test 
						UART_SendByte(10);              // Ausgabe Return
                        drehenr(CONTROLLWORD[3]);		// DEBUG
                        }
                    break; //END CASE1

                    case 2:               
                    if(CONTROLLWORD[4] == ADR485)       // CRC CHECK
                        {
                        putstring("TEMPANZEIGE AUS");   // Ausgabe Text
                        UART_SendByte(10);              // Ausgabe Return
                        sbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AUS 
                        }
                    break; //END CASE2

                    case 3: 
                    if(CONTROLLWORD[4] == ADR485)       // CRC CHECK
                        {
                        UART_SendByte(10);              // Ausgabe Return
                        putstring("Wuerfel Spezialmode 7"); // Ausgabe Text
						sbi(WUERFEL_7,FLAGS);			// Wuerfelspezialmode 7
                        UART_SendByte(10);              // Ausgabe Return
                        }
                    break; //END CASE3

                    case 4:                             
                    if(CONTROLLWORD[4] == ADR485)      // CRC CHECK
                    {
                        //ztemp = zufall;                // Zufahlszahl
                        UART_SendByte(10);             // Ausgabe Return
                        putstring("drehen an: ");      // Ausgabe Versionstext Text
                        //drehaktiv = CONTROLLWORD[3];
						drehaktiv = zufall;
                    }
                    break; //END CASE3
                    }

                }

            }

/*****************************************************************************
 startet den Analogdigitalwandler
 zum Auslesen des int Temp Sensor                                                                    
******************************************************************************/
         if (counter == 20)
              {
              sbi(6,ADCSRA);
              counter++;
              }

/******************************************************************************
Auslesen der Temperaturdaten aus den AD Wandler
******************************************************************************/
         if (counter == 70 )
              {
	          templ = ADCL;
	          temph = ADCH;
	          temperaturdaten = HILO(temph,templ);
              counter++;
              }

/******************************************************************************
Ausgabe der Temperaturdaten auf das LED Band
Ist Temp OFF gesetzt werden die TEMP LEDS geloescht 
FLAG TEMPISOFF wird gesetzt damit das LED Temperaturband nur einmal ruckgesetzt wird                                                                 
******************************************************************************/
         if ( qbi(TEMP_OFF,FLAGS) )
			{
			if (!qbi(TEMPISOFF,FLAGS))
			   {
				sbi(TEMPISOFF,FLAGS);   
				for (uint8_t step = 0; step <= 10 ; step++)
                    {
					LED_TASK[step][0]=0;
                    }   
			   }
			}
		 else
			{
		 if (counter == 200)
              {
	          ledband(temperaturdaten,330);
	          counter++;
              }
			}
			  
/******************************************************************************
Wuerfeln
Uebergabe der Geschwindigkeit beim Wuerfeln
Uebergabe der Zufahlszahl fuer den Wuerfel
Beim Start Taster muss noch die Anzahl der Umlaeufe drehaktiv festgelegt werden
drehaktiv = zufall oder man zaehlt die Taster Prellungen
******************************************************************************/	
         if (drehaktiv)
              {
              wuerfellos(3,zufall);                  
              }

            
	
    }
}



// Hier beginnen die Funktionen

/***************************************************************************************
Steuerung des LED Multiplexers
ueber den Uebergabeparameter wird festgelegt welche LED eingeschaltet wird
Die Globale Variable LED Helligkeit legt fest mit welcher Helligkeit (Pulsweite) die LED´s angesteuert werden
***************************************************************************************/

void led_multiplex(uint8_t ledcount)
{
    if(MAX_HELL < LED_HELLIGKEIT )
        {
            LED_HELLIGKEIT = MAX_HELL;  // hier wird die vorgegebene Helligkeit auf den maximal zulaessigen Wert begrenzt
        }

    if (LED_Timer)
        {                   // wenn LED Timer fuer Multiplexer 0, dann kann die naechste LED geschaltet werden
                            // Diese Funktion steuert das Multiplexing und das Puls-/Pausenverhaeltnis der LED`s
        LED_Timer--;        // besser ueber Interrupt
        if (LED_Timer <= (MAX_HELL - LED_HELLIGKEIT) )
            {
                zeilenwahl(0);      // Alle LEDs AUS
            }
        }
    else
        {                   // wenn LED Timer 0 wird die auf die naechste LED geschaltet
        if (LED_TASK[ledcount][0])
            {
            zeilenwahl(ledcount);   // LED AN wenn Array 1
            }
        LED_Timer = MAX_HELL;       // bestimmt die Durchlaufzeit (Puls + Pause)

        }

}

/***************************************************************************************
Ansteuerung Wuerfel (7 LED`s) Zahlen von 1 bis 7 sind moeglich
***************************************************************************************/
void wuerfel(uint8_t wzahl)
{
for (uint8_t step = 11; step <= 17 ; step++)    // Abschalten aller 7 LED`s
    {
    LED_TASK[step][0]=0;                        // Abschalten aller LED`s auf dem Wuerfelfeld
    }
switch (wzahl)
    {
    case 1:                 // Wuerfel Zahl 1
    LED_TASK[11][0]=1;      // Wuerfel Zahl 1 >  LED 11
    break;                  // Ende case

    case 2:                 // Wuerfel Zahl 2
    LED_TASK[14][0]=1;      // Wuerfel Zahl 2 >  LED 14
    LED_TASK[16][0]=1;      // Wuerfel Zahl 2 >  LED 16
    break;                  // Ende case

    case 3:                 // Wuerfel Zahl 3
    LED_TASK[11][0]=1;      // Wuerfel Zahl 3 >  LED 11
    LED_TASK[14][0]=1;      // Wuerfel Zahl 3 >  LED 14
    LED_TASK[16][0]=1;      // Wuerfel Zahl 3 >  LED 16
    break;                  // Ende case

    case 4:                 // Wuerfel Zahl 4
    LED_TASK[14][0]=1;      // Wuerfel Zahl 4 >  LED 14
    LED_TASK[15][0]=1;      // Wuerfel Zahl 4 >  LED 15
    LED_TASK[16][0]=1;      // Wuerfel Zahl 4 >  LED 16
    LED_TASK[17][0]=1;      // Wuerfel Zahl 4 >  LED 17
    break;                  // Ende case

    case 5:                 // Wuerfel Zahl 5
    LED_TASK[11][0]=1;      // Wuerfel Zahl 5 >  LED 11
    LED_TASK[14][0]=1;      // Wuerfel Zahl 5 >  LED 14
    LED_TASK[15][0]=1;      // Wuerfel Zahl 5 >  LED 15
    LED_TASK[16][0]=1;      // Wuerfel Zahl 5 >  LED 16
    LED_TASK[17][0]=1;      // Wuerfel Zahl 5 >  LED 17
    break;                  // Ende case

    case 6:                 // Wuerfel Zahl 6
    LED_TASK[12][0]=1;      // Wuerfel Zahl 6 >  LED 12
    LED_TASK[13][0]=1;      // Wuerfel Zahl 6 >  LED 13
    LED_TASK[14][0]=1;      // Wuerfel Zahl 6 >  LED 14
    LED_TASK[15][0]=1;      // Wuerfel Zahl 6 >  LED 15
    LED_TASK[16][0]=1;      // Wuerfel Zahl 6 >  LED 16
    LED_TASK[17][0]=1;      // Wuerfel Zahl 6 >  LED 17
    break;                  // Ende case

    case 7:                 // Wuerfel Zahl 7
    LED_TASK[11][0]=1;      // Wuerfel Zahl 7 >  LED 11
    LED_TASK[12][0]=1;      // Wuerfel Zahl 7 >  LED 12
    LED_TASK[13][0]=1;      // Wuerfel Zahl 7 >  LED 13
    LED_TASK[14][0]=1;      // Wuerfel Zahl 7 >  LED 14
    LED_TASK[15][0]=1;      // Wuerfel Zahl 7 >  LED 15
    LED_TASK[16][0]=1;      // Wuerfel Zahl 7 >  LED 16
    LED_TASK[17][0]=1;      // Wuerfel Zahl 7 >  LED 17
    break;                  // Ende case

    }

}


/******************************************************************************
 Ansteuerung des Wuerfels 
 Beginnt mit einen drehenden Feld im Wuerfel Segment
 Parameteruebergabe:
 drehg (Geschwindigkeit des Zeigers)
 ztemp (Uebergabe der gewuerfelten Zahl
 

Die Variable wzeiger bestimmt die Geschwindigkeit und wird im Timer2 hochgezaehlt
******************************************************************************/
void wuerfellos(uint8_t drehg,uint8_t ztemp)
{
	if (qbi (drehg,wzeiger))
	{
		wzeiger = 0;
		drehenr(drehcounter);
		drehcounter++;
		if (drehcounter >= 7)
		{
			drehcounter = 1;
			drehaktiv--;
		}
	}
	if (!drehaktiv)
	{
		wuerfel(ztemp);
		errorcodeu(ztemp);              // Ausgabe Zufallszahl
	}
}

/***************************************************************************************
Ansteuerung Wuerfelfeld (7 LED`s) um einen drehenden Zeiger darzustellen
Wird genutzt um eine Wuerfel Aktivitaet darzustellen
Uebergabe der Zeigerstellung 
***************************************************************************************/
void drehenr(uint8_t wstep)
{
for (uint8_t step = 11; step <= 17 ; step++)    // Abschalten aller 7 LED`s
    {
    LED_TASK[step][0]=0;                        // Abschalten aller LED`s auf dem Wuerfelfeld
    }
switch (wstep)
    {
    case 1:                 // Step 1 -
    LED_TASK[11][0]=1;      // LED 11
    LED_TASK[12][0]=1;      // LED 12
    LED_TASK[13][0]=1;      // LED 13
    break;                  // Ende case

    case 2:                 // Step 2 nr
    LED_TASK[11][0]=1;      // LED 11
    LED_TASK[15][0]=1;      // LED 15
    LED_TASK[17][0]=1;      // LED 17
    break;                  // Ende case

    case 3:                 // Step 3 /
    LED_TASK[11][0]=1;      // LED 11
    LED_TASK[14][0]=1;      // LED 14
    LED_TASK[16][0]=1;      // LED 16
    break;                  // Ende case

    case 4:                 // Step 4 -
    LED_TASK[11][0]=1;      // LED 11
    LED_TASK[12][0]=1;      // LED 12
    LED_TASK[13][0]=1;      // LED 13
    break;                  // Ende case

    case 5:                 // Step 5 nr
    LED_TASK[11][0]=1;      // LED 11
    LED_TASK[15][0]=1;      // LED 15
    LED_TASK[17][0]=1;      // LED 17
    break;                  // Ende case

    case 6:                 // Step 6 /
    LED_TASK[11][0]=1;      // LED 11
    LED_TASK[14][0]=1;      // LED 14
    LED_TASK[16][0]=1;      // LED 16
    break;                  // Ende case

    }
}

/******************************************************************************
Ansteuerung LED Band auf Basis AD Wandler
******************************************************************************/

void ledband(uint16_t tempwert, uint16_t tempset)
{
if ( tempwert < tempset)
    {
    for (uint8_t step = 0; step <= 9 ; step++)  // Fuer Testzwecke werden die Daten als Binaere Zahl auf die LED`s 1 bis 10 geschrieben.
        {
        LED_TASK[step+1][0]=0;                  // Abschalten aller LED`s der Temperaturanzeige
        }
    }



for (uint8_t step = 0; step <= 9 ; step++)      // Zur Ansteuerung des LED Temperaturbandes
    {
    tempset++; //
    if ( (tempwert >= tempset) && (tempwert <= (tempset + 2)) ) // Hier wird festgelegt in welchen Bereich die LED's zugeschaltet werden.
        {
        LED_TASK[step+1][0]=1;                  // Zuschalten LED auf den Band
        }
    else
        {
        LED_TASK[step+1][0]=0;                  // Abschalten der nichtaktiven LED auf den Band
        }
    tempset++;
    }



}


/******************************************************************************
Ansteuerung der LED im Multiplex Verfahren
Das Timing wird per Timer Interrupt gesteuert
******************************************************************************/
void zeilenwahl(uint8_t zeile)
{

 switch (zeile)
    {
     case 1:
     ZEILE1_ON;     // LED1 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_ON;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 2:
     ZEILE1_ON;     // LED2 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_ON;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 3:
      ZEILE1_ON;        // LED3 AN
      ZEILE2_OFF;
      ZEILE3_OFF;
      ZEILE4_OFF;
      SPALTE1_OFF;
      SPALTE2_OFF;
      SPALTE3_ON;
      SPALTE4_OFF;
      SPALTE5_OFF;
     break;
     case 4:
      ZEILE1_ON;        // LED4 AN
      ZEILE2_OFF;
      ZEILE3_OFF;
      ZEILE4_OFF;
      SPALTE1_OFF;
      SPALTE2_OFF;
      SPALTE3_OFF;
      SPALTE4_ON;
      SPALTE5_OFF;
     break;
     case 5:
     ZEILE1_ON;     // LED5 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_ON;
     break;
     case 6:
     ZEILE1_OFF;        // LED6 AN
     ZEILE2_ON;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_ON;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 7:
     ZEILE1_OFF;        // LED7 AN
     ZEILE2_ON;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_ON;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 8:
     ZEILE1_OFF;        // LED8 AN
     ZEILE2_ON;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_ON;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 9:
     ZEILE1_OFF;        // LED9 AN
     ZEILE2_ON;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_ON;
     SPALTE5_OFF;
     break;
     case 10:
     ZEILE1_OFF;        // LED10 AN
     ZEILE2_ON;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_ON;
     break;
     case 11:
     ZEILE1_OFF;        // LED11 AN
     ZEILE2_OFF;
     ZEILE3_ON;
     ZEILE4_OFF;
     SPALTE1_ON;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 12:
     ZEILE1_OFF;        // LED12 AN
     ZEILE2_OFF;
     ZEILE3_ON;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_ON;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 13:
     ZEILE1_OFF;        // LED13 AN
     ZEILE2_OFF;
     ZEILE3_ON;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_ON;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 14:
     ZEILE1_OFF;        // LED14 AN
     ZEILE2_OFF;
     ZEILE3_ON;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_ON;
     SPALTE5_OFF;
     break;
     case 15:
     ZEILE1_OFF;        // LED15 AN
     ZEILE2_OFF;
     ZEILE3_ON;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_ON;
     break;
     case 16:
     ZEILE1_OFF;        // LED16 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_ON;
     SPALTE1_ON;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 17:
     ZEILE1_OFF;        // LED17 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_ON;
     SPALTE1_OFF;
     SPALTE2_ON;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 18:
     ZEILE1_OFF;        // LED18 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_ON;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_ON;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
     case 19:
     ZEILE1_OFF;        // LED19 AN
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_ON;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_ON;
     SPALTE5_OFF;
     break;
     default:
     ZEILE1_OFF;        // LEDs AUS
     ZEILE2_OFF;
     ZEILE3_OFF;
     ZEILE4_OFF;
     SPALTE1_OFF;
     SPALTE2_OFF;
     SPALTE3_OFF;
     SPALTE4_OFF;
     SPALTE5_OFF;
     break;
    }

}

/***************************************************************************************
INIT der Ein und Ausgabeports
***************************************************************************************/

void PORTs_init(void)
{
/* Belegung PORTB
	
PB0 = Kathode LED 11,12,13,14,15 
PB1 = Kathode LED  6, 7, 8, 9,10
PB2 = Kathode LED  1, 2, 3, 4, 5
PB3 = MOSI / Jumper Konfig
PB4 = MISO / Jumper Konfig 
PB5 = SCK  / Jumper Konfig
PB6 = Quarz
PB7 = Quarz 
Bei Init werden die PORTS PB0 bis PB2 als Ausgang gesetzt
und die PORTS PB3 bis PB7 als Ausgang
PULLUP Widerstaende zur Abfrage der Jumper Stellung werden im Programm gesetzt.

*/
    DDRB = 0b00000111;
    PORTD =0b00000000;

/* Belegung PORTC
PC0 = USB D-
PC1 = USB D+
PC2 = 1 Wirer DATA 4,7K Widerstand geht auf Masse, in der Rev. 3.3 falsch?
PC3 = 1 Wirer Power 
PC4 = Taster Start Wuerfel, Eingang mit internen Pull_UP
PC5 = RX/TX Umschaltung fuer RS485 HI=TX LOW=RX
PC6 = RESET
PC7 = NC
Alle Ports auf Eingang, bis auf PORT PC5 AUSGANG fuer DEBUG Ausgabe 
PULLUP Widerstand vom PORT PC2,PC4 aktive
*/
    DDRC =  0b00100000;
    PORTC = 0b00010100;


/* Belegung PORTD
PD0 = RXD (485)
PD1 = TXD (485)
PD2 = Anode LED  5,10,15    > Spalte
PD3 = Anode LED  4,9,14,19  > Spalte 
PD4 = Anode LED  3,8,13,18  > Spalte
PD5 = Anode LED  2,7,12,17  > Spalte
PD6 = Anode LED  1,6,11,16  > Spalte
PD7 = Kathode LED 16,17,18,19 > Zeile
Port PD2 bis PD7 auf Ausgang



*/
    DDRD =  0b01111100;
    PORTD = 0b00000000;
}

/******************************************************************************
TIMER INIT
TIMER0 > Multiplexer
TIMER1 > nicht genutzt
TIMER2 > Zufallsgenerator und Rucksetzen des TX Betriebes UART
AD-Wandler > Auslesen des internen Temperatursensor 
im Singel Conversations Modus 
******************************************************************************/
void TIMER_init(void)
{

/************************ TIMER 0 8 BIT Zaehler  *****************************/
    TCCR0B = 0b00000001; // Teiler 1024 3,68MHZ = 259ns*8*256(8BIT) = 0,5ms
    TCCR0A = 0x00;
    OCR0A = 0;          // Output Compare Register
    OCR0B = 0;          // Output Compare Register
    TCNT0 = 0;          // counter Timer 0
    sbi(0,TIMSK0);      // Timer0 Overflow Interrupt Enable


/************************ Timer 1 16 BIT Zaehler *****************************/
    //TCCR1B = 0x05;    // Teiler 1024 3,68MHZ = 259ns
    //TCNT1H Zaehler HIGH Byte (8bit)
    //TCNT1L Zaehler  LOW Byte (8bit)


/************************ TIMER 2 8 BIT Zaehler ******************************/

    //TCCR2B = 0x07;    // Teiler 1024 3,68MHZ = 259ns*1024*256(8BIT) = 67,91ms
    TCCR2B = 0x06;      // Teiler  256 3,68MHZ = 259ns* 256*256(8BIT) = 16,97ms
    //TCCR2B = 0x05;    // Teiler  128 3,68MHZ = 259ns* 128*256(8BIT) =  8,49ms
    //TCCR2B = 0x04;    // Teiler   64 3,68MHZ =  83ns*  64*256(8BIT) =  4,248ms
    //TIMSK2 = 0x01;
    sbi(0,TIMSK2);      // Timer0 Overflow Interrupt Enable

/************************ Analog Digital Wandler Singel **********************/
    ADMUX = 0b11001000;  //int.	T-Sensor BIT4, Internal Reference 1,1V BIT 6&7
    // ADC Enable BIT7, ADC Start Conversion BIT 6,ADC Auto Trigger Enable BIT 5, Vorteiler auf 32 fuer den ADC BIT 1-3
	ADCSRA = 0b10000101; 
    ADCSRB = 0b00000000; // Timer/Counter1 Overflow startet Wandlung, BIT 1&2
    // ADCL zuerst lesen, dann ADHL
}


/*****************************************************************************************
initialisieren des UART
Bautrate einstellen 
Aktivieren der Betriebsart Senden und Empfangen
Aktivieren des Empfangs Interrupts 
*****************************************************************************************/
void UART_init(void)
{
    UBRR0H = HIGH(USARTSPEED);
    UBRR0L = LOW(USARTSPEED);
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);
    UCSR0C = (0<<USBS0)|(3<<UCSZ00);
}



/******************************************************************************
Sendet ein Byte ueber die UART
******************************************************************************/
void UART_SendByte(uint8_t data)
{
    SENDEN_AKTIV;                         // RX > TX Umschaltung 
    rucksetzcount = 3;                    // Delay zur Rueckschaltung RX
    _delay_ms(10);
    while(bit_is_clear(UCSR0A, UDRE0));   // wartet auf UART Ready 
    UDR0 = data;                            
}


/******************************************************************************
sendet einen String ueber UART
******************************************************************************/
void putstring(char *s)                     
{
    while (*s != 0)                        
    {
        UART_SendByte(*s);
        *s++;
    }
}

/******************************************************************************
sendet den Fehlercode (Wandlung Hexzahl >> ASCII)
******************************************************************************/
void errorcodeu(uint8_t zahl)
{
    uint8_t temp = 0;

    temp = zahl % 10;                       // niederwertigste Ziffer
    zahl = (zahl - temp) / 10;              // Rest von 10 ergibt die Einer
    temp = temp | 0x30;                     // Erzeuge ASCII Zeichen
    if(zahl != 0){  errorcodeu(zahl);   }   

    UART_SendByte(temp);                    // Sendet Ziffer
}


/******************************************************************************
sendet den Fehlercode (Wandlung Hexzahl >> ASCII) (16BIT)
******************************************************************************/
void errorcodeu16(int16_t zahl)
{
    uint8_t temp = 0;

    temp = zahl % 10;                       // niederwertigste Ziffer
    zahl = (zahl - temp) / 10;              // Rest von 10 ergibt die Einer
    temp = temp | 0x30;                     // Erzeuge ASCII Zeichen
    if(zahl != 0){  errorcodeu16(zahl); }   

    UART_SendByte(temp);                    // Sendet Ziffer
}


void warte_sekunde(void)
{
    _delay_ms(250);
    _delay_ms(250);
    _delay_ms(250);
    _delay_ms(250);
}

/******************************************************************************
Diese Funktion Liest ein Byte aus dem UART Puffer
Berechnet wieviel Zeichen sich noch im Buffer befinden
Wenn Controllword freigegeben, werden die Zeichen dort eingeschrieben.
Ruecksetzbedingung in Bearbeitungsroutine:
Controllword[0]         > loeschen
UART_PUFFER_VOLL,FLAGS  > loeschen


Das Steuerwort muss  CW_SIZE Stellen haben !!

             0 1 2 3 4 5
             > Steuerzeichen wird nicht verarbeitet !
               > Adresse
                 > Kommando
                   > WERT
                     > CRC


Belegung CONTROLLWORD:
                        0 - Steuerzeichen (>)
                        1 - Display Adresse
                        2 - Segmentadresse
                        3 - WERT
                        4 - CRC


******************************************************************************/
void usart_getc_intr(void)
{

uint8_t schreibzeigernew;

// Berechnung: Anzahl der Zeichen, wieviel sich im UART Buffer befinden
// Kontrolle ob Uebertrag ausgeloesst wurde

    if(lesezeiger > schreibzeiger)                                          // Pruefung Uebertrag
        {
        schreibzeigernew = PUFFER_GROESSE + schreibzeiger;                  // Uebertrag wird dazugerechnet
        }
    else
        {
        schreibzeigernew = schreibzeiger;                                   // Keine Korrektur notwendig
        }


    zeicheninbuffer = schreibzeigernew - lesezeiger;                        // Berechnung Anzahl der Zeichen(im UART Buffer)
if ( zeicheninbuffer > ( PUFFER_GROESSE - 20) )
    {
    //putstring("[ERR_BUF_OVERFLOW]");                                        // Error fuer Buffer Overflow !!!
    }

if(zeicheninbuffer && !qbi(CONTROLLWORD_VOLL,UFLAGS))                       // Wenn Zeichen im Buffer und Controllbuffer nicht gesperrt ist erfolgt Abarbeitung
    {
    do
        {

        if(puffer[lesezeiger] == STEUERZEICHEN)                             // hier befindet sich das Steuerzeichen zum ruecksetzen des Zeigers
            {
            UARTINDEX = CLEAR;                                              // das Zeichen > setzt den INDEX vom Controllword auf 0 zurueck
            }

        if( (UARTINDEX >= 0) && (UARTINDEX < CW_SIZE) )                     // nur wenn sich der INDEX zwischen 0 und Buffergroeße befindet
            {
            CONTROLLWORD[UARTINDEX] = puffer[lesezeiger];                   // wird das aktuelle Zeichen im Buffer ins Controllword geschrieben
            UARTINDEX++;                                                    // INDEX wird um eins erhoeht
            }

        if( (UARTINDEX == CW_SIZE) && (CONTROLLWORD[0] == STEUERZEICHEN) )  // Wenn Controllword gefuellt, wird geprueft ob es sich beim ersten Zeichen um das Steuerzeichen handelt
            {
            sbi(CONTROLLWORD_VOLL,UFLAGS);                                  // wenn beide Bedingung erfuellt, wird Controllword fuer Weiterbearbeitung freigegeben und gleichzeitig fuers beschreiben gesperrt
            CONTROLLWORD[0] = 0;                                            // Controllword "0" loeschen
            }
        lesezeiger++;                                                       // Lesezeiger um eins erhoehen
        zeicheninbuffer--;                                                  // Da keine Neuberechnung in der Schleife fuer die Anzahl der Zeichen im UART Buffer erfolgt wird manuell eins runtergezaehlt
        if(lesezeiger==PUFFER_GROESSE) lesezeiger=0;                        // wenn Lesepuffer am Ende ruecksetzen
        }while(zeicheninbuffer && !qbi(CONTROLLWORD_VOLL,UFLAGS));          // wenn keine Zeichen mehr im UART Buffer sind oder das Controllword gesperrt wurde wird Schleife verlassen

    }

}
/******************************************************************************
INTERRUPT Timer 0 Timerueberlauf
Einsprung alle XX ms
Dieser Timer wird fuer den Multiplexer genutzt
*******************************************************************************/
ISR(TIMER0_OVF_vect)
{


if ( (LED_TASK[LED_Timer][0]) && ( taskcount < 1 ) )
    {
    zeilenwahl(LED_Timer);  // LED Ein
    }
else
    {
    zeilenwahl(0);          // LED Aus 
    }
taskcount++;
if (taskcount > (MAX_HELL -LED_HELLIGKEIT))
    {
    taskcount = 0;
    }

LED_Timer++;
if (LED_Timer > LED_ANZAHL)
    {
    LED_Timer = 1;
    }


}


/******************************************************************************
INTERRUPT Timer 0 Vergleich A 
Wird nicht genutzt
******************************************************************************/
ISR (TIMER0_COMPA_vect)
{

}


/******************************************************************************
INTERRUPT Timer 2 Timerueberlauf
Einsprung alle XX ms
Setzt UART nach Zeit X wieder in den Empfangsmodus 
Zufallsgenerator fuer den Wuerfel
******************************************************************************/

ISR(TIMER2_OVF_vect)
{
    
// Funktion zum Ruecksetzen des aktiven Sendekanals der Schnittstelle RS485
if(rucksetzcount)
    {
    if(rucksetzcount == 1)
        {
        SENDEN_INAKTIV;
        }
    rucksetzcount--;
    }
	
// Test Zufallsgenerator fuer den Wuerfel	

if ( qbi(WUERFEL_7,FLAGS) )
     {
     if (zufall >= 7)
         {
	     zufall = 0;
         }
     }
else
	{
    if (zufall >= 6)
          {
          zufall = 0;
          }	   
    }
	
	
	
zufall++;
counter++;
wzeiger++;
}



/******************************************************************************
INTERRUPT ROUTINE fuer UART, schreibt empfangenes Zeichen in den BUFFER
Empfangene Zeichen werden in den Puffer geschrieben 
Ist der Zeiger am Ende des Puffesr wird dieser an den Anfang gesetzt
******************************************************************************/
ISR(USART_RX_vect)
{

puffer[schreibzeiger]=UDR0;
schreibzeiger++;
if(schreibzeiger==PUFFER_GROESSE) schreibzeiger=0;
}
