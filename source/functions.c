/*
 *  GTA Trophies
 *  Copyright (C) 2022-2023, Freakler
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <systemctrl.h>
#include <pspctrl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "main.h" // for LOG
#include "utils.h" // for LOG
#include "functions.h"

register int gp asm("gp");

/// externs
extern int LCS;
extern int VCS;

extern int pplayer;
extern int pcar;

extern u32 global_gametimer;
extern u32 global_ismultiplayer;
extern u32 global_currentisland;
extern u32 global_systemlanguage;
extern u32 global_weather;
extern u32 global_carscrushed;

extern u32 global_helpbox;
extern u32 global_helpbox_string;
extern u32 global_helpbox_timedisplayed;
extern u32 global_helpbox_displaybool;
extern u32 global_helpbox_permboxbool;
extern u32 global_helpbox_duration;

extern u32 global_ScriptSpace;
extern u32 global_empireowned;

extern u32 var_garageslots;
extern u32 var_garageslotsize;

extern u32 global_usjdone;
extern u32 global_usjtotal;
extern u32 global_exportedVehicles;
extern u32 global_exportVehicTotal;

extern u32 global_hitmankilled;
extern u32 global_rampagesdone;
extern u32 global_rampagestotal;
extern u32 global_hiddenpkgfound;
extern u32 global_bikessold;
extern u32 global_balloonsburst;


int getGametime() {
  return getInt(global_gametimer + (LCS ? 0 : gp));
}

int getMultiplayer() { 
  return getInt(global_ismultiplayer + (LCS ? 0 : gp)); 
}

int getIsland() { 
  // 1 = Portland, 2 = Staunton, 3 = ShoresideVale 
  // 1 = Vice Beach (right), 2 = Mainland (left), 3 = Stadium
  return getInt(global_currentisland + (LCS ? 0 : gp));    
}

int getLanguage() {  // 0 = English, 1 = French, 2 = German, ....
  return getInt(global_systemlanguage + (LCS ? 0 : gp)); 
}

float getVehicleSpeed(int vehicle_base_adr) {
  return getFloat(vehicle_base_adr + (LCS ? 0x124 : 0x108));
}

unsigned char getPedID(int ped_base_adr) { 
  if( ped_base_adr ) 
    return getShort(ped_base_adr+0x58);
  else return 0xFF; // unknown
}

short getVehicleID(int vehicle_base_adr) { 
  if( vehicle_base_adr ) 
    return getShort(vehicle_base_adr+0x58);
  else return 0; // not in car
}

short getWeather() {
  return getShort(global_weather + (LCS ? 0 : gp));
}

char getNumberEmpiresOwned() {
  return getByte(global_empireowned + gp);
}

int getWantedLevel(int ped_base_adr) {
  return getInt(ped_base_adr + (LCS ? 0x830 : 0x910));
}

void setTimedTextbox(const char *sentence, float duration) { 
  short i;
  int base = -1;
  short length = 0x200;

  if( LCS ) base = global_helpbox_string; // DAT_00649dc0_string (0x8E4DDC0)
  if( VCS ) base = getInt(global_helpbox+gp) + 0x243C; // iGp000016d8 + 0x243C
    
  for( i = 0; sentence[i] != '\0'; i++, base += 0x2 ) setByte(base, sentence[i]);
  for ( ; i < length; i++, base += 0x2 ) setByte(base,0); //clear the rest
    
  if( LCS ) {
    setInt(global_helpbox_timedisplayed, 0);      // reset displaying time - DAT_0035a808 (0x08B5E808)
    setByte(global_helpbox_displaybool, 0x1);     // display bool - DAT_0035a810 (0x08B5E810)
    setByte(global_helpbox_permboxbool, 0x0);     // clear permanent box if there is any - DAT_0035a815 (0x08B5E815)
    setFloat(global_helpbox_duration, duration);  // set duration - DAT_0035a818 (0x08B5E818)
    //setByte(global_helpbox_duration, 0xAA);     // set my indicator (to allow custom textbox while in menu and blocked flag) see blockTextBox()
  }
  if( VCS ) {
    setInt(getInt(global_helpbox+gp) + 0x2A3C, 0);          // reset displaying time            
    setByte(getInt(global_helpbox+gp) + 0x2a44, 0x1);       // display bool + 0x2a44
    setByte(getInt(global_helpbox+gp) + 0x2a49, 0x0);       // clear permanent box if there is any + 0x2a49
    setFloat(getInt(global_helpbox+gp) + 0x2a4C, duration); // set duration                
  }
    
}

int isTextboxShowing() {
  if( LCS ) return getByte(global_helpbox_displaybool);
  if( VCS ) return getByte(getInt(global_helpbox+gp) + 0x2a44);
  return -1;
}

void CustomScriptExecute(int address) {
  int loadadr = address - getInt(global_ScriptSpace + (LCS ? 0 : gp)); // address of script relative to script space (negative not intended but works! as long as no jumps!!!)
  StartNewScript(loadadr);
}

int getCarsCrushed() { // DAT_0035a280_STAT_
  return getInt(global_carscrushed);
}

int getGarageVehicleActiveObjects(int base) {
  int ret = 0, i;
  for( i = 0; i < var_garageslots; base+=var_garageslotsize, i++ ) {
    if( getShort(base) )
      ret++;      
  }
  return ret;
}

int getUsjDone() {
  return getInt(global_usjdone + (LCS ? 0 : gp));
}
int getUsjTotal() {
  return getInt(global_usjtotal + (LCS ? 0 : gp));
}

int getExportVehiclesDone() {
  return getInt(global_exportedVehicles + (LCS ? 0 : gp));
}
int getExportVehiclesTotal() {
  return getInt(global_exportVehicTotal + (LCS ? 0 : gp));
}

int getPedDrowning(int ped_base_adr) { // not in car!
  if( LCS ) 
    return (getInt(ped_base_adr + 0x19c) & 0x10000) != 0; // 0526: is_char_drowning_in_water
  //if( VCS ) 
	//return ...
  return 0;
}
float getPedHealth(int ped_base_adr) {
  return getFloat(ped_base_adr + (LCS ? 0x4B8 : 0x4E4));
}
int getKilledHitmen() { // LCS only
  return getInt(global_hitmankilled);
}
int getRampagesDone() {
  return getInt(global_rampagesdone + (LCS ? 0 : gp));
}
int getRampagesTotal() {
  return getInt(global_rampagestotal + (LCS ? 0 : gp));
}
int getHiddenPackagesFound() { // LCS only
  return getInt(global_hiddenpkgfound); 
}
int getBikesSold() { // LCS only
  return getInt(global_bikessold);
}
short getBalloonsBurst() { // VCS only
  return getShort(global_balloonsburst + gp);
}