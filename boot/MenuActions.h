#ifndef _MENUACTIONS_H_
#define _MENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//For the text menu jump
void MoveToTextMenu(void *nothing);


#ifdef ETHERBOOT
void BootFromEtherboot(void *);
#endif

void SetLEDColor(void *);
void BootFromCD(void *);
void BootFromNative(void *);
void BootFromFATX(void *);
#endif
