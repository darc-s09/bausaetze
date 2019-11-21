#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "thermo.h"

#include <util/delay.h>

#if UART_DEBUG == 1
uint8_t T_Sensorwert[5][2];               // Variable [Sensornummer] und [ID/Temperaturwerte]
uint8_t lesezeiger;                             // Lesezeiger, notwendig für die Abholroutine (UART BUFFER)
uint8_t schreibzeiger;                          // Schreibzeiger, notwendig für das richtige schreiben in den UART BUFFER
int     UARTINDEX;                              // Index für Arrya UART_BUFFER
uint8_t zeicheninbuffer;                        // díese Variable zeigt an wieviel Zeichen sich noch im UART Buffer befinden
uint8_t UFLAGS;                                 // Beschreibung siehe UFLAG Definition
unsigned char puffer[PUFFER_GROESSE];           // BUFFER für UART Eingang
uint8_t CONTROLLWORD[10];                       // Controllwort zur Steuerung des Controllers
volatile uint8_t rucksetzcount;           // clear Sendeanforderung

/*
 * UART-Teil der Hauptschleife
 */
void uart_action(void)
{
  usart_getc_intr();                      // CHECK und Verarbeitung des UART Buffers

  if (qbi(CONTROLLWORD_VOLL,UFLAGS))       // Daten im Puffer > Run
    {
      cbi(CONTROLLWORD_VOLL,UFLAGS);      // Clear FLAG "RUN"
      if (CONTROLLWORD[1] == ADR485)       // Check Richtige Adresse
        {
          switch (CONTROLLWORD[2])         // Abarbeitung switch
            {
            case 0:                     // Ausgabe Software Version
              if (CONTROLLWORD[4] == ADR485) // CRC CHECK 
                {
                  putstring("Version 0.4");   // Ausgabe Versionstext Text
                  UART_SendByte(10);               // Ausgabe Return
                }
              break;

            case 1:
              if (CONTROLLWORD[4] == ADR485)       // CRC CHECK
                {
                  putstring("Wuerfelmode:");		// Ausgabe Text
                  errorcodeu(CONTROLLWORD[3]);	// Wuerfel Funktion wird festgelegt
                  mode = CONTROLLWORD[3];
                  UART_SendByte(10);              // Ausgabe Return
                }
              break;

            case 3:
              if (CONTROLLWORD[4] == ADR485)       // CRC CHECK
                {
                  // Simulation Taster
                  sbi(TASTER,FLAGS); // DEBUG TASTER
                }
              break;

            case 4:
              if (CONTROLLWORD[4] == ADR485)      // CRC CHECK
                {
                  ztemp = zufall;                // Zufahlszahl
                  UART_SendByte(10);             // Ausgabe Return
                  putstring("wuerfeln: ");      // Ausgabe Versionstext Text
                  //drehaktiv = CONTROLLWORD[3];
                  drehaktiv = rand()%6+1;
                  errorcodeu(ztemp); //DEBUG
                }
              break;

            case 5:
              if (CONTROLLWORD[4] == ADR485)      // CRC CHECK
                {
                  UART_SendByte(10);             // Ausgabe Return
                  putstring("Multiplayer: ");   // Ausgabe Funktion
                  multi_player(CONTROLLWORD[3]);
                }
              break;
            }
        }
    }



/*****************************************************************************
 DEBUG
 Test Jumperabfrage fuer die Wuerfelfunktion
 Wuerfelmode:
 0 = Temperaturanzeige
 1 = Wuerfel Singelmode 1 - 6
 2 = Wuerfel Singelmode 1 - 7
 3 = Multiplayer Mode   1 - 6
 4 = Multiplayer Mode   1 - 7
 5 = Temperaturanzeige mit DS1820
 6 = Test LEDs alle einschalten

******************************************************************************/
  if (qbi(TASTER,FLAGS))
	{
      switch(mode)
        {
        case 0:                         // Temperaturanzeige
          putstring("TEMPANZEIGE AN:");   // Ausgabe Text
          UART_SendByte(10);              // Ausgabe Return
          cbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AN
          cbi(TEMPISOFF,FLAGS);			// Ruecksetzen TEMPIS OFF
          for (uint8_t step = 1; step <= 19 ; step++)
            {
              LED_TASK[step][0] = 0;       // Anzeige loeschen
            }
          break;

        case 1:	                        // Wuerfelsingelmode 1-6
          sbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AUS
          UART_SendByte(10);              // Ausgabe Return
          putstring("Wuerfel Spezialmode 6"); // Ausgabe Text
          cbi(WUERFEL_7,FLAGS);			// Wuerfelspezialmode 6
          UART_SendByte(10);              // Ausgabe Return
          multi_player(0);                // Singelplayer
          break;

        case 2:	                        // Wuerfelsingelmode 1-7
          sbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AUS
          UART_SendByte(10);              // Ausgabe Return
          putstring("Wuerfel Spezialmode 7"); // Ausgabe Text
          sbi(WUERFEL_7,FLAGS);			// Wuerfelspezialmode 7
          UART_SendByte(10);              // Ausgabe Return
          multi_player(0);                // Singelplayer
          break;

        case 3:	                        // Wuerfel Multiplayer 1-6
          sbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AUS
          UART_SendByte(10);              // Ausgabe Return
          putstring("Wuerfel Spezialmode 6"); // Ausgabe Text
          cbi(WUERFEL_7,FLAGS);			// Wuerfelspezialmode 6
          UART_SendByte(10);              // Ausgabe Return
          if (qbi(PLAYER,FLAGS))
            {
              multi_player(1);                // Multiplayer	1
              cbi(PLAYER,FLAGS); // TEST
            }
          else
            {
              multi_player(2);                // Multiplayer	2
              sbi(PLAYER,FLAGS); //TEST
            }
          break;

        case 4:	                        // Wuerfelmultiplayer 1-7
          sbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AUS
          UART_SendByte(10);              // Ausgabe Return
          putstring("Wuerfel Spezialmode 7"); // Ausgabe Text
          sbi(WUERFEL_7,FLAGS);			// Wuerfelspezialmode 7
          UART_SendByte(10);              // Ausgabe Return
          if (qbi(PLAYER,FLAGS))
            {
              multi_player(1);                // Multiplayer	1
              cbi(PLAYER,FLAGS); // TEST
            }
          else
            {
              multi_player(2);                // Multiplayer	2
              sbi(PLAYER,FLAGS); //TEST
            }
          break;

        case 5:
          // Noch nicht implementiert
          break;

        case 6:
          // Alle LEDs an
          sbi(TEMP_OFF,FLAGS);			// Temperaturanzeige AUS
          sbi(TEMPISOFF,FLAGS); 
          for (uint8_t step = 1; step <= 19 ; step++)
            {
              LED_TASK[step][0] = 1;
            }
          break;
		}

      cbi(TASTER,FLAGS); // DEBUG TASTER
	}
}

/******************************************************************************
INTERRUPT ROUTINE fuer UART, schreibt empfangenes Zeichen in den BUFFER
Empfangene Zeichen werden in den Puffer geschrieben 
Ist der Zeiger am Ende des Puffesr wird dieser an den Anfang gesetzt
******************************************************************************/
ISR(USART_RX_vect)
{
  puffer[schreibzeiger] = UDR0;
  schreibzeiger++;
  if (schreibzeiger == PUFFER_GROESSE)
    schreibzeiger = 0;
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
}



/******************************************************************************
Sendet ein Byte ueber die UART
******************************************************************************/
void UART_SendByte(uint8_t data)
{
    SENDEN_AKTIV;                         // RX > TX Umschaltung
    rucksetzcount = 3;                    // Delay zur Rueckschaltung RX
    _delay_ms(10);
    while (bit_is_clear(UCSR0A, UDRE0));   // wartet auf UART Ready
    UDR0 = data;
}


/******************************************************************************
sendet einen String ueber UART
******************************************************************************/
void putstring(char *s)
{
    while (*s != 0)
    {
        UART_SendByte(*s++);
    }
}

/******************************************************************************
sendet den Fehlercode (Wandlung Hexzahl >> ASCII)
******************************************************************************/
void errorcodeu(uint8_t zahl)
{
    uint8_t temp;

    temp = zahl % 10;                       // niederwertigste Ziffer
    zahl = zahl / 10;

    if (zahl != 0)
      errorcodeu(zahl);  // Rekursion für führende Ziffern

    UART_SendByte(temp + '0');                    // Sendet Ziffer
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

  if (lesezeiger > schreibzeiger)                                         // Pruefung Uebertrag
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

  if (zeicheninbuffer && !qbi(CONTROLLWORD_VOLL,UFLAGS))                       // Wenn Zeichen im Buffer und Controllbuffer nicht gesperrt ist erfolgt Abarbeitung
    {
      do
        {

          if (puffer[lesezeiger] == STEUERZEICHEN)                             // hier befindet sich das Steuerzeichen zum ruecksetzen des Zeigers
            {
              UARTINDEX = CLEAR;                                              // das Zeichen > setzt den INDEX vom Controllword auf 0 zurueck
            }

          if ((UARTINDEX >= 0) && (UARTINDEX < CW_SIZE))                     // nur wenn sich der INDEX zwischen 0 und Buffergroeße befindet
            {
              CONTROLLWORD[UARTINDEX] = puffer[lesezeiger];                   // wird das aktuelle Zeichen im Buffer ins Controllword geschrieben
              UARTINDEX++;                                                    // INDEX wird um eins erhoeht
            }

          if ((UARTINDEX == CW_SIZE) && (CONTROLLWORD[0] == STEUERZEICHEN))  // Wenn Controllword gefuellt, wird geprueft ob es sich beim ersten Zeichen um das Steuerzeichen handelt
            {
              sbi(CONTROLLWORD_VOLL,UFLAGS);                                  // wenn beide Bedingung erfuellt, wird Controllword fuer Weiterbearbeitung freigegeben und gleichzeitig fuers beschreiben gesperrt
              CONTROLLWORD[0] = 0;                                            // Controllword "0" loeschen
            }
          lesezeiger++;                                                       // Lesezeiger um eins erhoehen
          zeicheninbuffer--;                                                  // Da keine Neuberechnung in der Schleife fuer die Anzahl der Zeichen im UART Buffer erfolgt wird manuell eins runtergezaehlt
          if (lesezeiger==PUFFER_GROESSE) lesezeiger=0;                        // wenn Lesepuffer am Ende ruecksetzen
        }
      while (zeicheninbuffer && !qbi(CONTROLLWORD_VOLL,UFLAGS));          // wenn keine Zeichen mehr im UART Buffer sind oder das Controllword gesperrt wurde wird Schleife verlassen

    }

}

void uart_timer_action(void)
{
  if (rucksetzcount)
    {
      if (rucksetzcount == 1)
        {
          SENDEN_INAKTIV;
        }
      rucksetzcount--;
    }
}

#endif

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
