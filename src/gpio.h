/*
 * gpio.c
 *
 * Created: 11/04/2023 17:28:01
 * Author : Arthur DUPONT
 */

#include <avr/io.h>
#include "menu.h"
#include "macros.h"

#ifndef GPIO_H_
#define GPIO_H_

/**
 * Configures all the GPIOs
*/
void initGpio() {
    /*
   * PA0: [I] ADC0 / Potentio Volume
   * PA1: [-] ADC1 / Temperature LM335
   * PA2: [-] ADC2 / I/O Vers Carte (Effet Basse, Actif NL1)
   * PA3: [-] ADC3 / I/O Vers Carte
   * PA4: [-] ADC4 / I/O Vers Carte
   * PA5: [-] ADC5 / I/O Vers Carte
   * PA6: [-] ADC6 / I/O Vers Carte
   * PA7: [-] ADC7 / I/O Vers Carte
   */
  DDRA = 0x00;
  /*
   * PB0: [I] Mesure Vitesse ventilateur
   * PB1: [O] TX to LCD
   * PB2: [I] BP Menu
   * PB3: [O] Pilotage Ventilateur
   * PB4: [-] I/O Vers Carte (Effet Distortion, Actif NL1)
   * PB5: [I] BP -
   * PB6: [I] BP +
   * PB7: [O] MUTE Led
   */
  DDRB = 0x8A;
  /*
   * PC0: [O] Buzzer
   * PC1: [I] BP Mute
   * PC2: [-] JTAG TCK
   * PC3: [-] JTAG TMS
   * PC4: [-] JTAG TDO
   * PC5: [-] JTAG TDI
   * PC6: [-] I/O Vers Carte
   * PC7: [-] I/O Vers carte
   */
  DDRC = 0x01;
  /*
   * PD0: [I] RXD
   * PD1: [O] TXD
   * PD2: [O] CLK Stereo / LED 6
   * PD3: [O] RST Stereo / LED 5
   * PD4: [O] RL12 / LED 4
   * PD5: [O] CLK Mono / LED 3
   * PD6: [O] D / LED 2
   * PD7: [O] RST Mono / LED 1
   */
  DDRD = 0xFE;


  // Temperature/Volume Reading
  // Enable the ADC
  ADCSRA = (1 << ADEN);

  // For the FAN PWM generation
  // Set TIM0 in Fast PWM mode, output on OC0, and with a /8 prescaler
  TCCR0 = (1 << WGM00) | (1 << WGM01) | (1 << COM01) | (1 << CS01);

  // For the fan tach counter
  // Set TIM1A in Normal mode, with a /1024 prescaler
  TCCR1B = (1 << CS11) | (1 << CS12);
}


/**
 * Configures the digital potentiometer with the given data.
 * @details Implements a software SPI-like interface.
 * @param clk The clock pin
 * @param rst The reset pin
 * @param volume The volume level to set
 * @param mute Mute signal
 * @note The digital potentiometer does not completely mute the sound, even when muted.
*/
void setVolume(char clk, char rst, char volume, char mute) {
  // Get the volume %
  float vol = (float)volume / 100.;
  char s0 = (1-vol) * 63.; // Get the step number for the current
  char s1 = (1-vol) * 63.; // volume. the Pot has 64 steps.

  // Pull RST HIGH at the start of the com
  PORTD |= 1 << rst;

  for (uint8_t i = 0; i < 16; i++) {
    // Set data bit
    switch (i) {
    // Swiper 0
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      if (s0 & (1 << i))
        PORTD |= 1 << PD6;
      else
        PORTD &= ~(1 << PD6);
      break;

    // Swiper 1
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
      if (s1 & (1 << (i - 8)))
        PORTD |= 1 << PD6;
      else
        PORTD &= ~(1 << PD6);
      break;

    // Mute
    case 6:
    case 14:
      if (mute)
        PORTD |= 1 << PD6;
      else
        PORTD &= ~(1 << PD6);
      break;
    }

    // Send Clock pulse
    PORTD |= 1 << clk;
    _delay_us(1);
    PORTD &= ~(1 << clk);
    _delay_us(1);
  }

  // Pull RST LOW at the end of the com
  PORTD &= ~(1 << rst);
}
/**
 * Start an ADC conversion and synchronously read the result.
 * ADC must be enabled beforehand.
 * @param pin The ADC pin number to read.
 * @return The 10 bit adc value.
 * @note There is a known issue on the PCB causing the Volume knob to change the temperature reading. This is not a software bug.
*/
uint16_t readADC(uint8_t pin) {
  // Check if the ADC is enabled
  if ( !(ADCSRA & (1 << ADEN)) )
    return 0;

  // Clear the previous ADC interupt
  ADCSRA = (1 << ADEN) | (1 << ADIF);
  // Configure the MUX to the desired pin and the Ref to the AREF pin
  ADMUX = (ADMUX & ~MUX_MASK) | (pin & MUX_MASK);

  // Start the conversion
  ADCSRA |= (1 << ADSC);
  // Wait for the conversion
  while ( ~ADCSRA & (1 << ADIF) );

  return ADC;
}
void handleVolume(uint8_t source_, uint8_t mute_, char* music_title_) {
  char volume = round(readADC(0) / 1023. * 100.);

  setVolume(PD2, PD3, volume, mute_);
  setVolume(PD5, PD7, volume, mute_);

  // Set the digital potentiometer values based on the pot value
  // Update the LCD based on the info from the pot and from the stored title
  if (menuGet() == Stereo) {
    uint8_t dt[] = {music_title_, volume, mute_, source_};
    menuUpdateDynamic(dt);
  }
}

char last_fan = 0; // Store the last FAN tach state
void handleFan(float* fan_rpm_period_) {
  // If the fan rpm is too low and the TIM1 overflows
  if (TIFR & (1 << TOV1)) {
    TIFR = (1 << TOV1); // Reset the flag

    *fan_rpm_period_ = 1; // Set the period to an invalid value
  }

  // Calculate the fan rpm using the timer
  if (__RE(PINB, PB0, last_fan)) {
    // Calculate the period
    *fan_rpm_period_ = TCNT1 * 1.28e-4; // 1.28e-4 is calculated using the prescaler value and F_CPU

    // Reset the TIM1 counter
    TCNT1 = 0;
  }
  last_fan = PINB & (1 << PB0);

  // Read the temp from the LM335
  uint16_t temp_read = readADC(1);

  // Calculate the temperature
  uint16_t temp = (float)temp_read / 1023. * 5000. * 0.01;

  // If the fan menu is displayed, fill in the data
  if (menuGet() == Fan) {
    uint8_t dt[] = {temp, *fan_rpm_period_};
    menuUpdateDynamic(dt);
  }

  // Calculate the target fan rpm
  uint16_t fan_rpm_target = (float)temp / (float)FAN_TEMP_MAX * (float)(FAN_RPM_MAX-FAN_RPM_MIN) + (float)FAN_RPM_MIN;
  
  // Calculate the rpm error
  int16_t fan_rpm_error = (float)fan_rpm_target - (1. / *fan_rpm_period_ * 60.);
  lcdGoto(0,3);

  // Set the fan PWM
  OCR0 = 127 + CLAMP(fan_rpm_error, 125, 127);
}

// Flags used to detect rising edges on buttons
char last_menu = 0; // Store the last MENU button state
char last_bpm = 0; // Store the last BP- button state
char last_bpp = 0; // Store the last BP+ button state
char last_mute = 0; // Store the last MUTE button state
void handleButtons(uint8_t* source_, uint8_t* effects_, uint8_t* mute_) {
  // Menu button ========================================
  if (__RE(PINB, PB2, last_menu)) {
    // Go to the next menu and update on button press
    menuNext();
    menuUpdateStatic();

    if (menuGet() == Effects) menuUpdateDynamic(effects_);
  }
  last_menu = PINB & 1 << PB2;

  // BP- ================================================
  if (__RE(PINB, PB5, last_bpm)) {
    switch (menuGet()) {
      // Toggle the source
      case Stereo: {
        *source_ = !(*source_);

        {
          uint8_t dt[] = {*source_};
          menuUpdateDynamic(dt);
        }
        break;
      }

      // Toggle the Dist effect
      case Effects: {
        if (*effects_ & 0x02)
          *effects_ &= ~0x02;
        else
          *effects_ |= 0x02;

        {
          uint8_t dt[] = {*effects_};
          menuUpdateDynamic(dt);
        }
        break;
      }
    }
  }
  last_bpm = PINB & 1 << PB5;

  // BP+ ================================================
  if (__RE(PINB, PB6, last_bpp)) {
    switch (menuGet()) {
      // Toggle the source
      case Stereo: {
        *source_ = !(*source_);

        {
          uint8_t dt[] = {*source_};
          menuUpdateDynamic(dt);
        }
        break;
      }

      // Toggle the Bass effect
      case Effects: {
        if (*effects_ & 0x01)
          *effects_ &= ~0x01;
        else
          *effects_ |= 0x01;

        {
          uint8_t dt[] = {*effects_};
          menuUpdateDynamic(dt);
        }
        break;
      }
    }
  }
  last_bpp = PINB & (1 << PB6);

  // Mute button ========================================
  if (__RE(PINC, PC1, last_mute)) {
    // Toggle the mute state
    *mute_ = !(*mute_);
  }
  last_mute = PINC & 1 << PC1;
}

#endif