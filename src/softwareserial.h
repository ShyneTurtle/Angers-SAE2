/*
 * softwareserial.h
 *
 * Created: 07/03/2023 09:06:39
 *  Author: Arthur DUPONT
 */ 

#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>

#ifndef SOFTWARESERIAL_H_
#define SOFTWARESERIAL_H_

volatile uint8_t* software_serial_reg = NULL;
uint8_t software_serial_tx = 0, software_serial_rx = 0;
#ifndef SOFTWARESERIAL_BAUD
#define SOFTWARESERIAL_BAUD 9600
#endif

/**
 * Configures the DDR register to use with the software serial
 * @param The A pointer to the DDR register
*/
void softwareSerialInitReg(volatile uint8_t* reg) {
	software_serial_reg = reg;
}

/**
 * Configures the TX pin of the software serial
 * @param tx The TX pin to use within the selected port
*/
void softwareSerialInitTx(uint8_t tx) {
	software_serial_tx = tx;

	// Configure the TX pin
	software_serial_reg[0] |= (1<<tx); // Output
	software_serial_reg[1] |= (1<<tx); // Default level high
}
/**
 * Configures the RX pin of the software serial
 * @param rx The RX pin to use within the selected port
*/
void softwareSerialInitRx(uint8_t rx) {
	software_serial_rx = rx;

	// Configure the RX pin
	software_serial_reg[0] &= ~(1<<rx); // Input
}
/**
 * Initialize the software serial port with the given pins
 * You can set the baud rate of the software serial by defining
 * SOFTWARESERIAL_BAUD (value)
 * @param reg A pointer to the DDR register of the port you want to select
 * @param tx A pointer to the TX pin you want to use
 * @param rx A pointer to the RX pin you want to use
*/
void softwareSerialInit(volatile uint8_t* reg, uint8_t* tx, uint8_t* rx) {
	softwareSerialInitReg(reg);

	// Configure the TX pin
	if (tx != NULL)
		softwareSerialInitTx(*tx);
	// Configure the RX pin
	if (rx != NULL)
		softwareSerialInitRx(*rx);
}
/**
 * Sends a byte synchronously on the asynchronous software serial bus.
 * @note 1Start bit, 8Data bits, 1Stop bit
*/
void softwareSerialSend(char byte) {
	float d_us = 1./(float)SOFTWARESERIAL_BAUD*1000000.;
	float d_cpu_us = 1./(float)F_CPU*1000000.;

    // Start bit (pull down)
	software_serial_reg[1] &= ~(1<<software_serial_tx);
    _delay_us(d_us-d_cpu_us*20.);

    // Data bits
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & (1<<i))
            software_serial_reg[1] |= (1<<software_serial_tx);
        else
            software_serial_reg[1] &= ~(1<<software_serial_tx);

        _delay_us(d_us-d_cpu_us*60.);
    }

    // Stop bit (pull up)
    software_serial_reg[1] |= (1<<software_serial_tx);
    _delay_us(d_us-d_cpu_us*20.);
}

#endif