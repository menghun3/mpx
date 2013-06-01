/*****************************************************************************
* author menghun3@gmail.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation.
* 
* source code address https://github.com/menghun3/mpx
******************************************************************************/
/*
使用C语言方式需要定义宏COBJMACROS
*/
// #ifndef DS_CPP
// #define COBJMACROS
// #endif
#include <windows.h>
#include <tchar.h>
#include <DShow.h>
#include <stdio.h>

#include "mpx.h"

#pragma comment(lib, "Strmiids.lib")

// 版本号
#define MPX_MAJOR_VERSION 0
#define MPX_MINOR_VERSION 1
#define MPX_REVISION_VERSION 0
#define MPX_BUILD_VERSION 0

typedef enum 
{
	PT_LOADING = 1,
	PT_PLAY,
	PT_PAUSE,
	PT_STOP,
	PT_QUIT,
	PT_LAST
}PT_CTROL;

#ifdef DS_CPP
IGraphBuilder *pGraph = NULL;
IMediaControl *pControl = NULL;
IMediaEvent *pEvent = NULL;
IMediaSeeking *pSeek = NULL;
#else
IGraphBuilder *pGraph = NULL;
IMediaControl *pControl = NULL;
IMediaEvent *pEvent = NULL;
IMediaSeeking *pSeek = NULL;
IBasicAudio *pBA = NULL;
#endif
HANDLE h = NULL;
int threadid = 0;
HANDLE checkh = NULL;
int chechid = 0;

// 内部接口
void PlayThread(void *param);
void CheckThread(void *param);
void InitPlay();
void DestroyPlay();
void mpPlay();
void mpPlay1(void *playSongPath);
void mpStop();
void mpPause();
int mpGetCurrentPosition(long long *curpos);
int mpGetPositions(long long *curpos, long long *stoppos);
int mpGetStopPosition(long long *stoppos);
int mpGetVolume(long *plVolume);
int mpPutVolume(long lVolume);
int mpGetGuidFormat(GUID *pFormat);
int mpGetRate(double *dRate);

// 播放文件
void mpxPlayFile(void *path)
{
	if (h)
	{
		PostThreadMessage(threadid, PT_QUIT, (WPARAM)0, (LPARAM)0);
		WaitForSingleObject(h, INFINITE);
	}

	h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayThread, path, 0, (LPDWORD)&threadid);
	if (NULL == h)
	{
		printf("create thread error\n");
		return;
	}
}

// 播放
void mpxPlay()
{
	PostThreadMessage(threadid, PT_PLAY, (WPARAM)0, (LPARAM)0);
}

// 暂停
void mpxPause()
{
	PostThreadMessage(threadid, PT_PAUSE, (WPARAM)0, (LPARAM)0);
}

// 停止
void mpxStop()
{
	PostThreadMessage(threadid, PT_STOP, (WPARAM)0, (LPARAM)0);
}

// 初始化
void mpxInit()
{
	InitPlay();
}

// 销毁
void mpxDestroy()
{
	PostThreadMessage(threadid, PT_QUIT, (WPARAM)0, (LPARAM)0);
}

int mpxGetCurrentPosition(long long *curpos)
{
	return mpGetCurrentPosition(curpos);
}

int mpxGetPositions(long long *curpos, long long *stoppos)
{
	return mpGetPositions(curpos, stoppos);
}

int mpxGetStopPosition(long long *stoppos)
{
	return mpGetStopPosition(stoppos);
}

int mpxGetVolume(long *plVolume)
{
	return mpGetVolume(plVolume);
}

int mpxPutVolume(long lVolume)
{
	return mpPutVolume(lVolume);
}

int mpxGetGuidFormat(GUID *pFormat)
{
	return mpGetGuidFormat(pFormat);
}

int mpxGetRate(double *dRate)
{
	return mpGetRate(dRate);
}

#ifdef DS_CPP
void InitPlay()
{
	HRESULT hr = 0;
	
	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("init com error\n");
		return;
	}

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);

	if (FAILED(hr))
	{
		printf("create fgm error\n");
		return ;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	if (FAILED(hr))
	{
		printf("get control error\n");
		return ;
	}
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	if (FAILED(hr))
	{
		printf("get event error\n");
		return;
	}
	hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeek);
	if (FAILED(hr))
	{
		printf("get seek error\n");
		return;
	}
}


void DestroyPlay()
{
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();
}

void mpPlay()
{
	pControl->Run();
}

void mpPlay1(WCHAR *playSongPath)
{
	HRESULT hr;

	hr = pGraph->RenderFile(playSongPath,NULL);
	if (SUCCEEDED(hr))
	{
		hr = pControl->Run();
		if (FAILED(hr))
		{
			printf("run error\n");
		}
		checkh = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)CheckThread, NULL, 0, (LPDWORD)&chechid);
		if (checkh == NULL)
		{
			printf("create chech thread error\n");
		}
	}
	else
	{
		printf("render file error\n");
	}
}

void mpStop()
{
	HRESULT hr = 0;
	long long startpos = 0;
	hr = pSeek->SetPositions(&startpos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	pControl->Stop();
}

void mpPause()
{
	pControl->Pause();
}

void PlayThread(void *param)
{
	WCHAR *songpath = (WCHAR *)param;
	PT_CTROL ptstatus  = PT_LAST;
	InitPlay();
	mpPlay1(songpath);
	ptstatus = PT_PLAY;
	do 
	{
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case PT_LOADING:
				{
					WCHAR *path = (WCHAR*)msg.lParam;
					ptstatus = PT_PLAY;
					mpPlay1(path);
				}
				break;
			case PT_PLAY:
				{
					ptstatus = PT_PLAY;
					mpPlay();
				}
				break;
			case PT_PAUSE:
				{
					ptstatus = PT_PAUSE;
					mpPause();
				}
				break;
			case PT_STOP:
				{
					ptstatus = PT_STOP;
					mpStop();
				}
				break;
			case PT_QUIT:
				{
					ptstatus = PT_QUIT;
					mpStop();
					if (checkh)
					{
						CloseHandle(checkh);
						checkh = NULL;
					}
					DestroyPlay();
					return;
				}
				break;
			default:
				{
				}
				break;
			}
		}
	} while (1);
}

void CheckThread(void *param)
{
	HRESULT hr;
	while (1)
	{
		long long stoppos = 0;
		long long curpos = 0;
		Sleep(1000);
		hr = pSeek->GetPositions(&curpos, &stoppos);
		if (curpos == stoppos)
		{
			PostThreadMessage(threadid, PT_QUIT, NULL, NULL);
			printf("play end\n");
			return;
		}
	}
}

#else /* C style interface */

void InitPlay()
{
	HRESULT hr = 0;

	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("init com error\n");
		return;
	}

	hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder, (void **)&pGraph);

	if (FAILED(hr))
	{
		printf("create fgm error\n");
		return ;
	}
	hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaControl, (void **)&pControl);
	if (FAILED(hr))
	{
		printf("get control error\n");
		return ;
	}
	hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaEvent, (void **)&pEvent);
	if (FAILED(hr))
	{
		printf("get event error\n");
		return;
	}
	hr =IGraphBuilder_QueryInterface(pGraph, &IID_IMediaSeeking, (void **)&pSeek);
	if (FAILED(hr))
	{
		printf("get seek error\n");
		return;
	}

	hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicAudio, (void **)&pBA);
	if (FAILED(hr))
	{
		printf("get basic audio error\n");
		return;
	}

}

void DestroyPlay()
{
	IBasicAudio_Release(pBA);
	pBA = NULL;
	IMediaSeeking_Release(pSeek);
	pSeek = NULL;
	IMediaControl_Release(pControl);
	pControl = NULL;
	IMediaEvent_Release(pEvent);
	pEvent = NULL;
	IGraphBuilder_Release(pGraph);
	pGraph = NULL;
	CoUninitialize();
}


void mpPlay()
{
	IMediaControl_Run(pControl);
}

void mpPlay1(void *playSongPath)
{
	HRESULT hr;
	hr = IGraphBuilder_RenderFile(pGraph, playSongPath, NULL);
	if (SUCCEEDED(hr))
	{
		hr = IMediaControl_Run(pControl);
		if (FAILED(hr))
		{
			printf("run error\n");
		}
		checkh = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)CheckThread, NULL, 0, (LPDWORD)&chechid);
		if (checkh == NULL)
		{
			printf("create chech thread error\n");
		}
	}
	else
	{
		printf("render file error\n");
	}
}

void mpStop()
{
	HRESULT hr = 0;
	long long startpos = 0;
	hr =IMediaSeeking_SetPositions(pSeek, &startpos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	hr = IMediaControl_Stop(pControl);
}

void mpPause()
{
	HRESULT hr = 0;
	hr = IMediaControl_Pause(pControl);
}

int mpGetCurrentPosition(long long *curpos)
{
	HRESULT hr = 0;
	if (pSeek)
	{
		hr = IMediaSeeking_GetCurrentPosition(pSeek, curpos);
	}
	return hr;
}

int mpGetPositions(long long *curpos, long long *stoppos)
{
	HRESULT hr = 0;
	if (pSeek)
	{
		hr = IMediaSeeking_GetPositions(pSeek, curpos, stoppos);
	}
	return hr;
}

int mpGetStopPosition(long long *stoppos)
{
	HRESULT hr = 0;
	if (pSeek)
	{
		hr = IMediaSeeking_GetStopPosition(pSeek, stoppos);
	}
	return hr;
}

int mpGetVolume(long *plVolume)
{
	HRESULT hr = 0;
	if (plVolume == NULL)
	{
		return hr;
	}
	if (pBA)
	{
		hr = IBasicAudio_get_Volume(pBA, plVolume);
	}
	return hr;
}

//从-10000到0
int mpPutVolume(long lVolume)
{
	HRESULT hr = 0;

	if (pBA)
	{
		hr = IBasicAudio_put_Volume(pBA, lVolume);
	}
	return hr;
}

int mpGetGuidFormat(GUID *pFormat)
{
	HRESULT hr = 0;

	if (pBA)
	{
		hr = IMediaSeeking_GetTimeFormat(pSeek, pFormat);
	}
	return hr;
}

int mpGetRate(double *dRate)
{
	HRESULT hr = 0;

	if (pBA)
	{
		hr = IMediaSeeking_GetRate(pSeek, dRate);
	}
	return hr;
}

void PlayThread(void *param)
{
	WCHAR *songpath = (WCHAR *)param;
	PT_CTROL ptstatus  = PT_LAST;
	InitPlay();
	mpPlay1(songpath);
	ptstatus = PT_PLAY;
	do 
	{
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case PT_LOADING:
				{
					WCHAR *path = (WCHAR*)msg.lParam;
					ptstatus = PT_PLAY;
					mpPlay1(path);
				}
				break;
			case PT_PLAY:
				{
					ptstatus = PT_PLAY;
					mpPlay();
				}
				break;
			case PT_PAUSE:
				{
					ptstatus = PT_PAUSE;
					mpPause();
				}
				break;
			case PT_STOP:
				{
					ptstatus = PT_STOP;
					mpStop();
				}
				break;
			case PT_QUIT:
				{
					ptstatus = PT_QUIT;
					mpStop();
					if (checkh)
					{
						CloseHandle(checkh);
						checkh = NULL;
					}
					DestroyPlay();
					return;
				}
				break;
			default:
				{
				}
				break;
			}
		}
	} while (1);
}

void CheckThread(void *param)
{
	HRESULT hr;
	while (1)
	{
		long long stoppos = 0;
		long long curpos = 0;
		Sleep(1000);
		if (pSeek == NULL)
		{
			return;
		}
		hr = IMediaSeeking_GetPositions(pSeek, &curpos, &stoppos);
		if (curpos == stoppos)
		{
			PostThreadMessage(threadid, PT_QUIT, (WPARAM)0, (LPARAM)0);
			return;
		}
	}
}


#endif
