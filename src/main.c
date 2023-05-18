/*
 * main.c
 *
 * Created: 15/02/2023 16:43:55
 * Author : Arthur DUPONT
 */


// Config values
#define F_CPU 8000000
#define FAN_RPM_MIN 1200
#define FAN_RPM_MAX 6000
#define FAN_TEMP_MAX 80
#define LCD_USE_SOFTWARESERIAL
#define SOFTWARESERIAL_BAUD 19200
#define MAX_USART_RX 100

// Libs
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "gpio.h"
#include "macros.h"
#include "LCD.h"
#include "menu.h"
#include "usart.h"


int main() {
  initGpio();

  // State vars
  uint8_t mute = 0;           // Current mute state
  uint8_t menu = 0;           // Current selected menu
  uint8_t source = 0;         // Current selected source
  uint8_t effects = 0;        // Current effects | bit0 = Bass, bit1 = Dist
  float fan_rpm_period = 1;   // Current fan period
  char music_title[65] = {0}; // Current music playing title

  lcdInit(); // Start LCD
  lcdSetCursor(0); // Hide cursor

  // Start USB
  usartInit();

  // Display default menu
  menuInit(&menu);
  menuUpdateStatic();

  while (1) {
    // Music title
    if (usartCharAvail()) {
      char rx_buf[MAX_USART_RX+1] = {0};

      size_t i=0;
      for (; i < MAX_USART_RX; i++) {
        char rd_char[] = {usartReadChar(), 0};
        if (rd_char[0] == '\n') break;

        strcat(rx_buf, rd_char);
      }

      char cmd[] = "cTitle";
      memcpy(cmd, rx_buf, 7);
      if (strcmp(cmd, "cTitle")) memcpy(music_title, rx_buf+7, 16);
    }


    // Buttons ==================================
    handleButtons(&source, &effects, &mute);

    // Volume ===================================
    handleVolume(source, mute, music_title);
	
    // Fan regulation ===========================
    handleFan(&fan_rpm_period);
    
    // Output the effects values ================
    if (effects & 0x01) // Bass
      PORTA |= (1 << PA2);
    else
      PORTA &= ~(1 << PA2);
    
    if (effects & 0x02) // Dist
      PORTB |= (1 << PB4);
    else
      PORTB &= ~(1 << PB4);

    // Toggle the stereo source =================
    if (source)
      PORTD |= 1 << PD4;
    else
      PORTD &= ~1 << PD4;

    // Mute led =================================
    if (mute)
      PORTB |= (1 << PB7);
    else
      PORTB &= ~(1 << PB7);
  }

  return 0;
}