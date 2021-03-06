/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "boot.h"
#include "BootIde.h"
#include "video.h"

void LockHdd(void *driveId);

void UnlockHdd(void *driveId);

void DisplayHddPassword(void *driveId);
