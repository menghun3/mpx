/*****************************************************************************
* author menghun3@gmail.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation.
* 
* source code address https://github.com/menghun3/mpx
******************************************************************************/
#ifndef MPX_H
#define MPX_H

void mpxInit();
void mpxDestroy();
void mpxPlayFile(void *path);
void mpxPlay();
void mpxPause();
void mpxStop();
int mpxGetCurrentPosition(long long *curpos);
int mpxGetPositions(long long *curpos, long long *stoppos);
int mpxGetStopPosition(long long *stoppos);
int mpxGetVolume(long *plVolume);
int mpxPutVolume(long lVolume);
int mpxGetGuidFormat(GUID *pFormat);
int mpxGetRate(double *dRate);

#endif // MPX_H
