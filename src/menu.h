/*
 * menu.h
 *
 * Created: 11/04/2023 15:44:36
 *  Author: Arthur DUPONT
 */ 

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "LCD.h"

#ifndef MENU_H_
#define MENU_H_

enum MenuList {
  Stereo,
  Effects,
  Fan,
  Credit,
  MENU_OVERFLOW
};
uint8_t* menu_pt;
/**
 * Set the container for the menu counter
*/
void menuInit(uint8_t* target_pt) {
  menu_pt = target_pt;
}
/**
 * Get the current selected menu
*/
uint8_t menuGet() {
  return *menu_pt;
}
/**
 * Set the current selected menu.
 * @note You must update the menu manually.
*/
void menuSet(uint8_t index) {
  if (index > 0 && index < MENU_OVERFLOW)
    *menu_pt = index;
}
/**
 * Selects the previous menu and wraps around the menu list if needed.
 * @note You must update the menu manually.
*/
void menuPrev() {
  (*menu_pt)--;
  if (*menu_pt<0)
    *menu_pt = MENU_OVERFLOW - 1;
}
/**
 * Selects the next menu and wraps around the menu list if needed.
 * @note You must update the menu manually.
*/
void menuNext() {
  (*menu_pt)++;
  if (*menu_pt >= MENU_OVERFLOW)
    *menu_pt = 0;
}
/**
 * Prints the static parts of the current menu
 * Call this each time you want to change menu
*/
void menuUpdateStatic() {
  lcdClear();
  
  // Print the menu content
  char l1[17] = {0};
  char l2[17] = {0};
  char l3[17] = {0};
  char l4[17] = {0};
  switch (*menu_pt) {
    case Stereo: {
      strcat(l1, "[Stereo]");
      strcat(l1, "Title: Unknown  ");
      strcat(l3, "Volume: 000%%  ");
      strcat(l4, "Source: [RCA]");
      break;
    }

    case Effects: {
      strcat(l1, "[Effects]");
      break;
    }

    case Fan: {
      strcat(l1, "[Fan]");
      strcat(l2, "T:   C RPM:     ");
      break;
    }

    case Credit: {
      strcat(l1, "G111   2022-2023");
      strcat(l2, "Git: Angers-SAE2");
      strcat(l3, " Arthur  DUPONT ");
      strcat(l4, "  Mael   GADOU  ");
      break;
    }
  }
  lcdGoto(0, 0);
  lcdPrint(l1);
  lcdGoto(0, 1);
  lcdPrint(l2);
  lcdGoto(0, 2);
  lcdPrint(l3);
  lcdGoto(0, 3);
  lcdPrint(l4);
}
/**
 * Updates the dynamic parts of the menu
 * Can be called as much as you want
 * @param dt The data mayload to send to the current menu
*/
void menuUpdateDynamic(uint8_t* dt) {
  switch (*menu_pt) {
    case Stereo: {
      // Print the music title ========
      lcdGoto(1, 7);
      lcdPrint((char*)dt[0]);

      // Print the volume =============
      // Convert the volume to string
      char volume_txt[5] = "000%%";
      sprintf(volume_txt, "%3d%%", dt[1]);

      // Display MUTE when muted
      if (dt[2])
        sprintf(volume_txt, "MUTE");
      // Print
      lcdGoto(2, 8);
      lcdPrint(volume_txt);

      // Print the source =============
      lcdGoto(3, 8);
      if (dt[3])
        lcdPrint("[Jack]");
      else
        lcdPrint("[RCA ]");
      
      break;
    }
    case Effects: {
      // Bass FX
      lcdGoto(0, 1);
      if (dt[0] & 0x01)
        lcdPrint(" [Bass]");
      else
        lcdPrint("  Bass ");
      // Dist FX
      if (dt[0] & 0x02)
        lcdPrint(" [Dist]");
      else
        lcdPrint("  Dist ");
    }
    case Fan: {
      // Convert the temp to text
      char temp_txt[5] = "00C";
      sprintf(temp_txt, "%2dC", dt[0]);
      // Print the temp to the lcd
      lcdGoto(3, 1);
      lcdPrint(temp_txt);

      // Convert the fan RPM to text
      char fan_rpm_txt[5] = "0000";
      if (dt[1] <= 0) dt[1] = 1;
      sprintf(fan_rpm_txt, "%4d", (int)(1. / dt[1] * 60.));
      // Print the RPM to the lcd
      lcdGoto(16-4,1);
      lcdPrint(fan_rpm_txt);
      break;
    }
  }
}

#endif