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
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <math.h>

#include "utils.h"
#include "main.h"

extern const char *file_log;
int logPrintf(const char *text, ...) {
  va_list list;
  char string[512];

  va_start(list, text);
  vsprintf(string, text, list);
  va_end(list);

  SceUID fd = sceIoOpen(file_log, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
  if( fd >= 0 ) {
    sceIoWrite(fd, string, strlen(string));
    sceIoWrite(fd, "\n", 1);
    sceIoClose(fd);
  }

  return 0;
}

void AsciiToUnicode(const char* in, wchar_t* out) {
	while( *in != '\0' ) {
		*out++ = *in++;
	} *out = '\0';
}

int wcscmp(const wchar_t *s1, const wchar_t *s2) {
  wchar_t c1, c2;
  do {
      c1 = *s1++;
      c2 = *s2++;
      if (c2 == L'\0')
	return c1 - c2;
  } while (c1 == c2);
  return c1 < c2 ? -1 : 1;
}

int doesFileExist(const char* path) {
  SceUID dir = sceIoOpen(path, PSP_O_RDONLY, 0777); 
   if( dir >= 0 ) {
     sceIoClose(dir); 
     return 1; // true
   }
   return 0; // false
} 

int checkCoordinateInsideArea(float a, float b, float c, float x, float y, float z, float radius) {
  // logPrintf("%f %f", a, x);
  if( sqrt(pow(a-x, 2) + pow(b-y, 2) + pow(c-z, 2)) <= radius ) // Let the sphere's centre coordinates be (x,y,z) and its radius r, then point (a,b,c) is in the sphere if (a−x)^2 + (b−y)^2 + (c−z)^2 < r^2.
    return 1; // true
  return 0; // false
}

//////////////////////////////////////////////////////////////////////////////////////////

u32 memory_low  = 0x08400000; // memory bounds
u32 memory_high = 0x0C000000; // (high mem. hardcoded = bad, TODO!)

void setByte(int adr, unsigned char value) { // 8bit (unsigned 0-255)
  if( adr >= memory_low && adr < memory_high )
    *(unsigned char*)adr = value;
}

unsigned char getByte(int adr) { // 8bit (unsigned 0-255)
  if( adr >= memory_low && adr < memory_high )
    return *(unsigned char*)adr;
  return 0;
}

void setChar(int adr, char value) { // 8bit (signed -128 to +127)  
  if( adr >= memory_low && adr < memory_high )
    *(char*)adr = value;
}

char getChar(int adr) { // 8bit (signed -128 to +127)
  if( adr >= memory_low && adr < memory_high )
    return *(char*)adr;
  return 0;
}

void setShort(int adr, short value) { // 16bit
  if( adr >= memory_low && adr < memory_high && (adr % 2 == 0) )
    *(short*)adr = value;
}

short getShort(int adr) { // 16bit
  if( adr >= memory_low && adr < memory_high && (adr % 2 == 0) )
    return *(short*)adr;
  return 0;
}

void setInt(int adr, int value) { // 32bit
  if( adr >= memory_low && adr < memory_high && (adr % 4 == 0) )
    *(int*)adr = value;
}

int getInt(int adr) { // 32bit
  if( adr >= memory_low && adr < memory_high && (adr % 4 == 0))
    return *(int*)adr;
  return 0;
}

void setFloat(int adr, float value) { // 32bit
  if( adr >= memory_low && adr < memory_high && (adr % 4 == 0) )
    *(float*)adr = value;
}

float getFloat(int adr) { // 32bit
  if( adr >= memory_low && adr < memory_high && (adr % 4 == 0) )
    return *(float*)adr;
  return 0.0f;
}