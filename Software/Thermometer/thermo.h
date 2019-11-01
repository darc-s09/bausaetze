/*
 * HEADER FILE Temperaturanzeige und Wuerfel
 * Projekt für Bausatz DARC S09 > Wuerfel und Temperatranzeige
 * Created: 01.11.2019 18:00:00
 *  Author: uwe Liedtke und Joerg Wunsch
 */


// Constants definitions
#define CLEAR       0
#define F_CPU       3680000UL          // CPU 3.68MHZ
#define NODE_ID     2                  // NODE ID vom Board
#define SENS_ID     1                  // Sensor ID Debug
#define LED_ANZAHL  19                 // Anzahl der LEDs
#define MAX_HELL    17                 // Maximale Helligkeit der LEDs
#define ADR485      13                 // Adresse für DEBUG !!!

// Komandos
#define Ausgabe_TEMP        1          // Temperaturausgabe


//FLAGS
#define PULS_PAUSE          0          // DISPLAY INIT notwendig

// Librery declaration
//#include <avr/eeprom.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>



//Makros

#define sbi(TEST, PORT) PORT |= (1 << TEST)
#define cbi(TEST, PORT) PORT &= ~(1 << TEST)
#define qbi(TEST, PORT) ( (PORT & (0x01 << TEST))>>TEST )
#define LOW(x)  ((x) & 0xFF)
#define HIGH(x)   (((x) >> 8) & 0xFF)                               
#define HILO(HI,LO) ((unsigned int) (HI) << 8 | (unsigned int) (LO))

//RS485
#define BAUD  9600								// Baudrate 
#define STEUERZEICHEN   '>'                     // Steuerzeichen zur Definition des Komandobeginns
#define USARTSPEED  (F_CPU/(BAUD*16L)-1)        // Formel zur Berechnung der Werte für UBBRH/UBBRL
#define SENDEN_AKTIV        sbi(5,PORTC)        // Aktivierung der Sendeleitung
#define SENDEN_INAKTIV      cbi(5,PORTC)        // Deaktivierung der Sendeleitung
#define PUFFER_GROESSE 250                      // Puffergröße in Byte, für den UART Eingangspuffer
#define CW_SIZE 5                               // Größe des Controllwords
uint8_t lesezeiger;                             // Lesezeiger, notwendig für die Abholroutine (UART BUFFER)
uint8_t schreibzeiger;                          // Schreibzeiger, notwendig für das richtige schreiben in den UART BUFFER
int     UARTINDEX;                              // Index für Arrya UART_BUFFER
uint8_t zeicheninbuffer;                        // díese Variable zeigt an wieviel Zeichen sich noch im UART Buffer befinden
uint8_t UFLAGS;                                 // Beschreibung siehe UFLAG Definition
unsigned char puffer[PUFFER_GROESSE];           // BUFFER für UART Eingang
uint8_t CONTROLLWORD[10];                       // Controllwort zur Steuerung des Controllers

//UFLAG Definition
#define CONTROLLWORD_VOLL   0                   // FLAG für die Signalisierung das Daten im UART Buffer sind

/* Definition LED Matrix
PD2 = Anode LED  5,10,15
PD3 = Anode LED  4,9,14,19
PD4 = Anode LED  3,8,13,18
PD5 = Anode LED  2,7,12,17
PD6 = Anode LED  1,6,11,16
PD7 = Kathode LED 16,17,18,19

PB0 = Kathode LED 11,12,13,14,15 
PB1 = Kathode LED  6, 7, 8, 9,10
PB2 = Kathode LED  1, 2, 3, 4, 5
*/

#define ZEILE1_ON               sbi(2,DDRB)  //!< LED 1 bis 5
#define ZEILE1_OFF              cbi(2,DDRB)  //!< LED 1 bis 5
#define ZEILE2_ON               sbi(1,DDRB)  //!< LED 6 bis 10
#define ZEILE2_OFF              cbi(1,DDRB)  //!< LED 6 bis 10
#define ZEILE3_ON               sbi(0,DDRB)  //!< LED 11 bis 15
#define ZEILE3_OFF              cbi(0,DDRB)  //!< LED 11 bis 15
#define ZEILE4_ON               sbi(7,DDRD)  //!< LED 16 bis 19
#define ZEILE4_OFF              cbi(7,DDRD)  //!< LED 16 bis 19

#define SPALTE1_ON              sbi(6,PORTD)  //!< LED 1,6,11,16
#define SPALTE1_OFF             cbi(6,PORTD)  //!< LED 1,6,11,16
#define SPALTE2_ON              sbi(5,PORTD)  //!< LED 2,7,12,17
#define SPALTE2_OFF             cbi(5,PORTD)  //!< LED 2,7,12,17
#define SPALTE3_ON              sbi(4,PORTD)  //!< LED 3,8,13,18
#define SPALTE3_OFF             cbi(4,PORTD)  //!< LED 3,8,13,18
#define SPALTE4_ON              sbi(3,PORTD)  //!< LED 4,9,14,19
#define SPALTE4_OFF             cbi(3,PORTD)  //!< LED 4,9,14,19
#define SPALTE5_ON              sbi(2,PORTD)  //!< LED 5,10,15
#define SPALTE5_OFF             cbi(2,PORTD)  //!< LED 5,10,15
//#define LED1_ON       ZEILE1_ON & SPALTE1_ON
//#define LED1_OFF  ZEILE1_OFF & SPALTE1_OFF





void led_multiplex(uint8_t);
void wuerfel(uint8_t);
void drehenr(uint8_t);
void zeilenwahl(uint8_t);
void ledband(uint16_t,uint16_t);
void PORTs_init(void);                    // PORT INIT
void TIMER_init(void);                    // Timer INIT
void UART_init(void);                     // UART INIT
void UART_SendByte(uint8_t);              // sendet Byte > UART
void putstring(char *s);                  // sendet String >UART
void errorcodeu(uint8_t);                 // sendet Fehlercode (Wandlung Hexzahl >> ASCII)
void errorcodeu16(int16_t);               // sendet den Fehlercode (Wandlung Hexzahl >> ASCII)
void warte_sekunde(void);                 // delay 1 Sekunde
void usart_getc_intr(void);               // Funktion Liest Byte aus UART Puffer


// Globale Variable
volatile uint8_t counter;                 // Counter für Taskmanager
volatile uint8_t rucksetzcount;           // clear Sendeanforderung
uint8_t debug;                            // debug Variable	
uint8_t zufall;                           // Variable für Zufallsgenerator
uint8_t taskcount;                        // Counter für Multiplexer
uint8_t FLAGS;                            // Flags, Verwendung siehe Flags declaration
uint8_t LED_TASK[20][2];                  // Array LED Ansteuerung [0 = AN/AUS oder Timer
uint8_t LED_Timer;                        // Multiplexer
uint8_t LED_HELLIGKEIT;                   // Helligkeit , PWM
uint8_t T_Sensorwert[5][2];               // Variable [Sensornummer] und [ID/Temperaturwerte]
