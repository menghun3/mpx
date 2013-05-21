#ifndef MPX_H
#define MPX_H
// 对外接口
void mpxInit();
void mpxDestroy();
void mpxPlayFile(void *path);
void mpxPlay();
void mpxPause();
void mpxStop();
int mpxGetCurrentPosition(long long *curpos);
int mpxGetPositions(long long *curpos, long long *stoppos);
int mpxGetStopPosition(long long *stoppos);

#endif // MPX_H
