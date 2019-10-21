/*
  Thermometer.c

  Created: 12.11.2018 19:36:08
  Author: uwe
  fuer Atmega 168A
 */


#include "thermo.h"


int main(void)
{
    // uint8_t temp = mysensors_command_t.C_INTERNAL;
    uint8_t  ztemp = 0, templ = 0,temph = 0, drehcounter = 0, drehaktiv = 0;
    uint16_t temperaturdaten = 0;
    LED_HELLIGKEIT = 0;
    PORTs_init();                       // Init der Ein und Ausgabeports
    TIMER_init();                       // Timer Init
    UART_init();                        // INIT 485
    sei();                              // INTERRUPTS GLOBAL AN

    //LED_TASK[1][0]=1; // LED 1 AN
    //LED_TASK[3][0]=1; // LED 3 AN
    /*
    for (uint8_t step = 0; step <= LED_ANZAHL ; step++)
    {

            LED_TASK[step][0]=1;

    }
    */
    putstring("Tempampel Ver 0.2");         // Ausgabe Versionstext Text
    while(1)
    {


        usart_getc_intr();                          // CHECK und Verarbeitung des UART Buffers
        if(qbi(CONTROLLWORD_VOLL,UFLAGS))       // wenn Puffer voll beginnt Bearbeitung
            {
            cbi(CONTROLLWORD_VOLL,UFLAGS);      // Ruecksetzung des FLAG nach Abarbeitung des Eingangspuffer
            if(CONTROLLWORD[1] == ADR485)       // Abarbeitung beginnen wenn Adresse matcht
                {
                switch(CONTROLLWORD[2])     // Abarbeitung der eingegebene Komandos
                    {
                    case 0:                 // Ausgabe Version der Software auf RS485
                    if(CONTROLLWORD[4] == ADR485)   // Pruefnummer Adresse des Devices muss gesetzt sein um INIT zu starten
                        {
                        putstring("CNTR Version 0.2");          // Ausgabe Versionstext Text
                        UART_SendByte(10);                      // Ausgabe Return
                        }
                    break; //END CASE0

                    case 1:                 // Ausgabe Version der Software auf RS485
                    if(CONTROLLWORD[4] == ADR485)   // Pruefnummer Adresse des Devices muss gesetzt sein um INIT zu starten
                        {
                        putstring("LED AN");            // Ausgabe Versionstext Text
                        UART_SendByte(10);                      // Ausgabe Return
                        LED_TASK[CONTROLLWORD[3]][0]=1; // LED  AN
                        }
                    break; //END CASE1

                    case 2:                 // Ausgabe Version der Software auf RS485
                    if(CONTROLLWORD[4] == ADR485)   // Pruefnummer Adresse des Devices muss gesetzt sein um INIT zu starten
                        {
                        putstring("LED AUS");           // Ausgabe Versionstext Text
                        UART_SendByte(10);                      // Ausgabe Return
                        LED_TASK[CONTROLLWORD[3]][0]=0; // LED  AN
                        }
                    break; //END CASE2

                    case 3:                 // Ausgabe Version der Software auf RS485
                    if(CONTROLLWORD[4] == ADR485)   // Pruefnummer Adresse des Devices muss gesetzt sein um INIT zu starten
                        {
                        ztemp = zufall;
                        UART_SendByte(10);                      // Ausgabe Return
                        putstring("Wuerfel: ");          // Ausgabe Versionstext Text
                        errorcodeu(ztemp);
                        wuerfel(ztemp);
                        UART_SendByte(10);                      // Ausgabe Return
                        putstring("Wuerfel Test");           // Ausgabe Versionstext Text
                        UART_SendByte(10);                      // Ausgabe Return
                        }
                    break; //END CASE3

                    case 4:                 // Ausgabe Version der Software auf RS485
                    if(CONTROLLWORD[4] == ADR485)   // Pruefnummer Adresse des Devices muss gesetzt sein um INIT zu starten
                    {
                        ztemp = zufall;
                        UART_SendByte(10);                      // Ausgabe Return
                        putstring("drehen an: ");           // Ausgabe Versionstext Text
                        drehaktiv = 5;
                    }
                    break; //END CASE3
                    }

                }

            }


        if (counter == 50 )
            {
            sbi(6,ADCSRA);                          // startet den Analogdigitalwandler, also die Messung des internen Temperatursensors
            counter++;                              // Damit die Routine nur einmal durchlaufen wird
            }

        if(counter == 64 )
            {
            if (drehaktiv)
                {
                drehenr(drehcounter);
                counter++;
                drehcounter++;
                if (drehcounter >= 7)
                    {
                    drehcounter = 1;
                    drehaktiv--;
                    }
                }



            }

        if (counter < 128 )
            {
            LED_TASK[19][0]=1; // blinken der LED 19 fuer Test
            }
        else
            {
            LED_TASK[19][0]=0;  // blinken der LED 19 fuer Test
            }

        if (counter == 70 )
            {
            templ = ADCL;                           // liest die Temperaturdaten aus dem unteren Byte
            temph = ADCH;                           // liest die Daten aus den unteren Temperaturbyte
            temperaturdaten = HILO(temph,templ);    // schreibt die Daten in eine 16 BIT Variable
            counter++;
            }


        if (counter == 100)
            {
            UART_SendByte('T');
            errorcodeu16(temperaturdaten);
            UART_SendByte(':');
            UART_SendByte(0x0A);                        // LF
            counter++;
            }

            /*
            for (uint8_t step = 0; step <= 9 ; step++)  // Fuer Testzwecke werden die Daten als Binaere Zahl auf die LED`s 1 bis 10 geschrieben.
                {

                if(qbi(step,temperaturdaten))
                    {
                    LED_TASK[step+1][0]=1;                  // Zuschalten des Bits zur zugehoerigen LED
                    }
                else
                    {
                    LED_TASK[step+1][0]=0;                  // Abschalten des Bits zur zugehoerigen LED
                    }
                }

            */

            if (counter == 200)
                {
                ledband(temperaturdaten,330);
                counter++;
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

/***************************************************************************************
Ansteuerung Wuerfelfeld (7 LED`s) um einen zeiger darzustellen

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

/***************************************************************************************
Ansteuerung LED Band auf Basis AD Wandler
***************************************************************************************/

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


/***************************************************************************************
Auswahl Zeilennummer
***************************************************************************************/
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
    // PORTB
    // PORT0 = Zeile 3 (LED 11 bis 15)
    // PORT1 = Zeile 2 (LED 6 bis 10)
    // PORT2 = Zeile 1 (LED 1 bis 5)




    DDRB = 0b00000000;   // werden im Programm gesetzt
    PORTD =0b00000000;


        // PORTC
        // PORT5 / Umschalten Senden (HIGH) / Empfangen (LOW) (485)
        DDRC =  0b00100000;
        PORTC = 0b00000000;

    // PORT D
    // PORT0 = RS232 RX = IN
    // PORT1 = RS232 TX = OUT
    // PORT2 = Spalte 5 (LED 5,10,15)
    // PORT3 = Spalte 4 (LED 4,9,14,19)
    // PORT4 = Spalte 3 (LED 3,8,13,18)
    // PORT5 = Spalte 2 (LED 2,7,12,17)
    // PORT6 = Spalte 1 (LED 1,6,11,16
    // PORT7 =  Zeile 4 (LED 16 bis LED 19)

    DDRD =  0b01111100;
    PORTD = 0b01100000;
}

/*****************************************************************************************
TIMER INIT
TIMER0 > nicht genutzt
TIMER1 > nicht genutzt
TIMER2 > Fuer einen Interrupt aller XX ms
****************************************************************************************/
void TIMER_init(void)
{

    /**************** TIMER 0   *********************************/
    TCCR0B = 0b00000001; // Teiler 1024 3,68MHZ = 259ns*8*256(8BIT) = 0,5ms
    TCCR0A = 0x00;
    OCR0A = 0;          // Output Compare Register
    OCR0B = 0;          // Output Compare Register
    TCNT0 = 0;          // counter Timer 0
    //TIMSK0 = 0b00000001;
    sbi(0,TIMSK0);      // Timer0 Overflow Interrupt Enable loest alle XXX ms aus

    /************** Timer 1 16 BIT Zaehler **************************************/
    //TCCR1B = 0x05;    // Teiler 1024 3,68MHZ = 259ns
    //TCNT1H Zaehler HIGH Byte (8bit)
    //TCNT1L Zaehler  LOW Byte (8bit)

    /*************** TIMER 2 *************************************/

    //TCCR2B = 0x07;    // Teiler 1024 3,68MHZ = 259ns*1024*256(8BIT) = 67,91ms
    TCCR2B = 0x06;      // Teiler  256 3,68MHZ = 259ns* 256*256(8BIT) = 16,97ms
    //TCCR2B = 0x05;    // Teiler  128 3,68MHZ = 259ns* 128*256(8BIT) =  8,49ms
    //TCCR2B = 0x04;    // Teiler   64 3,68MHZ =  83ns*  64*256(8BIT) =  4,248ms
    //TIMSK2 = 0x01;
    sbi(0,TIMSK2);      // Timer0 Overflow Interrupt Enable loest alle XXX ms aus

    /************** Init Analog Digital Wandler, Auslesen des Temperatursensors im Singel Conversations Modus **********/
    ADMUX = 0b11001000;  // Auswahl Temperatursensor BIT4, Internal 1.1V Voltage Reference BIT 6&7
    //ADMUX = 0b11001110;  // Auswahl 1,1V BIT4,3,2, Internal 1.1V Voltage Reference BIT 6&7
    ADCSRA = 0b10000101; // ADC Enable BIT 7,ADC Start Conversion BIT 6,ADC Auto Trigger Enable BIT 5, Vorteiler auf 32 fuer den ADC BIT 1-3
    ADCSRB = 0b00000000; // Timer/Counter1 Overflow startet Wandlung, BIT 1&2 (wird nicht genutzt)
    // ADCL zuerst lesen, dann ADHL

}


/*****************************************************************************************
initialisieren des UART
*****************************************************************************************/
void UART_init(void)
{
    UBRR0H = HIGH(USARTSPEED);                  // Baudrate einstellen
    UBRR0L = LOW(USARTSPEED);                   // Baudrate einstellen
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0); // senden, empfangen,receiveint aktivieren
    //UCSR0B = _BV(TXEN0);                              // senden aktivieren
    //  UCSR0B = _BV(TXEN0) ;                           // senden
    // Frame Format setzen:8data, 1stop bit (URSEL=1 -> UCSRC->Settings werden genutzt)
    UCSR0C = (0<<USBS0)|(3<<UCSZ00);

}



//=======================================================================================
// Sendet ein Byte ueber die UART
//=======================================================================================
void UART_SendByte(uint8_t data)            // sendet ein Byte ueber das Uart
{
    SENDEN_AKTIV;                           // Gibt SENDEN Frei
    rucksetzcount = 3;                      // wird nach 6 Zyklen 48ms wieder rueckgesetz
    _delay_ms(10);
    while(bit_is_clear(UCSR0A, UDRE0));     // warten bis UART bereit ist zum senden
    UDR0 = data;                                // data ausgeben
}


//=======================================================================================
// sendet einen String ueber das UART
//=======================================================================================
void putstring(char *s)                     // setze den Pointer s an den Anfang des uebergebenen chararrays
{
    while (*s != 0)                         // ist der Pointer des Zeichens=0 dann chararray zu Ende
    {
        UART_SendByte(*s);                  // uebergibt das Zeichen an UART_SendByte
        *s++;                               // zeigt auf das naechste Zeichen
    }
}

//=======================================================================================
// sendet den Fehlercode (Wandlung Hexzahl >> ASCII)
//=======================================================================================
void errorcodeu(uint8_t zahl)
{
    uint8_t temp = 0;

    temp = zahl % 10;                       // niederwertigste Ziffer
    zahl = (zahl - temp) / 10;              // Rest von 10 ergibt die Einer
    temp = temp | 0x30;                     // Erzeuge ASCII Zeichen
    if(zahl != 0){  errorcodeu(zahl);   }   // Recursiv function

    UART_SendByte(temp);                    // Sendet Ziffer
}


//=======================================================================================
// sendet den Fehlercode (Wandlung Hexzahl >> ASCII) (16BIT)
//=======================================================================================
void errorcodeu16(int16_t zahl)
{
    uint8_t temp = 0;

    temp = zahl % 10;                       // niederwertigste Ziffer
    zahl = (zahl - temp) / 10;              // Rest von 10 ergibt die Einer
    temp = temp | 0x30;                     // Erzeuge ASCII Zeichen
    if(zahl != 0){  errorcodeu16(zahl); }   // Recursiv function

    UART_SendByte(temp);                    // Sendet Ziffer
}


void warte_sekunde(void)
{
    _delay_ms(250);
    _delay_ms(250);
    _delay_ms(250);
    _delay_ms(250);
}

/*******************************************************************************************
Diese Funktion Liest ein Byte aus dem UART Puffer
Berechnet wieviel Zeichen sich noch im Buffer befinden
Wenn Controllword freigegeben werden die Zeichen dort eingeschrieben.
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


*******************************************************************************************/
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
    //sbi(BUFFEROVR,PORTD);                                                 // Warnlampe fuer BUFFEROVR wird gesetzt (Ruecksetzen erfolgt mit Neustart !!)
    //putstring("[ERR_BUF_OVERFLOW]");                                          // Error fuer Buffer Overflow !!!
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
/***************************************************************************************
INTERRUPT Timerueberlauf (Timer 0)
Einsprung alle XX ms
***************************************************************************************/
ISR(TIMER0_OVF_vect)
{


if ( (LED_TASK[LED_Timer][0]) && ( taskcount < 1 ) )
    {
    zeilenwahl(LED_Timer);  // wenn LED in Array auf 1 gesetzt wir eingeschaltet
    }
else
    {
    zeilenwahl(0);          // sonst wird ausgeschaltet
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

ISR (TIMER0_COMPA_vect)  // timer0 overflow interrupt
{
    //event to be
}


/***************************************************************************************
INTERRUPT Timerueberlauf (Timer 2)
Einsprung alle XX ms
***************************************************************************************/

ISR(TIMER2_OVF_vect)
{
    counter++;
// Funktion zum Ruecksetzen des aktiven Sendekanals der Schnittstelle RS485
if(rucksetzcount)
    {
    if(rucksetzcount == 1)
        {
        SENDEN_INAKTIV;
        //LED1_OFF;                 //RS485 Device inaktiv
        }
    rucksetzcount--;
    }
if (zufall >= 6)
    {
    zufall = 0;
    }
zufall++;


}



/*******************************************************************************************
INTERRUPT ROUTINE fuer UART, schreibt empfangenes Zeichen in den BUFFER

********************************************************************************************/
ISR(USART_RX_vect)
{

//Automatisch empfangene Daten in den Puffer schreiben:
puffer[schreibzeiger]=UDR0;                         // Empfangenes Zeichen in den Buffer schreiben
schreibzeiger++;
if(schreibzeiger==PUFFER_GROESSE) schreibzeiger=0; // Befindet sich Zeiger am Ende, wird dieser zurueckgesetzt
}
