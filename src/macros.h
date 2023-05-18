/*
 * macros.h
 *
 * Created: 11/04/2023 17:49:00
 *  Author: Arthur DUPONT
 */ 


#ifndef MACROS_H_
#define MACROS_H_

// Helper macros
// ADC MUX Mask (0b11111)
#define MUX_MASK ((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3) | (1 << MUX4))
/**
 * Rising edge detection
 * @param reg A pointer to the PINx register to read
 * @param pin The pin to watch
 * @param x External variable giving the previous state of the pin
*/
#define __RE(reg, pin, x) (reg & 1 << pin && (reg & 1 << pin) != x && !x)
/**
 * Falling edge detection
 * @param reg A pointer to the PINx register to read
 * @param pin The pin to watch
 * @param x External variable giving the previous state of the pin
*/
#define __FE(reg, pin, x) (reg & 1 << pin && (reg & 1 << pin) != x && x)
/**
 * Return a constrained version of X, between (including) min and max
 * @param x The number to clamp
 * @param min The minimum value allowed
 * @param max The maximum value allowed
 * @return The clamped version of X
 * @note X MUST be a number
*/
#define CLAMP(x, min, max) ( ((x <= min) * min) + ((x >= max) * max) + ((x > min && x < max) * x) )

#endif