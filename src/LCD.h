/*
 * LCD.h
 *
 * Created: 15/02/2023 16:58:19
 *  Author: Arthur DUPONT
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>

#ifdef LCD_USE_SOFTWARESERIAL
#include "softwareserial.h"
#endif

#ifndef LCD_H_
#define LCD_H_



/**
 * Init the USART Peripheral to for the lcd
 * @param baud_rate The baud rate of the USART peripheral. Must be 19200 for the LCD
*/
void lcdUSARTInit(float baud_rate) {
    UBRRH = 0x00; UBRRL = (float) F_CPU / (16. * baud_rate) - 1; // Set USART bitrate
    UCSRB = 1<<TXEN | 1<<RXEN; // Enable USART TX/RX
    UCSRC = 1<<URSEL | 1<<UCSZ1 | 1<<UCSZ0; // Set frame settings: Async, 8 Data bits, no parity, 1 Stop Bit
}

/**
 * Sends one byte synchronously through serial
 * init() MUST be called once before using
*/
void lcdPutChar(char byte) {
	#ifdef LCD_USE_SOFTWARESERIAL
	softwareSerialSend(byte);
	#else
	// TODO: Implement USART Again
	#endif
}

/**
 * Clears the lcd screen and waits for 10ms
 * init() MUST be called once before using 
 * @note Waits 10ms for the LCD to clear
*/
void lcdClear() {
	lcdPutChar(0xA3); // Go into CMD mode
	lcdPutChar(0x01); // Clear the LCD and set cursor to 0,0
	_delay_ms(10); // Wait for the LCD to clear
}

/**
 * Initialize the LCD
 * MUST be called before using any of the functions
 * @note Waits 60ms for the LCD to start
*/
void lcdInit() {
	uint8_t pin_tx = PB1;
	#ifdef LCD_USE_SOFTWARESERIAL
	softwareSerialInit(&DDRB, &pin_tx, NULL);
	#else
	lcdUSARTInit(19200);
	#endif
	lcdPutChar(0xA0); // Initialize LCD
	_delay_ms(50); // Wait for the LCD to start
	lcdClear(); // Clear the LCD (waits 10ms)
}

/**
 * Sets the cursor position to the given coordinates
 * init() MUST be called once before using
 * @param x The target X pos
 * @param y The target Y pos
 * @note Waits 10ms for the LCD to move
*/
void lcdGoto(uint8_t x, uint8_t y) {
	lcdPutChar(0xA1); // Send the MOVE byte
	lcdPutChar(x);
	lcdPutChar(y);
	_delay_ms(10); // Wait for the LCD to move
}

/**
 * Sends a string through serial
 * init() MUST be called once before using
 * @param string The CString to print in ASCII on the LCD
 * @note Invisible characters (ASCII Code <= 31) can be customized, see LCD datasheet for ref
*/
void lcdPrint(char* string) {
	lcdPutChar(0xA2); // Go into ASCII mode
	// Send each character
    for (size_t i=0; string[i] != 0; i++)
        lcdPutChar(string[i]);
    lcdPutChar(0x00); // Quit the ASCII mode
}

/**
 * Set the visibility of the LCD cursor
 * init() MUST be called once before using
 * @param b Boolean, cursor enable
*/
void lcdSetCursor(uint8_t b) {
	lcdPutChar(0xA3); // Send the CMD byte
	if (b)
		lcdPutChar(0x0E);
	else
		lcdPutChar(0x0C);
}


#endif /* LCD_H_ */