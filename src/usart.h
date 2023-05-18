/*
 * usb.h
 *
 * Created: 16/54/2023 11:14:01
 * Author : Arthur DUPONT
 */
#ifndef USB_H_
#define USB_H_

#define BAUDRATE 9600

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * Configures the USART Peripheral
 * @note Must be called before any other USART function
*/
void usartInit() {
    const uint16_t baud_prescaler = (uint16_t) ((float)F_CPU / 16. * (float)BAUDRATE) - 1; // Calc the baud prescaler config
    UBRRH = (baud_prescaler>>8) & 0x0F;
    UBRRL = baud_prescaler & 0xFF;

    UCSRB = (1<<RXCIE) | (1<<TXCIE); // Enable TX/RX interrupts
    UCSRB |= (1<<RXEN) | (1<<TXEN); // Enable the tranceivers based on the buffers
    UCSRC = (1<<URSEL) | (1<<UCSZ0) | (1<<UCSZ1); // Set frame format: Async, 8data, 1stop, no parity
}

/**
 * Sends one byte through serial
 * @note Blocking function
*/
void usartPutChar(char byte_) {
    while ( !( UCSRA & (1<<UDRE)) );
    UDR = byte_;
}

/**
 * Sends a string through serial
 * @note Blocking function
*/
void usartPrint(char* str) {
    for (size_t i=0; str[i] != '\0'; i++)
        usartPutChar(str[i]);
}

/**
 * Checks if a byte is read to be read within the UART UDR register
 * @returns Boolean, if UDR (RX) is full
*/
char usartCharAvail() {
    return (UCSRA & (1<<RXC));
}

/**
 * Read the content of UDR register
 * @returns The character contained in UDR
 * @note This function is blocking while the register is not filled
*/
char usartReadChar() {
    while ( !(UCSRA & (1<<RXC)) );

    return UDR;
}

#endif