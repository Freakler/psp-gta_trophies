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

#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

int getIsland();
int getGametime();
int getMultiplayer();
float getVehicleSpeed(int vehicle_base_adr);
unsigned char getPedID(int ped_base_adr);
short getVehicleID(int vehicle_base_adr);
short getWeather();
int getWantedLevel(int ped_base_adr);
char getNumberEmpiresOwned();
int getGarageVehicleActiveObjects(int base);
void setTimedTextbox(const char *sentence, float duration);
int isTextboxShowing();
void CustomScriptExecute(int address);
int getCarsCrushed();
int getUsjDone();
int getUsjTotal();
int getPedDrowning(int ped_base_adr);
float getPedHealth(int ped_base_adr);
int getKilledHitmen();

#endif