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

#ifndef __MAIN_H__
#define __MAIN_H__

/** FLAGS ***********/
//#define LOG
//#define DEBUG

/********************/

#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a);

#define MAKE_JUMP(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);

#define MAKE_DUMMY_FUNCTION(a, r) { \
  u32 _func_ = a; \
  if (r == 0) { \
    _sw(0x03E00008, _func_); \
    _sw(0x00001021, _func_ + 4); \
  } else { \
    _sw(0x03E00008, _func_); \
    _sw(0x24020000 | r, _func_ + 4); \
  } \
}

#define REDIRECT_FUNCTION(a, f) { \
  u32 _func_ = a; \
  _sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), _func_); \
  _sw(0, _func_ + 4); \
}

#define HIJACK_FUNCTION(a, f, ptr) { \
  u32 _func_ = a; \
  static u32 patch_buffer[3]; \
  _sw(_lw(_func_), (u32)patch_buffer); \
  _sw(_lw(_func_ + 4), (u32)patch_buffer + 8);\
  MAKE_JUMP((u32)patch_buffer + 4, _func_ + 8); \
  _sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), _func_); \
  _sw(0, _func_ + 4); \
  ptr = (void *)patch_buffer; \
}

int (* GetPPLAYER)();
int (* GetPCAR)();
int (* StartNewScript)(int ip);
int (* SetHelpMessage)(ushort *msg, char quick, char forever);
int (* IsHelpMessageBeingDisplayed)(void);

float (* GetPercentageGameComplete)(void);

void (*PrintString_LCS)(float X, float Y, wchar_t* text, int* unknown);
void (*PrintString_VCS)(wchar_t* text, int x, int y);
void (*SetRightJustifyOn)();
void (*SetRightJustifyOff)();
void (*SetCentreOff)();
void (*SetCentreOn)();
void (*SetJustifyOn)();
void (*SetJustifyOff)();
void (*SetColor)(u32* color);
void (*SetBackgroundColor)(u32* color);
void (*SetFontStyle)(short style);
void (*SetDropShadowPosition)(short bool);
void (*SetDropColor)(char* rgba);
void (*SetScale_LCS)(float width, float height);
void (*SetScale_VCS)(float scale);
void (*SetBackGroundOnlyTextOn)();
void (*SetBackGroundOnlyTextOff)();
void (*SetPropOn)();
void (*SetPropOff)();
void (*SetBackgroundOn)();
void (*SetBackgroundOff)();
void (*SetTextOriginPoint)(int origin);
void (*SetTextSpaceing)(int option);
void (*SetTextBounds)(float* array);
void (*ResetFontStyling)();
void (*SetRightJustifyWrap)(float x);
void (*SetWrapx)(float x);
void (*SetSlant)(float param_1);
void (*SetCentreSize)(float param_1);
void (*SetSlantRefPoint)(float param_1, float param_2);

#define SCREEN_WIDTH 480.0f
#define SCREEN_HEIGHT 272.0f

enum{
  FALSE, // 0
  TRUE   // 1
};

typedef struct {
  short id;
  char unlocked;
  char* title;
  char* desc;
} trophies_pack;

void congrats();
void trophy();

int trophies_getTotal();
int trophies_getTotalCurrentGame();
int trophies_getDoneTotal();
int trophies_getDoneCurrentGame();

int module_stop(int argc, char *argv[]);
int module_start(SceSize argc, void* argp);

#endif