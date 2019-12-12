/*
 * HEADER FILE Temperaturanzeige und Wuerfel
 * Projekt f�r Bausatz DARC S09 > Wuerfel und Temperatranzeige
 * Created: 01.11.2019 18:00:00
 *  Author: uwe Liedtke und Joerg Wunsch
 */


#ifndef UART_DEBUG                     // Kann auf Kommandozeile mit -DUART_DEBUG
                                       // �berschrieben werden
#   define UART_DEBUG   0              // UART abschalten
#endif

// Constants definitions
#define CLEAR       0
//#define F_CPU       3680000UL          // CPU 3.68MHZ
#define F_CPU       12000000UL          // CPU 3.68MHZ
#define NODE_ID     2                  // NODE ID vom Board
#define SENS_ID     1                  // Sensor ID Debug
#define LED_ANZAHL  19                 // Anzahl der LEDs
#define MAX_HELL    17                 // Maximale Helligkeit der LEDs
#define ADR485      13                 // Adresse f�r DEBUG !!!
// Komandos
#define Ausgabe_TEMP        1          // Temperaturausgabe

// DS18b20
/* one-wire IC powerup */
#define OW_POWER_PIN    3

/* one-wire port DS18B20 */
#define OW_PORT         C
#define OW_PIN          2

// Family codes DS18xxx
#define DS18B20             0x28
#define DS1820              0x10


//FLAGS
#define PULS_PAUSE          0          // DISPLAY INIT notwendig
#define WUERFEL_7           1          // Spezialmode wuerfelt bis 7
#define TEMP_OFF            2          // Tempanzeige OFF
#define TEMPISOFF           3          // BIT wird gesetzt damit das LED Temperaturband nur einmal ruckgesetzt wird
#define PLAYER              4          // 0 = Player 1 / 1 = Player 2
#define TASTER              5          // DEBUG Taster Abfrage
#define T_FLAG              6          // Abfrage des Tasters wird durchgefuehrt
#define AD_WANDLER          7          // Start AD Wandler

//SW_FAGS
#define TEMPANZEIGE         0          // TEMPANZEIGE Auffrischen
#define SW_SPERRE           1          // Sperrt kurzzeitig den Taster
#define KALIBRIERUNG        2          // Kalibriermodus interner Temp-Sensor

// Librery declaration
//#include <avr/eeprom.h>
#include <inttypes.h>
#include <stdbool.h>

//Makros

#define sbi(TEST, PORT) PORT |= (1 << TEST)
#define cbi(TEST, PORT) PORT &= ~(1 << TEST)
#define qbi(TEST, PORT) ( (PORT & (0x01 << TEST))>>TEST )
#define LOW(x)  ((x) & 0xFF)
#define HIGH(x)   (((x) >> 8) & 0xFF)
#define HILO(HI,LO) ((unsigned int) (HI) << 8 | (unsigned int) (LO))

#if UART_DEBUG == 1
//RS485
#define BAUD  9600                              // Baudrate
#define STEUERZEICHEN   '>'                     // Steuerzeichen zur Definition des Komandobeginns
#define USARTSPEED  (F_CPU/(BAUD*16L)-1)        // Formel zur Berechnung der Werte f�r UBBRH/UBBRL
#define SENDEN_AKTIV        sbi(5,PORTC)        // Aktivierung der Sendeleitung
#define SENDEN_INAKTIV      cbi(5,PORTC)        // Deaktivierung der Sendeleitung
#define PUFFER_GROESSE 250                      // Puffergr��e in Byte, f�r den UART Eingangspuffer
#define CW_SIZE 5                               // Gr��e des Controllwords

//UFLAG Definition
#define CONTROLLWORD_VOLL   0                   // FLAG f�r die Signalisierung das Daten im UART Buffer sind
extern volatile uint8_t rucksetzcount;           // clear Sendeanforderung
#endif

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

#define JMP_PORT                PORTB         // Jumper PORT PULLUP
#define JMP_DDR                 DDRB          // Datenrichtungsregister
#define JMP_PIN                 PINB          // PIN B
#define SW_PORT                 PINC          // PORT Wuerfelstart
#define SW_WUERFEL              4             // TASTER Wuerfel
#define JMP_MOSI                3             // Jumper Wuerfelmodus 6
#define JMP_MISO                4             // ??
#define JMP_SCK                 5             //

// EEPROM layout
                                       // Byte 0: frei lassen (kann leicht zerst�rt werden)
#define EE_TEMP_KALIB ((uint16_t *)1)  // Byte 1/2: Kalibrierwert Offset Temperatursensor

//#define LED1_ON       ZEILE1_ON & SPALTE1_ON
//#define LED1_OFF  ZEILE1_OFF & SPALTE1_OFF


void multi_player(uint8_t);              // Multi player Mode
uint8_t jumper(void);                    // uebergibt eine Zahl entsprechend der Jumper Stellung
void led_multiplex(uint8_t);             // Steuerung des LED Multiplexers
void wuerfel(uint8_t);                   // Ansteuerung Wuerfel (7 LED`s)
void wuerfellos(uint8_t);                // Startet Wuerfel aktivitaet
void drehenr(uint8_t);                   // Ansteuerung Wuerfelfeld (7 LED`s)
void zeilenwahl(uint8_t);                // Uebergabe der Zeigerstellung
void ledband(double, double, double);    // Ansteuerung LED Band
void PORTs_init(void);                    // PORT INIT
void TIMER_init(bool);                    // Timer INIT
void ADC_init(bool);

void ow_power(bool);                      // One-wire sensor power on/off
bool ow_reset(void);                      // One-wire sensor reset => present?
double Read_Temperature(double);          // Read temperature; argument is divisor:
                                          // 2.0 for DS1820, 16.0 for DS18B20
bool Get_ROMCode(uint8_t *);              // Read ROM Code into buffer provided
void Configure_DS18B20(void);             // Configure DS18B20 for 9-bit operation

#if UART_DEBUG == 1
void UART_init(void);                     // UART INIT
void UART_SendByte(uint8_t);              // sendet Byte > UART
void putstring(char *s);                  // sendet String >UART
void errorcodeu(uint8_t);                 // sendet Fehlercode (Wandlung Hexzahl >> ASCII)
void usart_getc_intr(void);               // Funktion Liest Byte aus UART Puffer
void uart_action(void);                   // UART-bezogener Anteil der Hauptschleife
void uart_timer_action(void);             // UART-bezogener Anteil in Timer-Interrupt
#endif

// Globale Variable
extern volatile uint8_t drehcounter;             // Zahlt die Anzahl der Umlaeufe bevor der Wuerfel faellt
extern volatile uint8_t drehaktiv;               // Hier wird die Anzahl der Umlaeufe festgelegt
extern volatile uint8_t ztemp;                   // Hier steht die Zufahlszahl vom aktuellen Wuerfelumlauf drin
extern volatile uint16_t wzeiger;                // Counter fuer den drehenden Wuerfel
extern volatile uint8_t player1;                  // Spielstand Player 1
extern volatile uint8_t player2;                  // Spielstand Player 2
extern volatile uint8_t mode;                    // wuerfel funktion
extern uint8_t debug;                            // debug Variable
extern uint8_t zufall;                           // Variable fuer Zufallsgenerator
extern uint8_t taskcount;                        // Counter fuer Multiplexer
extern uint8_t FLAGS;                            // Flags, Verwendung siehe Flags declaration
extern uint8_t LED_TASK[20][2];                  // Array LED Ansteuerung [0 = AN/AUS oder Timer
extern uint8_t LED_Timer;                        // Multiplexer
extern uint8_t LED_HELLIGKEIT;                   // Helligkeit , PWM
extern uint8_t T_Sensorwert[5][2];               // Variable [Sensornummer] und [ID/Temperaturwerte]
