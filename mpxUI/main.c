/*****************************************************************************
* author menghun3@gmail.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation.
* 
* source code address https://github.com/menghun3/mpx
******************************************************************************/
#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <strsafe.h>
#include <conio.h>
#include <ShlObj.h>

#include "mpx.h"
#include "playlist.h"
#include "lyric.h"

#pragma comment (lib, "Comctl32.lib")
#pragma comment (lib, "mpx.lib")
#pragma comment (lib, "winmm.lib")

// 版本号
#define MPXUI_MAJOR_VERSION 0
#define MPXUI_MINOR_VERSION 1
#define MPXUI_REVISION_VERSION 0
#define MPXUI_BUILD_VERSION 0

//component http://msdn.microsoft.com/en-us/library/windows/desktop/bb775491(v=vs.85).aspx
//control library http://msdn.microsoft.com/en-us/library/windows/desktop/bb773169(v=vs.85).aspx
typedef enum
{
	C_BTN_PLAY = 1,
	C_BTN_PAUSE,
	C_BTN_PREV,
	C_BTN_NEXT,
	C_BTN_STOP,
	C_BTN_FROM_PL,
	C_LYRIC_STATIC,
	C_TIME_STATIC,
	C_PL_ADD,
	C_PL_ADDDIR,
	C_PL_DEL,
	C_PL_SAVE,
	C_LAST
}COMPONENT_ID;

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
int playIndex = 0;
static int status = -1; // C_BTN_PLAY:play;C_BTN_PAUSE:pause;C_BTN_STOP:stop
UINT g_timeId = 0;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void TimeEventProc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2);
void EndTimeEvent();
void BeginTimeEvent();
char *MakeLyricPathFromSongPath(char *songPath, char *lyricPath);
int AddFile(HWND hWnd);
int AddDirFile(HWND hWnd);
int DelFile(HWND hWnd);
void CreateCtrlBtn(HWND hWnd);
void CreateListBox(HWND hWnd);
void CreateLyricStatic(HWND hWnd);
void CreateTimeStatic(HWND hWnd);
void CreatePlayListBtn(HWND hWnd);

int APIENTRY WinMain(HINSTANCE hInstance, //应用程序的实例句柄，
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, //命令行
	int nCmdShow) //显示方式
{
	MSG msg;
	HWND hWnd;
	char szTitle[]="mpx"; // The title bar text
	WNDCLASSEX wcex={0};

	g_hInst = hInstance;
	wcex.cbSize = sizeof(WNDCLASSEX); //WNDCLASSEX结构体大小
	wcex.style = CS_HREDRAW | CS_VREDRAW; //位置改变时重绘
	wcex.lpfnWndProc = (WNDPROC)WndProc; //消息处理函数
	wcex.hInstance = 0; //当前实例句柄
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME; //背景色
	wcex.lpszClassName = "mpxclass"; //参窗口类名
	wcex.hIcon =0; //图标
	wcex.hCursor =LoadCursor(NULL, IDC_ARROW); //光标
	wcex.lpszMenuName =0; //菜单名称
	wcex.hIconSm =0; //最小化图标

	RegisterClassEx(&wcex); //注册窗口类

	hWnd = CreateWindowEx(WS_EX_ACCEPTFILES,"mpxclass", szTitle, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_VISIBLE, //创建窗口
		CW_USEDEFAULT,CW_USEDEFAULT,800, 400, NULL, NULL, 0, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	g_hWnd = hWnd;

	while (GetMessage(&msg, NULL, 0, 0)) // 消息循环:
	{
		TranslateMessage(&msg); //转化虚拟按键到字符消息
		DispatchMessage(&msg); //分派消息调用回调函数
	}
	return msg.wParam;
}

HWND btnPrev = NULL;
HWND btnPlay = NULL;
HWND btnStop = NULL;
HWND btnNext = NULL;
HWND playlistLB = NULL;
HWND lyricStatic = NULL;
HWND timeStatic = NULL;
HWND btnPLAdd = NULL;
HWND btnPLAddDir = NULL;
HWND btnPLDel = NULL;
HWND btnPLSave = NULL;

//windows class http://msdn.microsoft.com/en-us/library/bb775491(v=vs.85).aspx#WC_LISTVIEW
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	static RECT oldRect;
	switch (message) 
	{
	case WM_PAINT: //重绘消息
		{
			hdc = BeginPaint(hWnd, &ps);
			GetWindowRect(hWnd, &rect);
			if (rect.right - rect.left != oldRect.right - oldRect.left
				|| rect.bottom - rect.top != oldRect.bottom - oldRect.top)
			{
				oldRect = rect;
				SetWindowPos(playlistLB, NULL, 0, 70, 380, oldRect.bottom - oldRect.top - 120, SWP_SHOWWINDOW);
				SetWindowPos(lyricStatic, NULL, 400, 70, 380, oldRect.bottom - oldRect.top - 120, SWP_SHOWWINDOW);
			}
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_CREATE:
		{
			GetWindowRect(hWnd, &oldRect);
			CreateCtrlBtn(hWnd);
			CreateListBox(hWnd);
			CreateLyricStatic(hWnd);
			CreateTimeStatic(hWnd);
			CreatePlayListBtn(hWnd);
		}
		break;
	case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			switch (id)
			{
			case C_BTN_PREV:
				{
					char *p = NULL;
					static WCHAR songpath[MAX_PATH] = {0};
					char lyricpath[MAX_PATH] = {0};

					memset(songpath, 0, MAX_PATH);

					playIndex--;
					if (playIndex < 0)
					{
						playIndex =  GetDefaultPlaylistTotalItem();
					}

					p = GetItemFromDefaultPlaylist(playIndex);
					if (p == NULL)
					{
						break;
					}
					MakeLyricPathFromSongPath(p, lyricpath);
					LyricDestroy();
					LyricInit(lyricpath);

					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
					EndTimeEvent();
					mpxPlayFile(songpath);
					BeginTimeEvent();
					status = C_BTN_PLAY;
					SetWindowText(btnPlay, "pause");
					SetWindowText(hWnd, p);
				}
				break;
			case C_BTN_PLAY:
				{
					
					if (status != C_BTN_PLAY )
					{
						mpxPlay();
						status = C_BTN_PLAY;
						SetWindowText(btnPlay, "pause");
					}
					else if (status == C_BTN_PLAY)
					{
						mpxPause();
						status = C_BTN_PAUSE;
						SetWindowText(btnPlay, "play");
					}
				}
				break;
			case C_BTN_STOP:
				{
					if (status == C_BTN_PLAY || status == C_BTN_PAUSE)
					{
						mpxStop();
						status = C_BTN_STOP;
						SetWindowText(btnPlay, "play");
					}
				}
				break;
			case C_BTN_NEXT:
				{
					char *p = NULL;
					static WCHAR songpath[MAX_PATH] = {0};
					char lyricpath[MAX_PATH] = {0};

					memset(songpath, 0, MAX_PATH);

					playIndex++;
					if (playIndex > GetDefaultPlaylistTotalItem())
					{
						playIndex = 0;
					}

					p = GetItemFromDefaultPlaylist(playIndex);
					if (p == NULL)
					{
						break;
					}
					MakeLyricPathFromSongPath(p, lyricpath);
					LyricDestroy();
					LyricInit(lyricpath);

					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
					EndTimeEvent();
					mpxPlayFile(songpath);
					BeginTimeEvent();
					status = C_BTN_PLAY;
					SetWindowText(btnPlay, "pause");
					SetWindowText(hWnd, p);
				}
				break;
			case C_BTN_FROM_PL:
				{
					int selid = HIWORD(wParam);
					switch (selid) 
					{ 
					case LBN_DBLCLK:
						{
							// Get selected index.
							int lbItem = (int)SendMessage(playlistLB, LB_GETCURSEL, 0, 0); 
							char *p = NULL;
							static WCHAR songpath[MAX_PATH] = {0};
							char lyricpath[MAX_PATH] = {0};

							memset(songpath, 0, MAX_PATH);

							playIndex = lbItem;
							p = GetItemFromDefaultPlaylist(playIndex);
							if (p == NULL)
							{
								break;
							}
							MakeLyricPathFromSongPath(p, lyricpath);
							LyricDestroy();
							LyricInit(lyricpath);

							MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
							EndTimeEvent();
							mpxPlayFile(songpath);
							BeginTimeEvent();
							status = C_BTN_PLAY;
							SetWindowText(btnPlay, "pause");
							SetWindowText(hWnd, p);
						}
						break;
					default:
						{
						}
						break;
					}
				}
				break;
			case C_PL_ADD:
				{
					AddFile(hWnd);
				}
				break;
			case C_PL_ADDDIR:
				{
					AddDirFile(hWnd);
				}
				break;
			case C_PL_DEL:
				{
					DelFile(hWnd);
				}
				break;
			case C_PL_SAVE:
				{
					DefaultPlaylistSave();
				}
				break;
			default:
				break;
			}
		}
		break;
	case WM_DESTROY: //窗口销毁消息
		PostQuitMessage(0);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// http://msdn.microsoft.com/en-us/library/bb775146(v=vs.85).aspx
void CreateListBox(HWND hWnd)
{
	int i = 0;
	POINT p = {10, 70};
	int ident = 0;
	int plItemCount = 0;
	int colWidth = 0;
	int colMaxWidth = 300;
	RECT rect;

	GetWindowRect(hWnd, &rect);

	PlayListInit();
	plItemCount = GetDefaultPlaylistTotalItem();
	playlistLB = CreateWindowEx(WS_EX_ACCEPTFILES, WC_LISTBOX, "default", 
		WS_VISIBLE | LBS_COMBOBOX | WS_CHILD|WS_VSCROLL|LBN_DBLCLK|LBS_NOTIFY, 
		0, 70, 380, rect.bottom - rect.top - 120, hWnd, C_BTN_FROM_PL, g_hInst, NULL); 
	for (i = 0; i < plItemCount; i++)
	{
		char *val = NULL;
		char *pval = NULL;
		char plItem[MAX_PATH] = {0};
		val = GetItemFromDefaultPlaylist(i);
		pval = strrchr(val, '\\');
		if (pval == NULL)
		{
			pval = val;
		}
		else
		{
			pval++;
		}
		sprintf(plItem, "%03d.%s", i+1, pval);
		SendMessage(playlistLB, LB_INSERTSTRING, (-1), plItem);
	}
	
}

void TimeEventProc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2)
{
	long long curpos = 0;
	long long stoppos = 0;
	int curtime = 0;
	int stoptime = 0;
	char *p = NULL;
	char *szLryic = NULL;
	char szTime[1024] = {0};
	double rate = 0.0;

	mpxGetPositions(&curpos, &stoppos);

	if (curpos == stoppos)
	{
		// 播放下一曲
		WCHAR songpath[MAX_PATH] = {0};
		char lyricpath[MAX_PATH] = {0};
		char *p;

		playIndex++;
		if (playIndex > GetDefaultPlaylistTotalItem())
		{
			playIndex = 0;
		}

		p = GetItemFromDefaultPlaylist(playIndex);
		if (p == NULL)
		{
			return;
		}
		MakeLyricPathFromSongPath(p, lyricpath);
		LyricDestroy();
		LyricInit(lyricpath);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
		mpxPlayFile(songpath);
		status = C_BTN_PLAY;
		SetWindowText(btnPlay, "pause");
		SetWindowText(g_hWnd, p);
	}

	curtime = (int)(curpos / (double)10000000);
	stoptime = (int)(stoppos / (double)10000000);
	sprintf(szTime, "%02d:%02d->%02d:%02d", curtime/60, curtime%60, stoptime/60, stoptime%60);
	SendMessage(timeStatic, WM_SETTEXT, 0, szTime);

	szLryic = GetLyricByStartTime(curtime);

	if (szLryic != NULL)
	{
		SendMessage(lyricStatic, WM_SETTEXT, 0, szLryic);
	}
}

void BeginTimeEvent()
{

	if((g_timeId = timeSetEvent(500, 1,(LPTIMECALLBACK)TimeEventProc,(DWORD_PTR)0,TIME_PERIODIC)) == 0)
	{
		printf("time set event error!\n");
	}
}

void EndTimeEvent()
{
	if (g_timeId != 0)
	{
		timeKillEvent(g_timeId);
		g_timeId = 0;
	}
}

char *MakeLyricPathFromSongPath(char *songPath, char *lyricPath)
{
	char buf[MAX_PATH] = {0};
	char *p = buf;
	strcpy(buf, songPath);
	p = strrchr(p, '.');
	p++;
	memcpy(p, "lrc", 3);
	*(p+3) = '\0';
	strcpy(lyricPath, buf);
	return lyricPath;
}

void CreateLyricStatic(HWND hWnd)
{
	RECT rect;

	GetWindowRect(hWnd, &rect);

	lyricStatic = CreateWindowEx(WS_EX_ACCEPTFILES, WC_STATIC, "lyric", 
		WS_VISIBLE | WS_CHILD|WS_VSCROLL|LBS_NOTIFY|SS_CENTER, 400, 70, 380, rect.bottom - rect.top - 120, hWnd, C_LYRIC_STATIC, g_hInst, NULL);
}

void CreateTimeStatic(HWND hWnd)
{
	timeStatic = CreateWindowEx(WS_EX_ACCEPTFILES, WC_STATIC, "00:00->00:00", 
		WS_VISIBLE | WS_CHILD|LBS_NOTIFY|SS_CENTER, 240, 0, 100, 30, hWnd, C_TIME_STATIC, g_hInst, NULL);
}


int AddFile(HWND hWnd)
{
	OPENFILENAME fn;
	char    filefilter[] =
		"All Supported files\0*.mp1;*.mp2;*.mp3;*.m3u;*.ogg;*.pls;*.wav\0MPEG audio files (*.mp1;*.mp2;*.mp3)\0*.mp1;*.mp2;*.mp3\0Vorbis files (*.ogg)\0Playlist files (*.m3u;*.pls)\0*.m3u;*.pls\0WAV files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	BOOL    retVal = FALSE;
	char    initialfilename[MAX_PATH * 100] = "";
	fn.lStructSize = sizeof(OPENFILENAME);
	fn.hwndOwner = hWnd;
	fn.hInstance = NULL;
	fn.lpstrFilter = filefilter;
	fn.lpstrCustomFilter = NULL;
	fn.nMaxCustFilter = 0;
	fn.nFilterIndex = 0;
	fn.lpstrFile = initialfilename;
	fn.nMaxFile = MAX_PATH * 200;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrInitialDir = "./";
	fn.lpstrTitle = NULL;
	fn.Flags =
		OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_EXPLORER |
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	fn.nFileOffset = 0;
	fn.nFileExtension = 0;
	fn.lpstrDefExt = NULL;
	fn.lCustData = 0;
	fn.lpfnHook = NULL;
	fn.lpTemplateName = NULL;

	retVal = GetOpenFileName(&fn);

	if (retVal != FALSE)
	{
		char path_buffer[MAX_PATH];
		char *pval = NULL;
		strcpy(path_buffer, fn.lpstrFile);
		
		pval = strrchr(path_buffer, '\\');
		if (pval == NULL)
		{
			pval = path_buffer;
		}
		else
		{
			pval++;
		}
		SendMessage(playlistLB, LB_INSERTSTRING, (-1), pval);
		DefaultPlaylistAddItem(path_buffer);
	}
	return 0;
}

int AddDirFile(HWND hWnd)
{
	BROWSEINFO browseinfo;
	LPITEMIDLIST itemlist;
	int     image = 0;
	char    directorychoice[MAX_PATH];
	char    fullpath[MAX_PATH];
	HANDLE  found;
	WIN32_FIND_DATA finddata;
	char    pathbuf2[MAX_PATH];
	char	dirBuf[MAX_PATH];

	browseinfo.hwndOwner = hWnd;
	browseinfo.pidlRoot = NULL;
	browseinfo.pszDisplayName = directorychoice;
	browseinfo.lpszTitle = "Choose a directory to add";
	browseinfo.ulFlags = BIF_EDITBOX;
	browseinfo.lpfn = NULL;
	browseinfo.lParam = 0;
	browseinfo.iImage = image;

	itemlist = SHBrowseForFolder(&browseinfo);
	if (itemlist == NULL)
	{
		return 1;
	}

	SHGetPathFromIDList(itemlist,dirBuf);

	if (dirBuf[strlen(dirBuf) - 1] == '\\'
		&& strcmp(dirBuf, "\\") != 0) dirBuf[strlen(dirBuf) - 1] =
		'\0';

	strcpy(fullpath, dirBuf);

	if (strcmp(fullpath, "\\") == 0)
		strcat(fullpath, ".\\*.mp3");
	else
		strcat(fullpath, "\\*.mp3");

	found = FindFirstFile(fullpath, &finddata);
	do {
		char    somepath[MAX_PATH];
		strcpy(somepath, dirBuf);
		if (strcmp(somepath, "\\") == 0)
			strcpy(somepath, "\\.");
		sprintf(pathbuf2, "%s\\%s", somepath, finddata.cFileName);

		if ((finddata.cFileName[0] != '.' 
			&& finddata.cFileName[1] != 0)
			&& (finddata.cFileName[0] != '.'
			&& finddata.cFileName[1] != '.'
			&& finddata.cFileName[2] != 0))
		{
			char *pval = NULL;
			pval = strrchr(pathbuf2, '\\');
			if (pval == NULL)
			{
				pval = pathbuf2;
			}
			else
			{
				pval++;
			}
			SendMessage(playlistLB, LB_INSERTSTRING, (-1), pval);
			DefaultPlaylistAddItem(pathbuf2);
		}
	}
	while (FindNextFile(found, &finddata));
	FindClose(found);

	return 0;
}

void CreatePlayListBtn(HWND hWnd)
{
	RECT rect;
	int yPos = 
	GetWindowRect(hWnd, &rect);
	yPos = 40;
	btnPLAdd = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "add", WS_VISIBLE|WS_CHILD, 0, yPos, 50, 20, hWnd, C_PL_ADD, g_hInst, NULL);
	btnPLAddDir = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "dir", WS_VISIBLE|WS_CHILD, 60, yPos, 50, 20, hWnd, C_PL_ADDDIR, g_hInst, NULL);
	btnPLDel = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "del", WS_VISIBLE|WS_CHILD, 120, yPos, 50, 20, hWnd, C_PL_DEL, g_hInst, NULL);
	btnPLSave = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "save", WS_VISIBLE|WS_CHILD, 180, yPos, 50, 20, hWnd, C_PL_SAVE, g_hInst, NULL);
}

int DelFile(HWND hWnd)
{
	// Get selected index.
	int lbItem = -1;
	lbItem = (int)SendMessage(playlistLB, LB_GETCURSEL, 0, 0);
	if (lbItem != -1)
	{
		DefaultPlaylistDeleteItem(lbItem);
		SendMessage(playlistLB, LB_DELETESTRING, lbItem, 0);
	}
	return 0;
}

void CreateCtrlBtn(HWND hWnd)
{
	btnPrev = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "prev", WS_VISIBLE|WS_CHILD, 0, 0, 50, 30, hWnd, C_BTN_PREV, g_hInst, NULL);
	btnPlay = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "play", WS_VISIBLE|WS_CHILD, 60, 0, 50, 30, hWnd, C_BTN_PLAY, g_hInst, NULL);
	btnStop = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "stop", WS_VISIBLE|WS_CHILD, 120, 0, 50, 30, hWnd, C_BTN_STOP, g_hInst, NULL);
	btnNext = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "next", WS_VISIBLE|WS_CHILD, 180, 0, 50, 30, hWnd, C_BTN_NEXT, g_hInst, NULL);
}
