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

#ifndef __UTILSS_H__
#define __UTILSS_H__

int logPrintf(const char *text, ...);
int doesFileExist(const char* path);

void AsciiToUnicode(const char* in, wchar_t* out);
int wcscmp(const wchar_t *s1, const wchar_t *s2);

int checkCoordinateInsideArea(float a, float b, float c, float x, float y, float z, float radius);

/////////////////////////////////////

void setFloat(int adr, float value);
float getFloat(int adr);

void setInt(int adr, int value);
int getInt(int adr);

void setShort(int adr, short value);
short getShort(int adr);

void setChar(int adr, char value);
char getChar(int adr);

void setByte(int adr, unsigned char value);
unsigned char getByte(int adr);

#endif