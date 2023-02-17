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
#include <malloc.h>

#include "main.h"
#include "utils.h"

extern int magic;
extern char save[];
extern trophies_pack trophies[];
extern int trophies_size;

int loadprogress() {
  #ifdef LOG
  logPrintf("loadprogress (%s)", save);
  #endif  
  
  SceUID file = sceIoOpen(save, PSP_O_RDONLY, 0777);
  if(file < 0) {
    #ifdef LOG  
    logPrintf("sceIoOpen(%s) error 0x%08X", save, file);
    #endif
    return -1; // error
  }
  
  int local_magic = -1;
  sceIoRead(file, &local_magic, sizeof(int));
  
  if( magic != local_magic ) {
    #ifdef LOG
    logPrintf("magic mismatch!");
    #endif
    sceIoClose(file);
    sceIoRemove(save);
    return -1; // error 
  }
  
  int i, read;
  short local_id;
  char local_unlocked;
  do {
    local_id = -1;
    local_unlocked = -1;
    read = sceIoRead(file, &local_id, sizeof(short));
    if( read <= 0 ) break;
    sceIoRead(file, &local_unlocked, sizeof(char));
    #ifdef LOG
    logPrintf("checking: id %X val %X", local_id, local_unlocked);
    #endif
    for(i = 0; i < trophies_size; i++) {
      if( trophies[i].id == local_id ) {
        #ifdef LOG
        logPrintf("set!");
        #endif
        trophies[i].unlocked = local_unlocked;
        break;
      }
    } 
  } while( read );
  
  sceIoClose(file);  
  return 0; // success
}

int saveprogress() {
  #ifdef LOG
  logPrintf("saveprogress (%s)", save);
  #endif  
  
  int i;
    
  SceUID file = sceIoOpen(save, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  if(file < 0) {
    #ifdef LOG
    logPrintf("sceIoOpen(%s) error (0x%08X)", save, file);
    #endif  
    return -1; // error
  }
  
  sceIoWrite(file, &magic, sizeof(int)); // magic
  for(i = 0; i < trophies_size; i++) {
    sceIoWrite(file, &trophies[i].id, sizeof(short));
    sceIoWrite(file, &trophies[i].unlocked, sizeof(char));
  } 
  
  sceIoClose(file);  
  return 0; // success
}