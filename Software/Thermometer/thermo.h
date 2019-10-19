/*
 * HEADER FILE Für Temperaturüberwachung Fussbodenheizung
 * Auswertung der Fussbodentemperatursensoren 
 * Created: 25.10.2017 18:00:00
 *  Author: uwe
 */ 


// Constants definitions
#define CLEAR		0
#define	F_CPU		3680000UL					// CPU 3.68MHZ
#define NODE_ID		2							// NODE ID vom Board 
#define SENS_ID		1							// Sensor ID Debug
#define LED_ANZAHL	19							// Anzahl der zu multiplexende LEDs
#define MAX_HELL	17							// Maximale Helligkeit der LEDs 
#define ADR485		13							// Adresse für RS485 TEST !!!

// Komandos 
#define Ausgabe_TEMP		1							// Temperaturausgabe 


//FLAGS 
#define PULS_PAUSE			0					// DISPLAY INIT notwendig 
// Librery declaration
//#include <avr/io.h>
//#include <stdint.h>
//#include "MyMessage.h"
//#include <stdlib.h> //??
//#include <stdio.h> //??
//#include <util/delay.h>
//#include <inttypes.h>
//#include <compat/twi.h>
//#include <avr/interrupt.h>
//#include <avr/wdt.h>
//#include <avr/pgmspace.h> // TEST COMMAND MODE
//#include <string.h>		  // TEST COMMAND MODE
//#include <avr/eeprom.h>

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>
//#include <compat/twi.h>
//#include <avr/wdt.h>




#define sbi(TEST, PORT) PORT |= (1 << TEST)
#define cbi(TEST, PORT) PORT &= ~(1 << TEST)
#define qbi(TEST, PORT) ( (PORT & (0x01 << TEST))>>TEST )
#define LOW(x)  ((x) & 0xFF)											// Makro zum Lowbyte Zugriff eines Word
#define HIGH(x)   (((x) >> 8) & 0xFF)									// Makro zum Highbyte Zugriff eines Word
#define HILO(HI,LO) ((unsigned int) (HI) << 8 | (unsigned int) (LO)) 	// Makro um aus zwei Bytes ein Wort zu erstellen

//RS485
#define BAUD  9600
#define STEUERZEICHEN	'>'						// Steuerzeichen zur Definition des Komandobeginns
#define USARTSPEED  (F_CPU/(BAUD*16L)-1)		/* Formel zur Berechnung der Werte für UBBRH/UBBRL */
#define SENDEN_AKTIV		sbi(5,PORTC)		// Aktivierung der Sendeleitung
#define SENDEN_INAKTIV		cbi(5,PORTC)		// Deaktivierung der Sendeleitung
#define PUFFER_GROESSE 250 						//Puffergröße in Byte, für den UART Eingangspuffer
#define CW_SIZE	5								// Größe des Controllwords
uint8_t lesezeiger;								// Lesezeiger, notwendig für die Abholroutine (UART BUFFER)
uint8_t schreibzeiger;							// Schreibzeiger, notwendig für das richtige schreiben in den UART BUFFER
int		UARTINDEX;								// Index für Arrya UART_BUFFER
uint8_t zeicheninbuffer;						// díese Variable zeigt an wieviel Zeichen sich noch im UART Buffer befinden
uint8_t UFLAGS; 								// Beschreibung siehe UFLAG Definition
unsigned char puffer[PUFFER_GROESSE];			// BUFFER für UART Eingang
uint8_t CONTROLLWORD[10];						// Controllwort zur Steuerung des Controllers
//UFLAG Definition
#define CONTROLLWORD_VOLL	0					// FLAG für die Signalisierung das der Buffer Bearbeitet werden muss.


// PORTS
//#define REL_ON				sbi(4,PORTD)		// REL EIN 
//#define REL_OFF				cbi(4,PORTD)		// REL AUS
//#define REL_CHK				qbi(4,PIND)			// Prüft den Status des RELAIS
//#define TASTERDEBUG			qbi(7,PIND)			// SONDERKOMMANDO 

#define ZEILE1_ON				sbi(2,DDRB)  //!< LED 1 bis 5
#define ZEILE1_OFF				cbi(2,DDRB)  //!< LED 1 bis 5
#define ZEILE2_ON				sbi(1,DDRB)  //!< LED 6 bis 10
#define ZEILE2_OFF				cbi(1,DDRB)  //!< LED 6 bis 10 	
#define ZEILE3_ON				sbi(0,DDRB)  //!< LED 11 bis 15
#define ZEILE3_OFF				cbi(0,DDRB)  //!< LED 11 bis 15 
#define ZEILE4_ON				sbi(7,DDRD)  //!< LED 16 bis 19
#define ZEILE4_OFF				cbi(7,DDRD)  //!< LED 16 bis 19

#define SPALTE1_ON				sbi(6,PORTD)  //!< LED 1,6,11,16
#define SPALTE1_OFF				cbi(6,PORTD)  //!< LED 1,6,11,16
#define SPALTE2_ON				sbi(5,PORTD)  //!< LED 2,7,12,17
#define SPALTE2_OFF				cbi(5,PORTD)  //!< LED 2,7,12,17
#define SPALTE3_ON				sbi(4,PORTD)  //!< LED 3,8,13,18
#define SPALTE3_OFF				cbi(4,PORTD)  //!< LED 3,8,13,18
#define SPALTE4_ON				sbi(3,PORTD)  //!< LED 4,9,14,19
#define SPALTE4_OFF				cbi(3,PORTD)  //!< LED 4,9,14,19
#define SPALTE5_ON				sbi(2,PORTD)  //!< LED 5,10,15
#define SPALTE5_OFF				cbi(2,PORTD)  //!< LED 5,10,15
//#define LED1_ON		ZEILE1_ON & SPALTE1_ON 		
//#define LED1_OFF	ZEILE1_OFF & SPALTE1_OFF 	




							
void led_multiplex(uint8_t);
void wuerfel(uint8_t);
void drehenr(uint8_t);
void zeilenwahl(uint8_t);
void ledband(uint16_t,uint16_t);
void PORTs_init(void);							// PORT INIT
void TIMER_init(void);							// Timer INIT
void UART_init(void);
void UART_SendByte(uint8_t);					// sendet ein Byte über  UART
void putstring(char *s);						// sendet einen String über UART
void errorcodeu(uint8_t);						//sendet den Fehlercode (Wandlung Hexzahl >> ASCII)
void errorcodeu16(int16_t);						//sendet den Fehlercode (Wandlung Hexzahl >> ASCII)
void warte_sekunde(void);						// Wartet 1 Sekunde	
void usart_getc_intr(void);						// Diese Funktion Liest ein Byte aus dem UART Puffer


// Globale Variable
volatile uint8_t counter;								// Counter für Taskmanager 
volatile uint8_t rucksetzcount;					// zum Rücksetzen der Sendeanforderung 
uint8_t debug;
uint8_t zufall;									// Variable für Zufallsgenerator							
uint8_t taskcount;								// Counter für Multitaskbetrieb 
uint8_t FLAGS;									// Flags, Verwendung siehe Flags declaration 
uint8_t LED_TASK[20][2];						// LED Nummer [0 = AN/AUS oder Timer
uint8_t LED_Timer;								// Für Multiplexer
uint8_t LED_HELLIGKEIT;							// Für Helligkeit , Tastverhältnis
uint8_t T_Sensorwert[5][2];						// Variable für die [Sensornummer] und [ID/Temperaturwerte] 
//uint16_t displaycount;							// Conter zum abschalten des Displays
//uint8_t	ausgabecount;							// wird benötigt um die Anzahl der Ausgabe auf den BUS zu begrenzen



//REGISTER

//#define TEMP_P				PORTC	// Digitale Sensoren
//#define TEMP_D				DDRC	// Datenrichtungsregister für Temperatursensor
//#define TEMP_PIN			PINC	// PIN_Register  für Temperatursensor







