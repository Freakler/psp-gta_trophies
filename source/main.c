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

#include <pspkernel.h>
#include <pspctrl.h>
#include <systemctrl.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "save.h"
#include "utils.h"
#include "functions.h"


PSP_MODULE_INFO("GTATrophies", 0, 1, 0); // user

SceCtrlData pad;

int magic = 0xD0D0ADDE; // random magic for savefile

int alldone = 0;

const char *file_log = "ms0:log.txt";
const char *file_save = "gta_trophies.sav";

u32 mod_text_addr;
u32 mod_text_size;
u32 mod_data_size;

register int gp asm("gp");
static STMOD_HANDLER previous;

int PPSSPP = 0, LCS = 0, VCS = 0;

char save[128], *ptr;
char buffer[256]; // string buffer

///////////////////////////////////////////////////////////////////////////////

int pplayer = 0;
int pcar = 0;
int pobj = 0;
short pcar_id = 0;
int gametimer = 0;
int island = 0;
int multiplayer = 0;

u32 global_gametimer        = -1;
u32 global_timescale        = -1;
u32 global_currentisland    = -1;
u32 global_systemlanguage   = -1;
u32 global_ismultiplayer    = -1;
u32 global_weather          = -1;
u32 global_carscrushed      = -1;
u32 global_ScriptSpace      = -1;
u32 global_empireowned      = -1;
u32 global_garagedata       = -1;
u32 var_garageslots         = -1;
u32 var_garageslotsize      = -1;
u32 global_usjdone          = -1;
u32 global_usjtotal         = -1;
u32 global_exportedVehicles = -1;
u32 global_exportVehicTotal = -1;
u32 global_hitmankilled     = -1;
u32 global_rampagesdone     = -1;
u32 global_rampagestotal    = -1;

u32 global_helpbox                = -1; // VCS only
u32 global_helpbox_string         = -1; // LCS only
u32 global_helpbox_timedisplayed  = -1; // LCS only
u32 global_helpbox_displaybool    = -1; // LCS only
u32 global_helpbox_permboxbool    = -1; // LCS only
u32 global_helpbox_duration       = -1; // LCS only
u32 global_dialog_string          = -1; // LCS only

///////////////////////////////////////////////////////////////////////////////

trophies_pack trophies[] = {
 
  // ID      // TITLE                                  // DESC
  {0x100, 0, "Maximum Power!",                         "Reach the top speed with a vehicle of your choice" }, 
  {0x101, 0, "To Hell and Back!",                      "?" }, // Fall through map into hell
  {0x102, 0, "You picked the wrong house fool!",       "Try walk into the wrong savehouse" }, // "Eddies Garage" LCS / "OceanView Hotel" VCS
  {0x103, 0, "Wanted!",                                "Reach 6 Wanted Stars level" }, 
  {0x104, 0, "Tank'jackin!",                           "Get into a Rhino" }, // get in a tank
  {0x105, 0, "No Escape!",                             "?" }, // reach the map border
  {0x106, 0, "Done it all!",                           "100% complete the game" }, //
  {0x107, 0, "Full house!",                            "Fill all Garages to the max" }, // Portland 1, Staunton 2, Shoreside 3 (12 max) & 101Baysh. 1, Compound 1, Clym. 3 (12 max)
  {0x108, 0, "Stuntman!",                              "Complete all Unique Stunt Jumps" }, // LCS 26 & VCS 30
  {0x109, 0, "Feed the Fishes!",                       "Drown" }, // not working when in car
  {0x10A, 0, "Grand Theft Auto!",                      "Find and deliver all wanted Vehicles" },
  {0x10B, 0, "Rampage!",                               "Complete all Rampages" },
  
  
  /// LCS only //////////////
  {0x200, 0, "Let it Snow!",                           "Find a way to make it snow in Liberty City" }, //by finish salesman
  {0x201, 0, "Hello again!",                           "Visit the famous smiley face sign" }, // you werent supposed to get here you know
  {0x202, 0, "Crusher!",                               "Crush a total of 20 vehicles at the junkyard" },
  {0x203, 0, "Killer by the dozen!",                   "Whack 12 of Ma's Hitmen before they do you" },
  {0x204, 0, "Sleeping with Angels!",                  "Pay JD a visit" },
  
  
  /// VCS only //////////////
  {0x300, 0, "There are no Easter Eggs up here!",      "?" }, // Jump against Easteregg Window from Helipad
  {0x301, 0, "Ah shit, here we go again...",           "Drive a BMX while wearing the trailer trash outfit" },
  {0x302, 0, "In the skies!",                          "Fly higher than the game's limit" }, // 
  {0x303, 0, "Fun Ride!",                              "Use the Ferris wheel" }, // 
  {0x304, 0, "Empire much!",                           "Acquire all Business sites" }, // 

  
}; int trophies_size = (sizeof(trophies)/sizeof(trophies_pack));

///////////////////////////////////////////////////////////////////////////////

int sceKernelSysClock2USecWidePatched(SceInt64 clock, unsigned *low, unsigned int *high) { // LCS & VCS

  #ifdef LOG
  static int debug_skgstwp = 1;
  static int debug_skgstwp2 = 1;
  if( debug_skgstwp2 && debug_skgstwp == 0 ) {
    logPrintf("[INFO] %i: sceKernelSysClock2USecWidePatched() ran the SECOND time", getGametime());
    debug_skgstwp2 = 0;
  }  
  if( debug_skgstwp ) {
    logPrintf("[INFO] %i: sceKernelSysClock2USecWidePatched() ran the FIRST time", getGametime());
    debug_skgstwp = 0;
  }
  #endif
  
  ////////////////////////////////////
  
  /// pplayer
  pplayer = GetPPLAYER();
    
  /// pcar & more
  pcar = GetPCAR();
  pcar_id = getVehicleID(pcar);
  
  /// pobj 
  pobj = (pcar == 0) ? pplayer : pcar;
  
  /// gametimer
  gametimer = getGametime();

  /// island
  island = getIsland();

  /// is multiplayer active
  multiplayer = getMultiplayer();
  
  
  /// check for and trigger trophies
  if( gametimer >= 4000 && !multiplayer ) { // wait ~2 seconds before checking
    if( trophies_getDoneCurrentGame() != trophies_getTotalCurrentGame() ) {
      trophy();
    } else {
      congrats();
    }
  }
  
  ////////////////////////////////////
  
  return sceKernelSysClock2USecWide(clock, low, high);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Unused for now
void trophies_reset() {
  int i;
  for( i = 0; i < trophies_size; i++ ) { // loop trophies list
    trophies[i].unlocked = 0; // lock
    //trophies[i].value = 0; // zero value
  }
  setTimedTextbox("Trophies have been reset!", 7.00f);
}

int trophies_getTotal() {
  return trophies_size;  
}
int trophies_getTotalCurrentGame() {
  int i, counter = 0;
  for( i = 0; i < trophies_size; i++ ) { // loop trophies list
    if( LCS && (trophies[i].id >> 8 != 0x3) ) {
      counter++;
    }
    if( VCS && (trophies[i].id >> 8 != 0x2) ) {
      counter++;
    }
  }
  return counter;
}
int trophies_getDoneTotal() {
  int i, counter = 0;
  for( i = 0; i < trophies_size; i++ ) // loop trophies list
    if( trophies[i].unlocked )
      counter++;
  return counter;
}
int trophies_getDoneCurrentGame() {
  int i, counter = 0;
  for( i = 0; i < trophies_size; i++ ) { // loop trophies list
    if( trophies[i].unlocked ) {
      if( LCS && (trophies[i].id >> 8 != 0x3) ) {
        counter++;
      }
      if( VCS && (trophies[i].id >> 8 != 0x2) ) {
        counter++;
      }
    }
  }
  return counter;
}


void congrats() {
   
  if( alldone ) // show congrats message only once
    return;

  if( isTextboxShowing() ) // don't show message when in menu or a textbox is there already!
    return;

  char buffer[128];
  float duration = 7.0f; // duration the textbox should be displayed
  snprintf(buffer, sizeof(buffer), "~h~ALL %d TROPHIES EARNED!~w~", trophies_getTotalCurrentGame());
  setTimedTextbox(buffer, duration);
  alldone = 1;
}

void trophy() {
  int i;
  
  float pobj_x = getFloat(pobj+0x30);
  float pobj_y = getFloat(pobj+0x34);
  float pobj_z = getFloat(pobj+0x38);
  
  /// CHECK ////////////////////////////////////////////////////////////////////////
  for( i = 0; i < trophies_size; i++ ) { // loop trophies list
    #ifdef DEBUG
    trophies[i].unlocked = 2;
    #endif  
    if( !trophies[i].unlocked ) {
      switch( trophies[i].id ) {
        
        case 0x100: /// "Well, thats fast!"
          if( pcar && getVehicleSpeed(pcar) > 1.88f ) { // 1.00f = 50 kmh, 2.00 = 100 kmh, 6.00 = 300 kmh
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x101: /// "To Hell and Back!"
          if( (pobj_z <= ((LCS) ? -80.0f : -237.0f)) && (getFloat(pplayer+(LCS?0x78:0x148)) <= -0.05f ) ) { // below limit and falling check
            trophies[i].unlocked = 2; // unlocked
          } break;
          
        case 0x102: /// "You picked the wrong house fool!" 
          if( !pcar && island == 1 && checkCoordinateInsideArea(pobj_x, pobj_y, pobj_z, (LCS ? 889.70f : 228.70f), (LCS ? -308.40f : -1277.90f), (LCS ? 8.70f : 12.00f), 2.0f) ) { // Eddies Garage LCS or OceanView Hotel VCS
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x103: /// "Wanted!" 
          if( getWantedLevel(pplayer) == 6 ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x104: /// "Tankjackin!" 
          if( pcar && (getVehicleID(pcar) == (LCS ? 0xA2 : 0xF6)) ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x105: /// "No Escape" 
          if( pobj_x < (LCS ? -1948.0f : -2080.0f) || pobj_x > (LCS ? 1948.0f : 1530.0f) || pobj_y < (LCS ? -1948.0f : -1870.0f) || pobj_y > (LCS ? 1948.0f : 1520.0f) ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
       
        case 0x106: /// "Done it all" 
          if( GetPercentageGameComplete() == 100.0f ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x107: /// "Full House" 
          if( getGarageVehicleActiveObjects(global_garagedata) >= (LCS ? 6 : 5) ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x108: /// "Stuntman" 
          if( getUsjTotal() > 0 && getUsjDone() == getUsjTotal() ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x109: /// "Drown" 
          if( getPedDrowning(pplayer) && getPedHealth(pplayer) == 0.0f ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x10A: /// "Grand Theft Auto" 
          if( getInt(global_exportedVehicles + (LCS ? 0 : gp)) > 0 && (getInt(global_exportedVehicles + (LCS ? 0 : gp)) == getInt(global_exportVehicTotal + (LCS ? 0 : gp))) ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
          
        case 0x10B: /// "Rampage" 
          if( getRampagesDone() > 0 && (getRampagesDone() == getRampagesTotal()) ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
          
        
        /// LCS only //////////////
    
        case 0x200: /// "Let it Snow!"
          if( LCS && getWeather() == 0x7 ) { // Snow
            trophies[i].unlocked = 2; // unlocked
          } break;
          
        case 0x201: /// "Hello again!"
          if( LCS && island == 2 && checkCoordinateInsideArea(pobj_x, pobj_y, pobj_z, -43.0f, -1536.0f, 26.0f, 3.0f) ) {
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x202: /// "Crusher"
          if( LCS && getCarsCrushed() >= 20 ) { // 20 vehicles
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x203: /// "Killer by the dozen"
          if( LCS && getKilledHitmen() >= 12 ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
      
        case 0x204: /// "Sleeping with Angels"
          if( LCS && island == 2 && checkCoordinateInsideArea(pobj_x, pobj_y, pobj_z, -74.0f, -1166.0f, 26.0f, 2.0f) ) { 
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        
        /// VCS only //////////////
        
        case 0x300: /// "There are no Easter Eggs up here. Go away"
          if( VCS && !pcar && island == 2 && checkCoordinateInsideArea(pobj_x, pobj_y, pobj_z, -483.0f, 1133.0f, 65.5f, 2.0f) ) { // 
            trophies[i].unlocked = 2; // unlocked
          } break;
          
        case 0x301: /// "Ah shit, here we go again."
          if( VCS && ((pcar_id == 0xB2) ) && getPedID(pplayer) == 0x9D ) { // "plr4"
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x302: /// "Above the limit."
          if( VCS && pcar && pobj_z >= 100.0f ) { // countermeasures above 80. Heli max is like 88, Plane Max is 100
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x303: /// "Fun Ride"
          if( VCS && !pcar && island == 1 && checkCoordinateInsideArea(pobj_x, pobj_y, pobj_z, 342.0f, -242.0f, 57.0f, 2.0f) ) {
            trophies[i].unlocked = 2; // unlocked
          } break;
        
        case 0x304: /// "Empire much"
          if( VCS && getNumberEmpiresOwned() == 30 ) {
            trophies[i].unlocked = 2; // unlocked
          } break;
        
      
      }
    }
  }
  
  /// DISPLAY //////////////////////////////////////////////////////////////////////
  
  if( isTextboxShowing() ) // don't show trophies when in menu or a textbox is there already!
    return;
  
  char buffer[128];
  float duration = 7.00f; // duration the textbox should be displayed
  
  for(i = 0; i < trophies_size; i++) { // loop trophies list
    if( trophies[i].unlocked == 2 ) { // check for "just unlocked"

      if (VCS) // LCS crashes if you join ~n~ and ~h~
        snprintf(buffer, sizeof(buffer), "You have earned a trophy! ~n~~h~%s~w~", trophies[i].title);
      else
        snprintf(buffer, sizeof(buffer), "You have earned a trophy! ~n~ ~h~%s", trophies[i].title);

      trophies[i].unlocked = 1; // set "was displayed"
      goto triggertrophies; // trigger it then!
    }
  }
  
  return;
  
  static u8 script_trophies_lcs[] = {
    /// 0399: play_mission_passed_tune 1 
    0x99, 0x03, // opcode 0399
    0x07, // byte
    0x01, // 1

    /// 010E: add_score $PLAYER_CHAR value 50000 // money 
    0x0E, 0x01, // opcode 010E
    0xCE, 0x18, // $PLAYER_CHAR
    0x06, // int
    0x50, 0xC3, 0x00, 0x00, // $50.000
    
    /// 004E: terminate_this_script 
    0x4E, 0x00 // opcode 004E
  };
  
  static u8 script_trophies_vcs[] = {
    /// 022B: play_mission_passed_tune 1 
    0x2B, 0x02, // opcode 022B
    0x07, // byte
    0x01, // 1

    /// 0094: add_score $PLAYER_CHAR value 50000 // money 
    0x94, 0x00, // opcode 0094
    0xD0, 0x0E, // $PLAYER_CHAR
    0x06, // int
    0x50, 0xC3, 0x00, 0x00, // $50.000 
    
    /// 0023: terminate_this_script 
    0x23, 0x00 // opcode 0023
  };
  
  triggertrophies:
    CustomScriptExecute(LCS ? (int)&script_trophies_lcs : (int)&script_trophies_vcs); // make game execute it
    setTimedTextbox(buffer, duration);
    
    //ushort *message = L"This is a Test!";
    //SetHelpMessage(message, TRUE, FALSE);
    
    /// save current state / progress to file 
    saveprogress(trophies, trophies_size);
    
}

int adjustColorLCS(u32 color, float coord) { /// alpha calc could be better and dynamic - ok for now
  if( coord < 60.0f ) color -= 0x99000000; // alpha text top bound
  if( coord < 55.0f ) color = 0x00000000; // disable text top bound
  if( coord > 200.0f ) color -= 0x99000000; // alpha text bottom bound
  if( coord > 206.0f ) color = 0x00000000; // disable text bottom bound
  return color;
}

int adjustColorVCS(u32 color, float coord) { /// alpha calc could be better and dynamic - ok for now
  if( coord < 70.0f ) color -= 0x99000000; // alpha text top bound
  if( coord < 65.0f ) color = 0x00000000; // disable text top bound
  if( coord > 200.0f ) color -= 0x99000000; // alpha text bottom bound
  if( coord > 206.0f ) color = 0x00000000; // disable text bottom bound
  return color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 addr_scrollval = 0;
wchar_t trophystr[16] = L"Trophies";

int (*CTextGet)(int **ctext, char *string);
int CTextGet_patched(int **ctext, char *string) {
  if( strcmp(string, "FEH_BRI") == 0 ) // replace "FEH_BRI" aka "BRIEF" with custom text
    return (int)&trophystr;
  return CTextGet(ctext, string);
}

void (*DrawBrief)(int param_1); 
void DrawBrief_patched(int param_1) {
  
  // VCS! logPrintf("0x%08X", param_1);
  // 0x0B98F400 briefs  -> Brief_Scroll_Titles
  // 0x0B98F500 stats  -> Stats_Scroll_Titles
  // 0x0B98F600 stats  -> Stats_Scroll_val
  if( VCS && getInt(getInt(param_1)) != 0x65697242 ) // check if is "Brief_Scroll_Titles" (beacause for VCS this func also draws Stats!)
    return DrawBrief(param_1); // draw vcs stats, maybe more
  
  memset(&pad, 0, sizeof(SceCtrlData));
  sceCtrlPeekBufferPositive(&pad, 1);
  u32 buttons = pad.Buttons;
  static u32 old_buttons = 0;
  float ystick = (float)(pad.Ly - 128) / (float)128;
  
  static int mode = 1; // 1 / ON / Show all
  u32 color;
  wchar_t str[128] = L" ";
  char string[128];
  
  static float position = 0.0f;
  if( LCS ) {
  SetWrapx(SCREEN_WIDTH - 13.0f);
  SetRightJustifyWrap(13.0);
  }
  if( VCS ) {
  float coords[4];
  coords[0] = 20.0f; // left
  coords[1] = SCREEN_HEIGHT-0.0f; // bottom
  coords[2] = SCREEN_WIDTH-14.0f; //right
  coords[3] = 0.0f; // top
  SetTextBounds(coords);
  }
  
  /// Header
  snprintf(string, sizeof(string), "PROGRESS: %.0f%%", ((float)trophies_getDoneCurrentGame() / (float)trophies_getTotalCurrentGame()) * 100.0f);
  AsciiToUnicode(string, str);
  if( LCS ) {
    SetFontStyle(1);
    SetPropOn();
    SetDropShadowPosition(0);
    SetScale_LCS(0.4048,0.88);
    color = 0xFFdcb076; // LIGHT BLUEish
    SetColor(&color);
    PrintString_LCS(15.0, 40.0, str, 0);
  } 
  if( VCS ) {
    SetFontStyle(1);
    SetTextOriginPoint(3); // left
    color = 0xFFE3EC12; // Azure (not 100% correct)
    SetColor(&color);
    SetScale_VCS(0.6f);
    PrintString_VCS(str, 0.0f, 50.0f);
  }
  
  snprintf(string, sizeof(string), "%d/%d UNLOCKED", trophies_getDoneCurrentGame(), trophies_getTotalCurrentGame());
  AsciiToUnicode(string, str);
  if( LCS ) {
    SetRightJustifyOn();
    PrintString_LCS(SCREEN_WIDTH-13.0f, 40.0f, str, 0);
  }
  if( VCS ) {
    SetTextOriginPoint(4); // right
    PrintString_VCS(str, SCREEN_WIDTH, 50.0f);
  }
  
  /// DEBUG
  /*sprintf(string, "pos: %.02f, mode: %d, addr_scrollval: %.02f, ystick: %.02f", position, mode, getFloat(addr_scrollval), ystick);
  AsciiToUnicode(string, str);
  color = 0xFF0000FF; //red
  SetColor(&color);
  if( LCS ) PrintString_LCS(480.0 -13.0, 20.0, str, 0);
  if( VCS ) PrintString_VCS(str, SCREEN_WIDTH, 20.0);*/
  
  
  /// Trophy Texts
  int i;
  float cur = 0.0f;
  for( i = 0; i < trophies_size; i++ ) {
    
    if( mode == 0 && trophies[i].unlocked > 0 )
      continue;
      
    if( LCS && (trophies[i].id >> 8 != 0x3) ) { // both and LCS-only trophies
      
      SetRightJustifyOff();
      SetDropShadowPosition(0);
      
      /// draw title
      SetFontStyle(0); // 2
      SetScale_LCS(0.36432,0.792);
      color = (trophies[i].unlocked > 0) ? 0xFF00FF00 : 0xFFFFFFFF;  //GREEN else WHITE
      color = adjustColorLCS(color, 65.0f + position + cur);
      SetColor(&color);
      sprintf(string, "%s", trophies[i].title);
      AsciiToUnicode(string, str);
      PrintString_LCS(15.0f, 65.0f + position + cur, str, 0);
      
      /// draw dec
      SetFontStyle(1);
      color = (trophies[i].unlocked > 0) ? 0xFF00FF00 : 0xFFFFFFFF;  //GREEN else WHITE
      color = adjustColorLCS(color, 80.0f + position + cur); // WHITE
      SetColor(&color);
      sprintf(string, "%s", trophies[i].desc);
      AsciiToUnicode(string, str);
      PrintString_LCS(15.0f, 80.0f + position + cur, str, 0);
      
      cur+=42.0f;
    }
    
    if( VCS && (trophies[i].id >> 8 != 0x2) ) { // both and VCS-only trophies
      
      SetTextOriginPoint(3); // left
        
      /// draw Title
      SetFontStyle(2); //SetFontStyle(0);
      SetScale_VCS(0.35f); //SetScale_VCS(0.7f);
      color = (trophies[i].unlocked > 0) ? 0xFF00FF00 : 0xFFEEEEEE;  //GREEN else WHITE (grayish)
      color = adjustColorVCS(color, 75.0f + position + cur);
      SetColor(&color);
      sprintf(string, "%s", trophies[i].title);
      AsciiToUnicode(string, str);
      PrintString_VCS(str, 0.0f, 75.0f + position + cur);
    
      /// draw dec
      SetFontStyle(1);
      SetScale_VCS(0.5f);
      color = (trophies[i].unlocked > 0) ? 0xFF00FF00 : 0xFFFFFFFF;  //GREEN else WHITE
      color = adjustColorVCS(color, 92.0f + position + cur); // WHITE
      SetColor(&color);
      sprintf(string, "%s", trophies[i].desc);
      AsciiToUnicode(string, str);
      PrintString_VCS(str, 0.0f, 92.0f + position + cur);
    
      cur+=42.0f;
    }
  }
  
  /// Controls
  if( (getFloat(addr_scrollval) != 0.0f) || VCS ) { // if this float is set, the user is pressing a button (LCS!)
    if( pad.Buttons & PSP_CTRL_DOWN ) {
      if( position < 0.00f )
        position += 3.0f;
    }
  
    if( pad.Buttons & PSP_CTRL_UP ) {
      if( -position < cur-145.0f ) // cur = trophies * 42.0f (162 is the screen space for display)
        position -= 3.0f;
    }
  
    if( old_buttons != buttons && pad.Buttons & PSP_CTRL_CROSS ) {
      mode = 1 - mode;
      position = 0.0f; // reset scroll
    }
  
  position += ystick * 8.00f; // scroll with stick
  if( position > 0.0f ) position = 0.0f;
  if( -position > cur-145.0f ) position = (cur - 145.0f) * -1;
  
  }
  old_buttons = buttons;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int patchonce = 1; 

int PatchLCS(u32 addr, u32 text_addr) { //Liberty City Stories
  
  if( LCS && patchonce ) { //FOR TESTING ONLY
    
    SetHelpMessage = (void*)(0x182ae0);
    IsHelpMessageBeingDisplayed = (void*)(0x0182ca0);

    ///////////////
  
    //setByte(text_addr + 0x02df3d0, 0x01); // pause menu bottom menu fontstyle
    //MAKE_DUMMY_FUNCTION(text_addr + 0x02d510c, 0); // disable draw pause bottom menu 
    //MAKE_DUMMY_FUNCTION(text_addr + 0x002d8ad4, 0); // disable all Pause Menu Controls

    //HIJACK_FUNCTION(text_addr + 0x2e090c, DrawBrief_patched, DrawBrief); //replace FUN_002e090c_PauseMenu_DrawBrief
    //HIJACK_FUNCTION(text_addr + 0x002df66c, DrawStats_patched, DrawStats); //replace FUN_002df66c_PauseMenu_DrawStats
  
    //HIJACK_FUNCTION(text_addr + 0x002d510c, DrawMenu_patched, DrawMenu); //FUN_002d510c_drawMenuBottom
  
  
    //HIJACK_FUNCTION(text_addr + 0x10fad4, CTextGet_patched, CTextGet);
    //addr_scrollval = text_addr + 0x0355c3c; // 
    //addr_b = text_addr + 0x0355c3c; // 
    //addr_c = text_addr + 0x0355c40; // 
  

  //MAKE_DUMMY_FUNCTION(text_addr + 0x002f5b8c, 0); //
  //MAKE_DUMMY_FUNCTION(text_addr + 0x002247ac, 0); //
  
  
    patchonce = 0;
  }
  
/////////////////////////////////////
/////////////////////////////////////

  if( _lw(addr + 0xC) == 0x3C0443E9 && _lw(addr + 0x58) == 0x3C044150 ) { // sub_002E090C
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> DrawBrief", addr-text_addr, addr);
    #endif
    HIJACK_FUNCTION(addr, DrawBrief_patched, DrawBrief);
    return 1;
  }
  if( PPSSPP && _lw(addr - 0x1C) == 0x28850008 && _lw(addr + 0x38) == 0x92240021 ) { // sub_0010fad4
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> CTextGet", addr-text_addr, addr);
    #endif
    HIJACK_FUNCTION(addr, CTextGet_patched, CTextGet);
    return 1;
  }
  if( _lw(addr + 0x18) == 0x3C054316 && _lw(addr - 0x18) == 0x92040131 ) { // 0x002daa5c
    /**************************************************************
    0x002DAA5C: 0x3C040035 '5..<' - lui        $a0, 0x35
    0x002DAA60: 0xE48C5C3C '<\..' - swc1       $fpr12, 23612($a0)
    **************************************************************/
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> addr_scrollval", addr-text_addr, addr); 
    #endif
    addr_scrollval = (_lh(addr+0x0) * 0x10000) + (int16_t)_lh(addr+0x4); // DAT_00355c3c
    return 1;
  }
  
/////////////////////////////////////
 
  if( _lw(addr - 0x6C) == 0x0211A02B && _lw(addr - 0xD8) == 0x0211A02B ) { // 0x00252AD4
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> PrintString_LCS", addr-text_addr, addr);
    #endif
    PrintString_LCS = (void*)(addr);
    return 1;
  }
  if( _lw(addr - 0x4) == 0xA080001F && _lw(addr + 0x8) == 0xE48C0004 && _lw(addr + 0x10) == 0xE48D0008 && _lw(addr + 0x20) == 0xE48C000C ) { // 0x00250DF8
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetScale_LCS", addr-text_addr, addr);
    #endif
    SetScale_LCS = (void*)(addr);
    
    SetSlant = (void*)(addr+0x14); // FUN_00250e0c_SetSlant
    SetSlantRefPoint = (void*)(addr+0x14); // FUN_00250e1c_SetSlantRefPoint
    
    return 1;
  }
  if( _lw(addr - 0x4) == 0xE48D0014 && _lw(addr) == 0x00802825 && _lw(addr + 0x4) == 0x90A40000 && _lw(addr + 0x10) == 0x90A70001 ) { // 0x00250E30
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetColor", addr-text_addr, addr);
    #endif
    SetColor = (void*)(addr);
    return 1;
  }
  if( _lw(addr + 0xC) == 0xA0800018 && _lw(addr + 0x10) == 0x03E00008 && _lw(addr + 0x14) == 0xA0800019 && _lw(addr + 0x18) == 0x44807000 && _lw(addr + 0x1C) == 0x3C0443F0 ) { // 0x00250F14
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetJustify(s)", addr-text_addr, addr);
    #endif
    SetRightJustifyOff   = (void*)(addr);
    SetRightJustifyOn   = (void*)(addr - 0x1C); // FUN_00250ef8_
    SetCentreOff     = (void*)(addr - 0x2C); // FUN_00250ee8_
    SetCentreOn     = (void*)(addr - 0x48); // FUN_00250ecc_
    SetJustifyOff     = (void*)(addr - 0x5C); // FUN_00250eb8_
    SetJustifyOn     = (void*)(addr - 0x78); // FUN_00250e9c_
    return 1;
  }
  if( _lw(addr - 0x4) == 0xA080001C && _lw(addr + 0x4) == 0x34040001 && _lw(addr + 0xC) == 0x03E00008 && _lw(addr + 0x10) == 0xA0A4001D ) { // 0x00251030
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetPropOn", addr-text_addr, addr);
    #endif
    SetPropOn = (void*)(addr);
    SetPropOff = (void*)(addr+14);
    return 1;
  }
  if( _lw(addr) == 0x00042C00 && _lw(addr + 0xc) == 0x34060002 && _lw(addr + 0x10) == 0x14A60005 && _lw(addr + 0x18) == 0xA4850038 ) { // 0x00251054
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetFontStyle", addr-text_addr, addr);
    #endif
    SetFontStyle = (void*)(addr);
    return 1;
  }
  if( _lw(addr - 0x3C) == 0x34060002 && _lw(addr + 0x10) == 0x00802825 &&  _lw(addr + 0xB4) == 0x00000000 ) { // 0x0025109c
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetDropShadowPosition & SetDropColor", addr-text_addr, addr);
    #endif
    SetDropShadowPosition = (void*)(addr);
    SetDropColor = (void*)(addr + 0x10);
    return 1;
  }
  if( _lw(addr+0x4) == 0x3C0443F0 && _lw(addr + 0x44) == 0x3C0443F0 && _lw(addr + 0x68) == 0x00000000 ) {  
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetRightJustifyWrap", addr-text_addr, addr);
    logPrintf("blitn: 0x%08X (0x%08X) -> SetWrapx", addr+0x40-text_addr, addr+0x40);
    #endif
    SetRightJustifyWrap = (void*)(addr); // 0x00250F2C
    SetWrapx = (void*)(addr+0x40); // 0x00250f6c
    
    SetCentreSize = (void*)(addr+0x80); // FUN_00250fac_SetCentreSize
    SetBackgroundOn = (void*)(addr+0x90); // FUN_00250fbc_SetBackgroundOn
    SetBackgroundOff = (void*)(addr+0xA4); // FUN_00250fd0_SetBackgroundOff
    SetBackgroundColor = (void*)(addr+0xB4); // FUN_00250fe0_SetBackgroundColor
    SetBackGroundOnlyTextOn = (void*)(addr+0xE0); // FUN_0025100c_SetBackGroundOnlyTextOn
    SetBackGroundOnlyTextOff = (void*)(addr+0xF4); // FUN_00251020_SetBackGroundOnlyTextOff
    
    return 1;
  }
  
/////////////////////////////////////
/////////////////////////////////////
  
  /// sceKernelSysClock2USecWide
  if( _lw(addr - 0x70) == 0x3C043586  && _lw(addr + 0x18) == 0x00402025  && _lw(addr + 0x64) == 0x34040001 ) {  //LCS US 3.00 -> 0x002AF398
    /*******************************************************************
     *  002af3ac bb 1e 0c 0c     jal        sceKernelSysClock2USecWide   
    *******************************************************************/
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) --> sceKernelSysClock2USecWide()", addr+0x14-text_addr, addr+0x14);
    #endif
    MAKE_CALL(addr+0x14, sceKernelSysClock2USecWidePatched); //LCS US 3.00 -> 0x002af3ac
    return 1;
  }
  
  /// StartNewScript
  if( _lw(addr - 0xC) == 0x2402FFFF  && _lw(addr + 0x30) == 0x24C70001 && _lw(addr + 0x58) == 0x34040001 ) { // FUN_015415c
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> StartNewScript()", addr-text_addr, addr);
    #endif
    StartNewScript = (void*)(addr); // 0x15415c
    return 1;
  }
  /// ScriptSpace
  if(  _lw(addr + 0x2C) == 0xA224020E && _lw(addr + 0x28) == 0x34040001 ) { // 0x000E0FA0
    /*******************************************************************
     *  0x000E0FA0: 0x3C100033 '3..<' - lui        $s0, 0x33
     *  0x000E0FA4: 0x8E054D7C '|M..' - lw         $a1, 19836($s0)
    *******************************************************************/
    global_ScriptSpace = (_lh(addr) * 0x10000) + (int16_t)_lh(addr+0x4); 
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_ScriptSpace", global_ScriptSpace-text_addr, global_ScriptSpace); // DAT_00334d7c_ScriptSpace
    #endif
    return 1;
  }
  
  
  /// get pplayer function
  if( _lw(addr + 0x8) == 0x000429C0 && _lw(addr + 0xC) == 0x00A53021 && _lw(addr + 0x18) == 0x00A42023 && _lw(addr + 0x2C) == 0x8C820000 ) {  //0x1d18b0
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) --> GetPPLAYER()", addr-text_addr, addr);
    #endif
    GetPPLAYER = (void*)(addr); //get pplayer 
    return 1;
  }
  
  
  /// get pobj function 
  if( _lw(addr - 0x18) == 0x00000000 && _lw(addr + 0x8) == 0x000429C0 && _lw(addr + 0x24) == 0x00852021 && _lw(addr + 0x2C) == 0x10800006  ) {  //0x1d17b4
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) --> GetPCAR()", addr-text_addr, addr);
    #endif
    GetPCAR = (void*)(addr); // get pcar 
    return 1;
  }
  
  
  /// global gametimer
  if( _lw(addr - 0xC) == 0x02002825 &&  _lw(addr - 0x4) == 0x00001025 && _lw(addr + 0x14) == 0x02002025 ) { 
    /*******************************************************************
     *  0x001DB5EC: 0x3C040036 '6..<' - lui        $a0, 0x36
     *  0x001DB5F0: 0x8C84A144 'D...' - lw         $a0, -24252($a0)
    *******************************************************************/
    global_gametimer = (_lh(addr) * 0x10000) + (int16_t)_lh(addr+0x4); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) --> global_gametimer", global_gametimer-text_addr, global_gametimer); // 0x35A144
    #endif        
    return 1;
  }
  
  
  /// global_currentisland & global_systemlanguage [this function sets lots of globals -> weather, clock, cheatsused, gamespeed]
  if( _lw(addr + 0x20) == 0xAE040008  && _lw(addr + 0x18C) == 0xAE040070 ) {     
    /*******************************************************************
     *  0x000ACA2C: 0x3C040036 '6..<' - lui        $a0, 0x36
     *  0x000ACA30: 0x8C84A46C 'l...' - lw         $a0, -23444($a0)
    *******************************************************************/
    global_currentisland = (_lh(addr+0x14) * 0x10000) + (int16_t)_lh(addr+0x18); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) --> global_currentisland", global_currentisland-text_addr, global_currentisland); // DAT_0035A46C
    #endif
    
    /*******************************************************************
     *  0x000ACA40: 0x3C050035 '5..<' - lui        $a1, 0x35
     *  0x000ACA48: 0x8CA45C9C '.\..' - lw         $a0, 23708($a1)
    *******************************************************************/
    global_systemlanguage = (_lh(addr+0x28) * 0x10000) + (int16_t)_lh(addr+0x30); //actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) --> global_systemlanguage", global_systemlanguage-text_addr, global_systemlanguage); // DAT_00355c9c
    #endif
    return 1;
  }
  
  
  /// globals for helpbox (via "FUN_00182e94_ClearSmallPrints")
  if( _lw(addr - 0x10) == 0x14C0FFF2 && _lw(addr + 0x5C) == 0x28A90100 ) { // US 3.00 -> 0x00182E94 / ULUX -> 0x001599BC
    /*******************************************************************
     *  0x00182EB4: 0x3C080065 'e..<' - lui        $t0, 0x65
     *  0x00182EC8: 0x25089DC0 '...%' - addiu      $t0, $t0, -25152
    *******************************************************************/
    global_helpbox_string = (_lh(addr+0x20) * 0x10000) + (int16_t)_lh(addr+0x34); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_helpbox_string", global_helpbox_string-text_addr, global_helpbox_string); // DAT_00649dc0
    #endif
    /*******************************************************************
     *  0x00182EFC: 0x3C050036 '6..<' - lui        $a1, 0x36
     *  0x00182F00: 0xACA0A808 '....' - sw         $zr, -22520($a1)
    *******************************************************************/
    global_helpbox_timedisplayed = (_lh(addr+0x68) * 0x10000) + (int16_t)_lh(addr+0x6C); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_helpbox_timedisplayed", global_helpbox_timedisplayed-text_addr, global_helpbox_timedisplayed); // DAT_0035a808
    #endif
    /*******************************************************************
     *  0x00182F0C: 0x3C050036 '6..<' - lui        $a1, 0x36
     *  0x00182F10: 0xACA0A810 '....' - sw         $zr, -22512($a1)
    *******************************************************************/
    global_helpbox_displaybool = (_lh(addr+0x78) * 0x10000) + (int16_t)_lh(addr+0x7C); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_helpbox_displaybool", global_helpbox_displaybool-text_addr, global_helpbox_displaybool); // DAT_0035a810
    #endif
    /*******************************************************************
     *  0x00182F1C: 0x3C050036 '6..<' - lui        $a1, 0x36
     *  0x00182F20: 0xA0A0A815 '....' - sb         $zr, -22507($a1)
    *******************************************************************/
    global_helpbox_permboxbool = (_lh(addr+0x88) * 0x10000) + (int16_t)_lh(addr+0x8C); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_helpbox_permboxbool", global_helpbox_permboxbool-text_addr, global_helpbox_permboxbool); // DAT_0035a815
    #endif
    /*******************************************************************
     *  0x00182F5C: 0x3C060036 '6..<' - lui        $a2, 0x36
     *  0x00182F64: 0xE4CCA818 '....' - swc1       $fpr12, -22504($a2)
    *******************************************************************/
    global_helpbox_duration = (_lh(addr+0xC8) * 0x10000) + (int16_t)_lh(addr+0xD0); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_helpbox_duration", global_helpbox_duration-text_addr, global_helpbox_duration); // DAT_0035a818
    #endif
    
    
    /** BONUS *****************************************************************
     *  0x00182EC0: 0x3C040065 'e..<' - lui        $a0, 0x65
     *  0x00182ED4: 0x24849BC0 '...$' - addiu      $a0, $a0, -25664
    *******************************************************************/
    global_dialog_string = (_lh(addr+0x2C) * 0x10000) + (int16_t)_lh(addr+0x40); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_dialog_string", global_dialog_string-text_addr, global_dialog_string); // DAT_00649bc0
    #endif
    return 1;
  }
  
  
  /// weather
  if( _lw(addr + 0x1C) == 0x2404FFFF  && _lw(addr - 0x14) == 0x03E00008 && _lw(addr - 0x8) == 0x03E00008 && _lw(addr+0x14) == 0x03E00008 ) {
    global_weather = (_lh(addr+0x8) * 0x10000) + (int16_t)_lh(addr + 0xC); // actual address!  (only to READ current weather)
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_weather", global_weather-text_addr, global_weather); // 0x0035A484
    #endif
    return 1;
  }
  
  
  /// cars crushed
  if( _lw(addr + 0x4) == 0x34060001 && _lw(addr + 0x28) == 0x8E04009C ) { // @ 0x0013C45C
    global_carscrushed = (_lh(addr+0x2C) * 0x10000) + (int16_t)_lh(addr + 0x30); 
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_carscrushed", global_carscrushed-text_addr, global_carscrushed); // DAT_0035a280_STAT_
    #endif
    return 1;
  }
  
  
  if( _lw(addr + 0xC) == 0x3C0442C8 && _lw(addr + 0x5C) == 0x24A50001 ) { // 0x00040ee8
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> GetPercentageGameComplete", addr-text_addr, addr);
    #endif
    GetPercentageGameComplete = (void*)(addr); // FUN_00040ee8
    return 1;
  }
  
  if( _lw(addr - 0x20) == 0x2404FFFF && _lw(addr + 0x30) == 0x29040004 ) { //
    
    /*******************************************************************
     *  0x001376B4: 0x3C050063 'c..<' - lui        $a1, 0x63
     *  0x001376C0: 0x24A54A88 '.J.$' - addiu      $a1, $a1, 19080
    *******************************************************************/
    global_garagedata = (_lh(addr) * 0x10000) + (int16_t)_lh(addr+0xC); // actual address!
    #ifdef LOG
    logPrintf("0x%08X: 0x%08X -> global_garagedata", addr-text_addr, global_garagedata-text_addr); // 0x634A88
    #endif
    
    /*******************************************************************
     *  0x001376D4: 0x28E9000C '...(' - slti       $t1, $a3, 12
     *  0x001376EC: 0x24C6002C ',..$' - addiu      $a2, $a2, 44
     *******************************************************************/
    var_garageslots = *(char*)(addr+0x20); // 0xC = 12 slots (4 in each garage)
    var_garageslotsize = *(char*)(addr+0x38); // 0x2C
    #ifdef LOG
    logPrintf("var_garageslots = 0x%08X", var_garageslots); //
    logPrintf("var_garageslotsize = 0x%08X", var_garageslotsize); //
    #endif
    
    return 1;
  }
  
  
  /// stuntjumps
  if( _lw(addr + 0xC0) == 0x34060008 &&_lw(addr + 0x18) == 0x26050018 && _lw(addr + 0x8) == 0x24A50001   ) { // 0x00220698
    /*******************************************************************
    * 0x00220698: 0x3C040036 '6..<' - lui        $a0, 0x36
    * 0x0022069C: 0x8C85A2B4 '....' - lw         $a1, -23884($a0)
    *******************************************************************/
    global_usjdone = (_lh(addr) * 0x10000) + (int16_t)_lh(addr + 0x4); // 
    global_usjtotal = global_usjdone + 0x4; // 
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_usjdone", global_usjdone-text_addr, global_usjdone); // DAT_0035a2b4_STAT_USJs_done
    logPrintf("0x%08X (0x%08X) -> global_usjtotal", global_usjtotal-text_addr, global_usjtotal); // DAT_0035a2b8_USJs_total
    #endif
    return 1;
  }
  
  /// exported vehicles
  if( _lw(addr - 0x18) == 0x26250018 &&_lw(addr + 0x54) == 0x34060002 && _lw(addr + 0x114) == 0x3406000A   ) { // 0x0005ED44
    /*******************************************************************
    * 0x0005ED44: 0x3C050036 '6..<' - lui        $a1, 0x36
    * 0x0005ED48: 0x8CA6A3CC '....' - lw         $a2, -23604($a1)
    *******************************************************************/
    global_exportedVehicles = (_lh(addr) * 0x10000) + (int16_t)_lh(addr + 0x4); // 
    global_exportVehicTotal = global_exportedVehicles + 0x4; // 
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_exportedVehicles", global_exportedVehicles-text_addr, global_exportedVehicles); // DAT_0035a3cc_STAT_ExportedVehicles
    logPrintf("0x%08X (0x%08X) -> global_exportVehicTotal", global_exportVehicTotal-text_addr, global_exportVehicTotal); // DAT_0035a3d0_STAT_ExportVehiclesTotal
    #endif
    return 1;
  }
  
  /// STAT hitmen killed
  if( _lw(addr - 0x80) == 0x2404FFF7 &&_lw(addr + 0xC) == 0x24A50001 ) { // 0x00179740
    /*******************************************************************
    * 0x00179740: 0x3C040036 '6..<' - lui        $a0, 0x36
    * 0x00179744: 0x8C85A3F4 '....' - lw         $a1, -23564($a0)
    *******************************************************************/
    global_hitmankilled = (_lh(addr) * 0x10000) + (int16_t)_lh(addr + 0x4); // 
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_hitmankilled", global_hitmankilled-text_addr, global_hitmankilled); // DAT_0035a3f4_STAT_HitmenKilled
    #endif
    return 1;
  }
  
  /// rampages
  if( _lw(addr - 0x48) == 0x3C0442C8 &&_lw(addr + 0x8) == 0x24A50001 ) { // FUN_00040f3c
    /*******************************************************************
    * 0x00040F3C: 0x3C040036 '6..<' - lui        $a0, 0x36
    * 0x00040F40: 0x8C85A2FC '....' - lw         $a1, -23812($a0)
    *******************************************************************/
    global_rampagesdone = (_lh(addr) * 0x10000) + (int16_t)_lh(addr + 0x4); // 
    global_rampagestotal = global_rampagesdone + 0xC; // 
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_rampagesdone", global_rampagesdone-text_addr, global_rampagesdone); // DAT_0035a2fc_STAT_RampagesDone
    logPrintf("0x%08X (0x%08X) -> global_rampagestotal", global_rampagestotal-text_addr, global_rampagestotal); // DAT_0035a308_STAT_RampagesTotal
    #endif
    return 1;
  }
  
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////


int PatchVCS(u32 addr, u32 text_addr) { // Vice City Stories
  
  if( VCS && patchonce ) { //FOR TESTING ONLY
  
  //logPrintf("patchonce");
  
  //HIJACK_FUNCTION(text_addr + 0x4ac30, DrawBrief_patched, DrawBrief); // correct vcs!
  //HIJACK_FUNCTION(text_addr + 0x1f2390, CTextGet_patched, CTextGet); // correct vcs!
  //addr_scrollval = text_addr + 0x; // 

  /// FUN_0004abcc_PauseMenu_DrawStatsBriefs
  /// FUN_0004ac30_printStatsBriefs_actualPrint
  //MAKE_DUMMY_FUNCTION(text_addr + 0x0004bba0, 0); // disable stats texts loading checking
  //MAKE_DUMMY_FUNCTION(text_addr + 0x0016e99c, 0); // disable actual PauseMenu Category texts
  //MAKE_DUMMY_FUNCTION(text_addr + 0x0004ac30, 0); // disable actual Stats/Briefs printing
  
  
  //MAKE_DUMMY_FUNCTION(text_addr + 0x00308660, 0); // test

  
    patchonce = 0;
  }
  
/////////////////////////////////////

 if( _lw(addr - 0xC) == 0xAC8500F4 && _lw(addr + 0x38) == 0x34110000 ) { // 0x4ac30
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> DrawBrief", addr-text_addr, addr);
    #endif
    HIJACK_FUNCTION(addr, DrawBrief_patched, DrawBrief);
    return 1;
  }
  if( PPSSPP && _lw(addr - 0x20) == 0x3084FFFF && _lw(addr + 0x14) == 0xA3A00000 ) { // sub_001F2390
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> CTextGet", addr-text_addr, addr);
    #endif
    HIJACK_FUNCTION(addr, CTextGet_patched, CTextGet);
    return 1;
  }

/////////////////////////////////////
/////////////////////////////////////

  if( _lw(addr + 0x30) == 0x00409825 && _lw(addr + 0x54) == 0x00404825 ) { 
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> PrintString_VCS", addr-text_addr, addr);
    #endif
    PrintString_VCS = (void*)(addr); // 0x00308138
    return 1;
  }
  if( _lw(addr - 0x4) == 0x27BD0010 && _lw(addr) == 0x27BDFFE0 && _lw(addr + 0x4) == 0xAFB00014 && _lw(addr + 0x8) == 0x341000FF ) { 
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> ResetFontStyling", addr-text_addr, addr);
    #endif
    ResetFontStyling = (void*)(addr); // 0x0030805C
    return 1;
  }
  if( _lw(addr) == 0x03E00008 && _lw(addr + 0x8) == 0x03E00008 && _lw(addr + 0x20) == 0x90840003 ) { 
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetFontStyle, SetColor, SetScale_VCS, ...", addr-text_addr, addr);
    #endif
    SetFontStyle = (void*)(addr); // 0x308498
    SetColor = (void*)(addr+0x10); // 0x3084A8
    SetTextBounds = (void*)(addr+0x3C); // 0x3084d4
    SetScale_VCS = (void*)(addr+0x88); // 0x308520
    SetTextSpaceing = (void*)(addr+0x130); // 0x3085c8
    SetTextOriginPoint = (void*)(addr+0x16C); // 0x308604
    return 1;
  }
  if( _lw(addr+0x20) == 0x00052880 && _lw(addr + 0x68) == 0x00052880 && _lw(addr + 0x6C) == 0x00852021 ) { // JP only: 0x000FCE10
    #ifdef LOG
    logPrintf("blitn: 0x%08X (0x%08X) -> SetTextSpaceing, SetTextOriginPoint (JP only)", addr-text_addr, addr);
    #endif
    SetTextSpaceing = (void*)(addr); // sub_000FCE10
    SetTextOriginPoint = (void*)(addr+0x48); // sub_000FCE58
    return 1;
  } 
 
  //////////////////////////////////////
  //////////////////////////////////////
  
  /// sceKernelSysClock2USecWide
  /*************************************
   * ULUS-10160 v1.03 | 
   * ULES-00502 v1.02 | 
   * ULES-00503 v1.02 | 
   * ULJM-05297 v1.01 |  
   * ULET-00417 v0.06 | 
   **************************************/ 
   if( _lw(addr - 0xC) == 0x1000FFF7 && _lw(addr + 0x4) == 0x00000000 && _lw(addr + 0x8) == 0x27A60010 && _lw(addr + 0x10) == 0x00602825 && _lw(addr + 0x1C) == 0x8FA40014 ) {
    #ifdef LOG
    logPrintf("[0] 0x%08X (0x%08X) --> sceKernelSysClock2USecWide()", addr+0x14-text_addr, addr+0x14);
    #endif
    MAKE_CALL(addr+0x14, sceKernelSysClock2USecWidePatched); // 0x002030E8
    return 1;
  }
    
    
  /// StartNewScript
  if( _lw(addr - 0xC) == 0x2402FFFF  && _lw(addr + 0x28) == 0x24A60001 && _lw(addr + 0x4C) == 0x34040001 ) { // FUN_0005F470
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> StartNewScript()", addr-text_addr, addr);
    #endif
    StartNewScript = (void*)(addr); // 0x0005F470
    return 1;
  }
  /// ScriptSpace
  if(  _lw(addr + 0x2C) == 0xA204020A && _lw(addr + 0x28) == 0x34040001 ) { // 0x002B8118
    /*******************************************************************
     *  0x002B8118: 0x8F858E24 '$...' - lw         $a1, -29148($gp)
    *******************************************************************/
    global_ScriptSpace = (int16_t)_lh(addr); // WITHOUT GP!!
    #ifdef LOG
    logPrintf("0x%08X -> global_ScriptSpace", global_ScriptSpace); // iGpffff8e24 aka ScriptSpace
    #endif
    return 1;
  }
  
  
  /// pplayer function
  /*************************************
   * ULUS-10160 v1.03 | 0x0015C424 | OK!
   * ULES-00502 v1.02 |  | 
   * ULES-00503 v1.02 |  | 
   * ULJM-05297 v1.01 |  | 
   * ULET-00417 v0.06 | 0x00153DD8 | OK!
   **************************************/ 
  if( _lw(addr + 0x8) == 0x00043200 && _lw(addr + 0x18) == 0x00C42021 && _lw(addr + 0x28) == 0x8C820000 ) {  // 0x15c424
    #ifdef LOG
    logPrintf("[1] 0x%08X (0x%08X) --> GetPPLAYER()", addr-text_addr, addr);
    #endif
    GetPPLAYER = (void*)(addr); // get pplayer 
    return 1;
  }
  
  
  /// pcar function
  /*************************************
   * ULUS-10160 v1.03 | 0x0015C2C8 | OK!
   * ULES-00502 v1.02 |  
   * ULES-00503 v1.02 |  
   * ULJM-05297 v1.01 |  
   * ULET-00417 v0.06 | 
   **************************************/ 
  if( _lw(addr + 0x1C) == 0x00042140 && _lw(addr + 0x8) == 0x00842821 && _lw(addr + 0x38) == 0x00000000 ) {  // 0x0015C2C8
    #ifdef LOG
    logPrintf("[1] 0x%08X (0x%08X) --> GetPCAR()", addr-text_addr, addr);
    #endif
    GetPCAR = (void*)(addr); // get pcar 
    return 1;
  }
  
  
  /// global gametimer
  /*************************************
   * ULUS-10160 v1.03 | 0x00026BB8 | OK!
   * ULES-00502 v1.02 |  | 
   * ULES-00503 v1.02 |  | 
   * ULJM-05297 v1.01 |  | 
   * ULET-00417 v0.06 | 0x0002F9AC | OK!
   **************************************/ 
  if( _lw(addr + 0xC) == 0x00A6202B && _lw(addr - 0x20) == 0x03E00008 && _lw(addr + 0x50) == 0x00000000 ) { 
    /*******************************************************************
     *  0x00026BB8: 0x8F851DEC '....' - lw         $a1, 7660($gp)
    *******************************************************************/
    global_gametimer = (int16_t) _lh(addr); // WITHOUT GP!!
    #ifdef LOG
    logPrintf("[1] 0x%08X --> global_gametimer", global_gametimer); // 0x1DEC
    #endif
    return 1;
  }
  
  /// global_currentisland & global_systemlanguage [this function sets lots of globals -> weather, clock, cheatsused, gamespeed]
  /*************************************
   * ULUS-10160 v1.03 |  | 
   * ULES-00502 v1.02 |  | 
   * ULES-00503 v1.02 |  | 
   * ULJM-05297 v1.01 |  | 
   * ULET-00417 v0.06 |  |
   **************************************/ 
  if( _lw(addr + 0x4c) == 0xAE040014  &&  _lw(addr + 0x1A8) == 0xA2040074 ) { 
    /*******************************************************************
     *  0x002B2660: 0x8F851DE4 '....' - lw         $a1, 7652($gp)
    *******************************************************************/
    global_currentisland = (int16_t) _lh(addr+0xC); // WITHOUT GP!!
    #ifdef LOG
    logPrintf("[1] 0x%08X --> global_currentisland", global_currentisland); // uGp00001de4
    #endif
    
    /*******************************************************************
     *  0x002B2694: 0x8F841EBC '....' - lw         $a0, 7868($gp)
    *******************************************************************/
    global_systemlanguage = (int16_t) _lh(addr+0x40); // WITHOUT GP!!
    #ifdef LOG
      logPrintf("[1] 0x%08X --> global_systemlanguage", global_systemlanguage); // uGp00001EBC
    #endif
    
    return 1;
  }
  
  
  /// global for helpbox (special! there should be an address at gp+global_helpbox -> addr there + x
  if( _lw(addr + 0x80) == 0x3404014B && _lw(addr + 0x258) == 0x34040168 ) { // 0x00184E28
    /**** from cheat give weapons 1 ***********************************
     * 0x00184E28: 0x8F8416D8 '....' - lw         $a0, 5848($gp)
    *******************************************************************/
    global_helpbox = (int16_t)_lh(addr); // WITHOUT GP!!
    #ifdef LOG
    logPrintf("0x%08X -> global_helpbox", global_helpbox); // iGp000016d8
    #endif
    
    return 1;
  }
  
  /// weather
  if( _lw(addr - 0x10) == 0x03E00008 && _lw(addr - 0x8) == 0x03E00008 && _lw(addr+0x8) == 0x03E00008 && _lw(addr+0x10) == 0x2404FFFF && _lw(addr+0x14) == 0x03E00008 ) {
    global_weather = (int16_t)_lh(addr + 0x4); // WITHOUT GP!! (only to READ current weather)
    #ifdef LOG
    logPrintf("0x%08X -> global_weather", global_weather); //0x2098
    #endif
    return 1;
  }
  
  

  
    /// global_garagedata
  if( _lw(addr - 0x10) == 0x2404FFFF && _lw(addr + 0x3C) == 0x24C60030 ) {  // 0x46F9B0
    /*******************************************************************
     *  0x00168E80: 0x3C050047 'G..<' - lui        $a1, 0x47
     *  0x00168E90: 0x24A5F9B0 '...$' - addiu      $a1, $a1, -1616
    *******************************************************************/
    global_garagedata = (_lh(addr) * 0x10000) + (int16_t)_lh(addr+0x10); // actual address!
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> global_garagedata", global_garagedata-text_addr, global_garagedata); //
    #endif
    
    var_garageslots = *(char*)(addr+0x24); // 0xC = 12 slots (4 in each garage)
    var_garageslotsize = *(char*)(addr+0x3C); // 0x30
    #ifdef LOG
    logPrintf("var_garageslots = 0x%08X", var_garageslots); //
    logPrintf("var_garageslotsize = 0x%08X", var_garageslotsize); //
    #endif
    
    return 1;
  }
    
  if( _lw(addr + 0x8) == 0x3C0442C8 && _lw(addr + 0x50) == 0x24840001 ) { // 0x0018d03c
    #ifdef LOG
    logPrintf("0x%08X (0x%08X) -> GetPercentageGameComplete", addr-text_addr, addr);
    #endif
    GetPercentageGameComplete = (void*)(addr); // FUN_0018d03c
    return 1;
  }
  
  if( _lw(addr - 0x54) == 0x29AE0017 && _lw(addr + 0xA8) == 0x24840004  ) { // 0x0018D870
    /*******************************************************************
     *  0x0018D870: 0xAF801FAC '....' - sw         $zr, 8108($gp)
     *  0x0018D874: 0xAF801FB0 '....' - sw         $zr, 8112($gp)
    *******************************************************************/
    global_usjdone = (int16_t) _lh(addr); // WITHOUT GP!!
    global_usjtotal = (int16_t) _lh(addr+0x4); // WITHOUT GP!!
    global_rampagesdone = (int16_t) _lh(addr+0x4C); // WITHOUT GP!!
    global_rampagestotal = (int16_t) _lh(addr+0x50); // WITHOUT GP!!
    #ifdef LOG
    logPrintf("0x%08X --> global_usjdone", global_usjdone); // piGp00001fac
    logPrintf("0x%08X --> global_usjtotal", global_usjtotal); // iGp00001fb0
    logPrintf("0x%08X --> global_rampagesdone", global_rampagesdone); // piGp000024a0
    logPrintf("0x%08X --> global_rampagestotal", global_rampagestotal); // iGp000024a4
    #endif
    return 1;
  }
  if( _lw(addr - 0x2C) == 0x28850006 && _lw(addr + 0x28) == 0x28850008  ) { // 0x0018DA80
    /*******************************************************************
     *  0x0018DA80: 0xA3802078 'x ..' - sb         $zr, 8312($gp)
    *******************************************************************/
    global_empireowned = (int16_t) _lh(addr); // WITHOUT GP!!
    global_exportedVehicles = (int16_t) _lh(addr+0x58); // WITHOUT GP!!
    global_exportVehicTotal = (int16_t) _lh(addr+0x5C); // WITHOUT GP!!
    #ifdef LOG
    logPrintf("0x%08X --> global_empireowned", global_empireowned); // uGp00002078
    logPrintf("0x%08X --> global_exportedVehicles", global_exportedVehicles); // piGp00001fd4
    logPrintf("0x%08X --> global_exportVehicTotal", global_exportVehicTotal); // iGp00001fd8
    #endif
    return 1;
  }
  

  
  return 0;
}


int patch() {
  
  #ifdef LOG
  logPrintf("[INFO] found 'GTA3' module! mod_text_addr = 0x%08X, text_size = 0x%08X, data_size = 0x%08X", mod_text_addr, mod_text_size, mod_data_size);
  #endif
  
  
  /// blacklist
  if( mod_text_size == 0x00386750 && mod_data_size == 0x0001F7E0) { //ULJM-05297_v1.01
    #ifdef LOG
    logPrintf("[ERROR] unsupported game version");
    #endif
    return -1; //exit out
    
  } else if( mod_text_size == 0x0033388C && mod_data_size == 0x0002FEB0) { //ULJM-05255_v1.01
    #ifdef LOG
    logPrintf("[ERROR] unsupported game version");
    #endif
    return -1; //exit out
    
  }
  
  
  #ifdef LOG
  logPrintf("\n> searching & patching EBOOT..\n");
  #endif
  u32 i;
  int lcs_counter = 0, vcs_counter = 0;
  for (i = 0; i < mod_text_size; i += 4) {
    u32 addr = mod_text_addr + i;
    
    // first hit decides on version -> yes, potentially dangerous
    if( (LCS == 1 || (LCS == VCS)) && PatchLCS(addr, mod_text_addr)) {
      lcs_counter++;
      LCS = 1;
      continue;
    }
    if( (VCS == 1 || (LCS == VCS)) && PatchVCS(addr, mod_text_addr)) {
      vcs_counter++;
      VCS = 1;
      continue;
    }
  }
  

  #ifdef LOG
  logPrintf("[INFO] %i LCS & %i VCS locations found", lcs_counter, vcs_counter);
  #endif
  if( lcs_counter > 0 && vcs_counter > 0 ) { // error check
    #ifdef LOG
    logPrintf("[ERROR] both counters found something");
    #endif
    return -1;
  }
  if( LCS == VCS ) { // error check
    #ifdef LOG
    logPrintf("[ERROR] LCS == VCS");
    #endif  
    return -1;
  }
  
  #ifdef LOG
  logPrintf("[INFO] loadprogress()");
  #endif  
  loadprogress(trophies, trophies_size);
  
  if( trophies_getTotalCurrentGame() == trophies_getDoneCurrentGame() )
    alldone = 1; // all unlocked in previous game session. no need to reshow Congrats message
  
  #ifdef LOG
  logPrintf("\n> Setup all done! Starting game..\n");
  #endif  
  
  return 0; //success
}


static void CheckModules() { // PPSSPP only
  SceUID modules[10];
  int count = 0;
  if (sceKernelGetModuleIdList(modules, sizeof(modules), &count) >= 0) {
    int i;
    SceKernelModuleInfo info;
    for (i = 0; i < count; ++i) {
      info.size = sizeof(SceKernelModuleInfo);
      if (sceKernelQueryModuleInfo(modules[i], &info) < 0) {
        continue;
      }
      if (strcmp(info.name, "GTA3") == 0) {
    mod_text_addr = info.text_addr;
    mod_text_size = info.text_size;
    mod_data_size = info.data_size;
    
    /// with this approach the game continues to run when patch() is still in progress
    
    int ret = patch();
    if( ret != 0 ) // patching returned error
      return;

    sceKernelDcacheWritebackAll();
    return;
      }
    }
  }
}


int OnModuleStart(SceModule2 *mod) {
  char *modname = mod->modname;
  
  if( strcmp(modname, "GTA3") == 0 ) {
  mod_text_addr = mod->text_addr;
  mod_text_size = mod->text_size;
  mod_data_size = mod->data_size;

  int ret = patch();
  if( ret != 0 ) // patching returned error
    return -1;
  
    sceKernelDcacheWritebackAll();
  }

  if( !previous )
    return 0;

  return previous(mod);
}


int module_start(SceSize argc, void* argp) {
  
  /// remove old log file (if there is one)
  sceIoRemove(file_log); // delete old logfile
  
  #ifdef LOG
  logPrintf("[INFO] Starting GTA Trophies (argc: %i, argp: %s)", argc, argp);
  logPrintf("----------------------------\n");
  #endif


  /// check PPSSPP
  if( sceIoDevctl("kemulator:", 0x00000003, NULL, 0, NULL, 0) == 0 ) {
    PPSSPP = 1;
    #ifdef LOG
    logPrintf("[INFO] PPSSPP detected!");
    #endif
    #ifdef DEBUG
    sceIoRemove("ms0:/PSP/PLUGINS/gta_trophies/gta_trophies.sav");
    #endif
  }

  /// savefile
  if(argc > 0) { // on real hardware we use module_start's argp path to put the savefile next to the prx
    /// location depending on where prx is loaded from
    strcpy(save, (char*)argp);
    ptr = strrchr(save, '/');
    if(!ptr) return -1; // error
    strcpy(ptr + 1, file_save);
    
  } else { // no arguments found
    if( PPSSPP ) // PPSSPP doesn't have the prx's path in argp (last tested for v1.12.3)
      sprintf(save, "ms0:/PSP/PLUGINS/gta_trophies/%s", file_save);
    else // last resort
      sprintf(save, "ms0:/seplugins/%s", file_save);    
  }
  #ifdef LOG
  if( doesFileExist(save) ) 
    logPrintf("[INFO] Savefile found! (%s)", save);
  #endif
  

  /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
  
  if( PPSSPP ) 
    CheckModules(); // scan the modules using normal/official syscalls (https://github.com/hrydgard/ppsspp/pull/13335#issuecomment-689026242)
  else // PSP
    previous = sctrlHENSetStartModuleHandler(OnModuleStart); 
  
  return 0;
}
