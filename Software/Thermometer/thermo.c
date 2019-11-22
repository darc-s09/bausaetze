/*
  "THE BEER-WARE LICENSE" (Revision 42):
  Uwe Liedtke and Joerg Wunsch wrote this file.  As long as you retain this notice you
  can do whatever you want with this stuff. If we meet some day, and you think
  this stuff is worth it, you can buy us a beer in return.
*/
/*
  Thermometer.c

  Created: 01.11.2019 14:00:00
  Author:  Uwe Liedtke und Joerg Wunsch
  Projekt: Temperaturanzeige und Wuerfel
  Funktionen:
  Temperaturanzeige
  Wuerfel mit 6 und 7 Ziffern
  Zweispielermode 20 Gewinnt

  fuer Atmega 168A
 */


#include "thermo.h"

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t counter;                 // Counter fuer Taskmanager
volatile uint8_t drehcounter;             // Zahlt die Anzahl der Umlaeufe bevor der Wuerfel faellt
volatile uint8_t drehaktiv;               // Hier wird die Anzahl der Umlaeufe festgelegt
volatile uint8_t ztemp;                   // Hier steht die Zufahlszahl vom aktuellen Wuerfelumlauf drin
volatile uint16_t wzeiger;                // Counter fuer den drehenden Wuerfel
volatile uint8_t player1;                 // Spielstand Player 1
volatile uint8_t player2;                 // Spielstand Player 2
volatile uint8_t mode;                    // wuerfel funktion
volatile uint16_t temperaturdaten;
double temp_1wire;

uint8_t debug;                            // debug Variable
uint8_t zufall;                           // Variable fuer Zufallsgenerator
uint8_t taskcount;                        // Counter fuer Multiplexer
uint8_t SW_FLAGS;                         // Flags, Verwendung siehe Flags declaration
uint8_t FLAGS;                            // Flags, Verwendung siehe Flags declaration
uint8_t LED_TASK[20][2];                  // Array LED Ansteuerung [0 = AN/AUS oder Timer
uint8_t LED_Timer;                        // Multiplexer
uint8_t LED_HELLIGKEIT;                   // Helligkeit , PWM


double temp_ds18b20(void)
{
    ow_power(true);
    _delay_ms(10);
    double tfloat = Read_Temperature();
    ow_power(false);

    return tfloat;
}


int main(void)
{

    bool ds18b20_present = ow_reset();

    LED_HELLIGKEIT = 0;
    drehcounter = 0;
    PORTs_init();                       // Init der Ein und Ausgabeports
    TIMER_init();                       // Timer Init
#if UART_DEBUG == 1
    UART_init();                        // INIT 485
#endif
    sei();                              // INTERRUPTS GLOBAL AN
    LED_TASK[1][0]=1;                   // LED 1 AN nach INIT
    //DEBUG
    //LED_TASK[1][0]=1; // LED 1 AN
    //LED_TASK[3][0]=1; // LED 3 AN
    /*
    for (uint8_t step = 0; step <= LED_ANZAHL ; step++)
    {

            LED_TASK[step][0]=1;

    }
    */
#if UART_DEBUG == 1
    putstring("Tempanzeige und Wuerfel Ver 0.4");         // Ausgabe Versionstext Text
#endif

    while (1)
    {
#if UART_DEBUG == 1
        uart_action();
#endif

/*****************************************************************************
 Abfrage Taster zum starten des Wuerfels und Auswertung der Jumper


******************************************************************************/
        if (qbi(T_FLAG, FLAGS))
        {
            if (!qbi(SW_WUERFEL, SW_PORT))
            {
                switch (mode)
                {
                case 0:                         // Temperatur Anzeige
                    cbi(TEMP_OFF, FLAGS);       // Temperaturanzeige AN
                    cbi(TEMPISOFF, FLAGS);      // Ruecksetzen TEMPIS OFF
                    for (uint8_t step = 1; step <= 19 ; step++)
                    {
                        LED_TASK[step][0] = 0;  // LEDs aus
                    }
                    break;

                case 1:                         // Wuerfel
                    sbi(TEMP_OFF, FLAGS);       // Temperaturanzeige AUS
                    cbi(WUERFEL_7, FLAGS);      // Wuerfel 1 - 6
                    multi_player(0);            // Singelplayer
                    break;

                case 2:                         // Wuerfel
                    sbi(TEMP_OFF, FLAGS);       // Temperaturanzeige AUS
                    sbi(WUERFEL_7, FLAGS);      // Wuerfel 1 - 7
                    multi_player(0);            // Singelplayer
                    break;

                case 3:                         // Wuerfel
                    sbi(TEMP_OFF, FLAGS);       // Temperaturanzeige AUS
                    cbi(WUERFEL_7, FLAGS);      // Wuerfel 1 - 6
                    if (qbi(PLAYER, FLAGS))
                    {
                        multi_player(1);        // Player1
                        cbi(PLAYER, FLAGS);
                    }
                    else
                    {
                        multi_player(2);        // Player 2
                        sbi(PLAYER, FLAGS);
                    }
                    break;

                case 4:                         // Wuerfel
                    sbi(TEMP_OFF, FLAGS);       // Temperaturanzeige AUS
                    sbi(WUERFEL_7, FLAGS);      // Wuerfel 1 - 7
                    if (qbi(PLAYER, FLAGS))
                    {
                        multi_player(1);        // Player1
                        cbi(PLAYER, FLAGS);
                    }
                    else
                    {
                        multi_player(2);         // Player2
                        sbi(PLAYER, FLAGS);
                    }
                    break;

                case 5:
                    // Noch nicht implementiert
                    break;

                case 6:
                    // Alle LEDs an
                    sbi(TEMP_OFF, FLAGS);        // Temperaturanzeige AUS
                    sbi(TEMPISOFF, FLAGS);
                    for (uint8_t step = 1; step <= 19 ; step++)
                    {
                        LED_TASK[step][0] = 1;
                    }
                    break;
                }
                cbi(T_FLAG, FLAGS);
            }
        }


/*****************************************************************************
 startet den Analogdigitalwandler
 zum Auslesen des int Temp Sensor

******************************************************************************/
         if (qbi(AD_WANDLER, FLAGS))
         {
             cbi(AD_WANDLER, FLAGS);
             if (ds18b20_present)
             {
                 temp_1wire = temp_ds18b20();
                 sbi(TEMPANZEIGE, SW_FLAGS);
             }
             else
             {
                 sbi(6, ADCSRA); // Wandlung starten
                 //sbi(ADIE,ADC); // Interrupt ein
             }
         }



/******************************************************************************
Ausgabe der Temperaturdaten auf das LED Band
Ist Temp OFF gesetzt werden die TEMP LEDS geloescht
FLAG TEMPISOFF wird gesetzt damit das LED Temperaturband nur einmal ruckgesetzt wird
******************************************************************************/
         if (qbi(TEMP_OFF, FLAGS))
         {
             if (!qbi(TEMPISOFF, FLAGS))
             {
                 sbi(TEMPISOFF, FLAGS);
                 for (uint8_t step = 0; step <= 10 ; step++)
                 {
                     LED_TASK[step][0] = 0;
                 }
             }
         }
         else
         {
             if (qbi(TEMPANZEIGE,SW_FLAGS))
             {
                 cbi(TEMPANZEIGE, SW_FLAGS);
                 if (ds18b20_present)
                     ledband(temp_1wire, 42); // XXX
                 else
                     ledband(temperaturdaten, 75);
             }
         }

/******************************************************************************
Im Multiplayer Mode
Hier wird nach dem Wuerfeln die Anzeige des aktuellen Spielstandes aktualisiert
Ist der Spielstand kleiner 10 blinkt die erste LED der Anzeige
******************************************************************************/
         if ((drehaktiv == 0) && (qbi(TEMPISOFF, FLAGS)) && (mode > 2))
         {
             if (qbi(PLAYER, FLAGS))
             { // PLAYER 2
                 if (player2 > 10)
                 {
                     for (uint8_t step = 1; step <= (player2 - 10) ; step++)
                     {
                         LED_TASK[step][0] = 1;
                     }
                 }
                 else
                 {
                     if (qbi(4, counter))
                     {
                         LED_TASK[1][0] = 1;
                     }
                     else
                     {
                         LED_TASK[1][0] = 0;
                     }
                 }
             }
             else
             { //PLAYER 1
                 if (player1 > 10)
                 {
                     for (uint8_t step = 1; step <= (player1 - 10) ; step++)
                     {
                         LED_TASK[step][0] = 1;
                     }
                 }
                 else
                 {
                     if (qbi(4, counter))
                     {
                         LED_TASK[1][0] = 1;
                     }
                     else
                     {
                         LED_TASK[1][0] = 0;
                     }
                 }
             }
         }


/******************************************************************************
Wenn dreaktiv gesetzt ist startet der Wuerfel,
drehaktiv bestimmt die Anzahl der Umlaufe
Uebergabe der Geschwindigkeit des Wuerfels
******************************************************************************/

         if (drehaktiv)
         {
             wuerfellos(3);
         }
    }
}



// Hier beginnen die Funktionen


/******************************************************************************
Multi player Mode
case 0 Spielstart
case 1 Aufruf Spieler 1
case 2 Aufruf Spieler 2

******************************************************************************/
void multi_player(uint8_t playernr)
{
    sbi(TEMP_OFF, FLAGS);            // Temperaturanzeige AUS
    ztemp = zufall;
    for (uint8_t step = 1; step <= 10 ; step++)
    {
        LED_TASK[step][0] = 0; //LEDs 1 bis 10 aus
    }
    LED_TASK[18][0] = 0; // LED 10 AUS
    LED_TASK[19][0] = 0; // LED 19 AUS

    switch (playernr)
    {
    case 0:
        player1 = 0;
        player2 = 0;
        break;

    case 1:
        cbi(PLAYER, FLAGS); // PLAYER 1
        LED_TASK[18][0] = 1; // LED 10 AN
        LED_TASK[19][0] = 0; // LED 19 AUS
        player1 = player1 + ztemp;
#if UART_DEBUG == 1
        errorcodeu(player1); // DEBUG
#endif
        break;

    case 2:
        sbi(PLAYER,FLAGS); // PLAYER 2
        LED_TASK[18][0] = 0; // LED 10 AUS
        LED_TASK[19][0] = 1; // LED 19 AN
        player2 = player2 + ztemp;
#if UART_DEBUG == 1
        errorcodeu(player2); // DEBUG
#endif
        break;
    }
    drehaktiv = rand() % 6 + 1;
}


/***************************************************************************************
Steuerung des LED Multiplexers
ueber den Uebergabeparameter wird festgelegt welche LED eingeschaltet wird
Die Globale Variable LED Helligkeit legt fest mit welcher Helligkeit (Pulsweite) die LED´s angesteuert werden
***************************************************************************************/

void led_multiplex(uint8_t ledcount)
{
    if (MAX_HELL < LED_HELLIGKEIT )
    {
        LED_HELLIGKEIT = MAX_HELL;  // hier wird die vorgegebene Helligkeit auf den maximal zulaessigen Wert begrenzt
    }

    if (LED_Timer)
    {                       // wenn LED Timer fuer Multiplexer 0, dann kann die naechste LED geschaltet werden
                            // Diese Funktion steuert das Multiplexing und das Puls-/Pausenverhaeltnis der LEDs
        LED_Timer--;        // besser ueber Interrupt
        if (LED_Timer <= (MAX_HELL - LED_HELLIGKEIT))
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
Ansteuerung Wuerfel (7 LEDs) Zahlen von 1 bis 7 sind moeglich
***************************************************************************************/
void wuerfel(uint8_t wzahl)
{
    for (uint8_t step = 11; step <= 17 ; step++)    // Abschalten aller 7 LEDs
    {
        LED_TASK[step][0] = 0;                      // Abschalten aller LEDs auf dem Wuerfelfeld
    }
    switch (wzahl)
    {
    case 1:                 // Wuerfel Zahl 1
        LED_TASK[11][0] = 1;      // Wuerfel Zahl 1 >  LED 11
        break;                  // Ende case

    case 2:                 // Wuerfel Zahl 2
        LED_TASK[14][0] = 1;      // Wuerfel Zahl 2 >  LED 14
        LED_TASK[16][0] = 1;      // Wuerfel Zahl 2 >  LED 16
        break;                  // Ende case

    case 3:                 // Wuerfel Zahl 3
        LED_TASK[11][0] = 1;      // Wuerfel Zahl 3 >  LED 11
        LED_TASK[14][0] = 1;      // Wuerfel Zahl 3 >  LED 14
        LED_TASK[16][0] = 1;      // Wuerfel Zahl 3 >  LED 16
        break;                  // Ende case

    case 4:                 // Wuerfel Zahl 4
        LED_TASK[14][0] = 1;      // Wuerfel Zahl 4 >  LED 14
        LED_TASK[15][0] = 1;      // Wuerfel Zahl 4 >  LED 15
        LED_TASK[16][0] = 1;      // Wuerfel Zahl 4 >  LED 16
        LED_TASK[17][0] = 1;      // Wuerfel Zahl 4 >  LED 17
        break;                  // Ende case

    case 5:                 // Wuerfel Zahl 5
        LED_TASK[11][0] = 1;      // Wuerfel Zahl 5 >  LED 11
        LED_TASK[14][0] = 1;      // Wuerfel Zahl 5 >  LED 14
        LED_TASK[15][0] = 1;      // Wuerfel Zahl 5 >  LED 15
        LED_TASK[16][0] = 1;      // Wuerfel Zahl 5 >  LED 16
        LED_TASK[17][0] = 1;      // Wuerfel Zahl 5 >  LED 17
        break;                  // Ende case

    case 6:                 // Wuerfel Zahl 6
        LED_TASK[12][0] = 1;      // Wuerfel Zahl 6 >  LED 12
        LED_TASK[13][0] = 1;      // Wuerfel Zahl 6 >  LED 13
        LED_TASK[14][0] = 1;      // Wuerfel Zahl 6 >  LED 14
        LED_TASK[15][0] = 1;      // Wuerfel Zahl 6 >  LED 15
        LED_TASK[16][0] = 1;      // Wuerfel Zahl 6 >  LED 16
        LED_TASK[17][0] = 1;      // Wuerfel Zahl 6 >  LED 17
        break;                  // Ende case

    case 7:                 // Wuerfel Zahl 7
        LED_TASK[11][0] = 1;      // Wuerfel Zahl 7 >  LED 11
        LED_TASK[12][0] = 1;      // Wuerfel Zahl 7 >  LED 12
        LED_TASK[13][0] = 1;      // Wuerfel Zahl 7 >  LED 13
        LED_TASK[14][0] = 1;      // Wuerfel Zahl 7 >  LED 14
        LED_TASK[15][0] = 1;      // Wuerfel Zahl 7 >  LED 15
        LED_TASK[16][0] = 1;      // Wuerfel Zahl 7 >  LED 16
        LED_TASK[17][0] = 1;      // Wuerfel Zahl 7 >  LED 17
        break;                  // Ende case
    }

}


/******************************************************************************
 Ansteuerung des Wuerfels
 Startet mit einen drehenden Feld im Wuerfel Segment
 Parameteruebergabe:
 drehg (Geschwindigkeit des Zeigers)
Die Variable wzeiger bestimmt die Geschwindigkeit und wird im Timer2 hochgezaehlt
******************************************************************************/
void wuerfellos(uint8_t drehg)
{
    if (qbi(drehg, wzeiger))
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
    }
}

/***************************************************************************************
Ansteuerung Wuerfelfeld (7 LEDs) um einen drehenden Zeiger darzustellen
Wird genutzt um eine Wuerfel Aktivitaet darzustellen
Uebergabe der Zeigerstellung
***************************************************************************************/
void drehenr(uint8_t wstep)
{
    for (uint8_t step = 11; step <= 17 ; step++)    // Abschalten aller 7 LEDs
    {
        LED_TASK[step][0] = 0;                        // Abschalten aller LEDs auf dem Wuerfelfeld
    }

    switch (wstep)
    {
    case 1:                 // Step 1 -
        LED_TASK[11][0] = 1;      // LED 11
        LED_TASK[12][0] = 1;      // LED 12
        LED_TASK[13][0] = 1;      // LED 13
        break;                  // Ende case

    case 2:                 // Step 2 nr
        LED_TASK[11][0] = 1;      // LED 11
        LED_TASK[15][0] = 1;      // LED 15
        LED_TASK[17][0] = 1;      // LED 17
        break;                  // Ende case

    case 3:                 // Step 3 /
        LED_TASK[11][0] = 1;      // LED 11
        LED_TASK[14][0] = 1;      // LED 14
        LED_TASK[16][0] = 1;      // LED 16
        break;                  // Ende case

    case 4:                 // Step 4 -
        LED_TASK[11][0] = 1;      // LED 11
        LED_TASK[12][0] = 1;      // LED 12
        LED_TASK[13][0] = 1;      // LED 13
        break;                  // Ende case

    case 5:                 // Step 5 nr
        LED_TASK[11][0] = 1;      // LED 11
        LED_TASK[15][0] = 1;      // LED 15
        LED_TASK[17][0] = 1;      // LED 17
        break;                  // Ende case

    case 6:                 // Step 6 /
        LED_TASK[11][0] = 1;      // LED 11
        LED_TASK[14][0] = 1;      // LED 14
        LED_TASK[16][0] = 1;      // LED 16
        break;                  // Ende case
    }
}

/******************************************************************************
Ansteuerung LED Band auf Basis AD Wandler
******************************************************************************/

void ledband(uint16_t tempwert, uint16_t tempset)
{
    if (tempwert < tempset)
    {
        for (uint8_t step = 0; step <= 9 ; step++)  // Fuer Testzwecke werden die Daten als Binaere Zahl auf die LEDs 1 bis 10 geschrieben.
        {
            LED_TASK[step + 1][0] = 0;                  // Abschalten aller LEDs der Temperaturanzeige
        }
    }

    for (uint8_t step = 0; step <= 9 ; step++)      // Zur Ansteuerung des LED Temperaturbandes
    {
        tempset++;
        if ((tempwert >= tempset) && (tempwert <= (tempset + 2))) // Hier wird festgelegt in welchen Bereich die LED's zugeschaltet werden.
        {
            LED_TASK[step + 1][0] = 1;                  // Zuschalten LED auf den Band
        }
        else
        {
            LED_TASK[step + 1][0] = 0;                  // Abschalten der nichtaktiven LED auf den Band
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

    /*
     * Timer 0: LED-Multiplexing, im Überlauf-Interrupt
     */
    TCCR0B = _BV(CS00); // Vorteiler 1, bei 3,68 MHz => 69us => 14kHz
                        // bei 12 MHz => 47kHz
    TIMSK0 = _BV(TOIE0); // Timer0 Overflow Interrupt Enable

   /*
     * Timer 1: allgemeine Zeitablaufsteuerung
     */
#if F_CPU < 10000000
    TCCR1B = _BV(CS12); // Teiler  256, bei 3,68 MHz = 4,56s
#else
    TCCR1B = _BV(CS10) | _BV(CS12); // Teiler 1024, bei 12 MHz = 5,6s
#endif
    TIMSK1 = _BV(TOIE2); // Timer2 Overflow Interrupt Enable

    /*
     * Timer 2: allgemeine Zeitablaufsteuerung
     */
#if F_CPU < 10000000
    TCCR2B = _BV(CS22) | _BV(CS21); // Teiler  256, bei 3,68 MHz 52 Hz Rate
#else
    TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20); // Teiler 1024, bei 12 MHz = 21,84ms
#endif
    TIMSK2 = _BV(TOIE2); // Timer2 Overflow Interrupt Enable

/************************ Analog Digital Wandler Singel **********************/
    // Interne Referenz 1,1 V; Kanal 8 (interner Temperatursensor)
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
#if F_CPU < 10000000
    // ADC enable, Vorteiler 32 => 115 kHz Takt bei 3,68 MHz
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0) | _BV(ADIE);
#else
    // ADC enable, Vorteiler 64 => 187 kHz bei 12 MHz
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADIE);
#endif
}

/******************************************************************************
INTERRUPT Timer 0 Timerueberlauf
Einsprung alle XX ms
Dieser Timer wird fuer den Multiplexer genutzt
*******************************************************************************/
ISR(TIMER0_OVF_vect)
{

    if ((LED_TASK[LED_Timer][0]) && ( taskcount < 1 ))
    {
        zeilenwahl(LED_Timer);  // LED Ein
    }
    else
    {
        zeilenwahl(0);          // LED Aus
    }

    taskcount++;
    if (taskcount > (MAX_HELL - LED_HELLIGKEIT))
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
INTERRUPT Timer 1 Timerueberlauf
16 BIT Timer
Einsprung alle 5s

*******************************************************************************/
ISR(TIMER1_OVF_vect)
{
    sbi(AD_WANDLER, FLAGS); // Start Temperaturmessung interner Wandler
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
Abfrage Taster alle 20ms
******************************************************************************/

ISR(TIMER2_OVF_vect)
{

// Funktion zum Ruecksetzen des aktiven Sendekanals der Schnittstelle RS485

#if UART_DEBUG == 1
    uart_timer_action();
#endif

// Test Zufallsgenerator fuer den Wuerfel
    if (qbi(WUERFEL_7, FLAGS))
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
    sbi(T_FLAG, FLAGS);  // Abfrage Taster

    zufall++;
    wzeiger++;
}

/******************************************************************************
INTERRUPT Analog Digital CONVERTER
Auslesen der Temperaturdaten aus den AD Wandler
Wenn aktuelle Daten vorliegen wird dieser ausgelesen und in 8 BIT Messwet gewandelt
******************************************************************************/
ISR(ADC_vect)
{
    temperaturdaten = ADC;
    sbi(TEMPANZEIGE, SW_FLAGS);      // Aufrischen der Temperaturanzeige
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
