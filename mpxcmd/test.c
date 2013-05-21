#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <ShlObj.h>

#include "../mpx/mpx.h"
#include "playlist.h"
#include "lyric.h"

#pragma comment(lib, "mpx.lib")
#pragma comment(lib, "winmm.lib")
#pragma warning(disable:4996)

// ��������
#define MPXCMD_PROC_NAME "mpxcmd"

// �汾��
#define MPXCMD_MAJOR_VERSION 0
#define MPXCMD_MINOR_VERSION 1
#define MPXCMD_REVISION_VERSION 0
#define MPXCMD_BUILD_VERSION 0

typedef enum 
{
	CUSTOMER_QUIT = 0,
	CUSTOMER_PLAYFROMFILE,
	CUSTOMER_PLAY,
	CUSTOMER_PAUSE,
	CUSTOMER_STOP,
	CUSTOMER_PREV,
	CUSTOMER_NEXT,
	CUSTOMER_ADD,
	CUSTOMER_ADDDIR,
	CUSTOMER_SAVEPL,
	CUSTOMER_LAST
}CUSTOMER_CTROL;

int status = 0;
int i = 0;
UINT g_timeId = 0;
void BeginTimeEvent();
void EndTimeEvent();
char *MakeLyricPathFromSongPath(char *songPath, char *lyricPath);
void PrintLyric();

void print_version()
{
	printf("%s version:%d.%d.%d.%d %s\n", MPXCMD_PROC_NAME, MPXCMD_MAJOR_VERSION,
		MPXCMD_MINOR_VERSION, MPXCMD_REVISION_VERSION, MPXCMD_BUILD_VERSION, __DATE__);
}

int print_defaultplaylist()
{
	int total = 0;
	int i = 0;
	total = GetDefaultPlaylistTotalItem();
	if (total == 0)
	{
		return -1;
	}

	while (i < total)
	{
		char *p;
		p = GetItemFromDefaultPlaylist(i);
		printf("%d:%s\n", i, p);
		i++;
	}

	return 0;
}

void print_note()
{
	print_version();
	printf("%d:playfromfile\n", CUSTOMER_PLAYFROMFILE);
	printf("%d:play\t", CUSTOMER_PLAY);
	printf("%d:pause\t", CUSTOMER_PAUSE);
	printf("%d:stop\t", CUSTOMER_STOP);
	printf("%d:prev\n", CUSTOMER_PREV);
	printf("%d:next\t", CUSTOMER_NEXT);
	printf("%d:add\t", CUSTOMER_ADD);
	printf("%d:adddir\t", CUSTOMER_ADDDIR);
	printf("%d:savepl\n", CUSTOMER_SAVEPL);
	printf("%d:quit\n", CUSTOMER_QUIT);
}

int AddFile();
int AddDirFile();

void main(int argc, char **argv)
{
	int choice = 0;

	int ch;
	int ret = 0;

	ret = PlayListInit();
	if (ret == -1)
	{
		printf("init play list error\n");
		return;
	}

	mpxInit();
	print_note();
	do 
	{
		ch = getch();
		choice = ch - '1'+ CUSTOMER_PLAYFROMFILE;
		switch (choice)
		{
		case CUSTOMER_PLAYFROMFILE:
			{
				char buf[MAX_PATH]= {0};
				WCHAR songpath[MAX_PATH] = {0};
				char lyricpath[MAX_PATH] = {0};
				char *p;
				printf("\n");
				if (print_defaultplaylist() == -1)
				{
					printf("play list no item\n");
					break;
				}
				printf("input number choice");
				scanf("%d", &i);
				p = GetItemFromDefaultPlaylist(i);
				if (p == NULL)
				{
					break;
				}
				MakeLyricPathFromSongPath(p, lyricpath);
				LyricDestroy();
				LyricInit(lyricpath);
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
				system("cls");
				printf("playing:%s\n", p);
				print_note();
				EndTimeEvent();
				mpxPlayFile(songpath);
				BeginTimeEvent();
				//PrintLyric();
				status = 1;
			}
			break;
		case CUSTOMER_PLAY:
			{
				mpxPlay();
				BeginTimeEvent();
				status = 1;
			}
			break;
		case CUSTOMER_STOP:
			{
				mpxStop();
				EndTimeEvent();
				status = 2;
			}
			break;
		case CUSTOMER_PAUSE:
			{
				mpxPause();
				EndTimeEvent();
				status = 3;
			}
			break;
		case CUSTOMER_PREV:
			{
				WCHAR songpath[MAX_PATH] = {0};
				char lyricpath[MAX_PATH] = {0};
				char *p;
				
				printf("\n");
				if (i > 0)
				{
					i--;
				}
				p = GetItemFromDefaultPlaylist(i);
				if (p == NULL)
				{
					break;
				}
				MakeLyricPathFromSongPath(p, lyricpath);
				LyricDestroy();
				LyricInit(lyricpath);
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
				system("cls");
				printf("playing:%s\n", p);
				print_note();
				EndTimeEvent();
				mpxPlayFile(songpath);
				BeginTimeEvent();
				//PrintLyric();
				status = 1;
			}
			break;
		case CUSTOMER_NEXT:
			{
				WCHAR songpath[MAX_PATH] = {0};
				char lyricpath[MAX_PATH] = {0};
				char *p;

				printf("\n");
				i++;
				p = GetItemFromDefaultPlaylist(i);
				if (p == NULL)
				{
					break;
				}
				MakeLyricPathFromSongPath(p, lyricpath);
				LyricDestroy();
				LyricInit(lyricpath);
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
				system("cls");
				printf("playing:%s\n", p);
				print_note();
				EndTimeEvent();
				mpxPlayFile(songpath);
				BeginTimeEvent();
				//PrintLyric();
				status = 1;
			}
			break;
		case CUSTOMER_ADD:
			{
				AddFile();
			}
			break;
		case CUSTOMER_ADDDIR:
			{
				AddDirFile();
			}
			break;
		case CUSTOMER_SAVEPL:
			{
				DefaultPlaylistSave();
				printf("save play list\n");
			}
			break;
		case CUSTOMER_QUIT:
			{
				mpxDestroy();
				return;
			}
			break;
		default:
			break;
		}
	} while (1);
}

int AddFile()
{
	OPENFILENAME fn;
	char    filefilter[] =
		"All Supported files\0*.mp1;*.mp2;*.mp3;*.m3u;*.ogg;*.pls;*.wav\0MPEG audio files (*.mp1;*.mp2;*.mp3)\0*.mp1;*.mp2;*.mp3\0Vorbis files (*.ogg)\0Playlist files (*.m3u;*.pls)\0*.m3u;*.pls\0WAV files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	BOOL    retVal = FALSE;
	char    initialfilename[MAX_PATH * 100] = "";
	fn.lStructSize = sizeof(OPENFILENAME);
	fn.hwndOwner = GetConsoleWindow();
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
		strcpy(path_buffer, fn.lpstrFile);
		DefaultPlaylistAddItem(path_buffer);
	}
	return 0;
}

int AddDirFile()
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

	browseinfo.hwndOwner = GetConsoleWindow();
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
			DefaultPlaylistAddItem(pathbuf2);
		}
	}
	while (FindNextFile(found, &finddata));
	FindClose(found);

	return 0;
}

void TimeEventProc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2)
{
	long long curpos = 0;
	long long stoppos = 0;
	int curtime;
	int stoptime;
	COORD pos = {1,7};
	HANDLE hOut = NULL;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	char *p = NULL;

	mpxGetPositions(&curpos, &stoppos);

	if (curpos == stoppos)
	{
		// ������һ��
		WCHAR songpath[MAX_PATH] = {0};
		char lyricpath[MAX_PATH] = {0};
		char *p;

		printf("\n");
		i++;
		p = GetItemFromDefaultPlaylist(i);
		if (p == NULL)
		{
			return;
		}
		MakeLyricPathFromSongPath(p, lyricpath);
		LyricDestroy();
		LyricInit(lyricpath);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
		system("cls");
		printf("playing:%s\n", p);
		mpxPlayFile(songpath);
		status = 1;
		print_note();
	}


	hOut = GetStdHandle(STD_OUTPUT_HANDLE); // ��ȡ��׼����豸���
	if (!GetConsoleScreenBufferInfo(hOut, &csbi))
	{
		return;
	}
	SetConsoleCursorPosition(hOut, pos);
	curtime = (int)(curpos / (double)10000000);
	stoptime = (int)(stoppos / (double)10000000);
	printf("%02d:%02d->%02d:%02d", curtime/60, curtime%60, stoptime/60, stoptime%60);
	
	p = GetLyricByStartTime(curtime);

	if (p != NULL)
	{
		int i = 0;
		char buf[128] = {0};
		
		memset(buf, ' ', 127);
		pos.Y += 1;
		SetConsoleCursorPosition(hOut, pos);
		printf("%s", buf);
		pos.Y += 1;
		SetConsoleCursorPosition(hOut, pos);
		printf("%s", buf);
		pos.Y += 1;
		SetConsoleCursorPosition(hOut, pos);
		printf("%s", buf);
		pos.Y -= 2;
		SetConsoleCursorPosition(hOut, pos);
		printf("%s", p);
		free(p);
	}
	
	SetConsoleCursorPosition(hOut, csbi.dwCursorPosition);
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

void PrintLyric()
{
	HANDLE h;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD curWritePos = {1,10};
	int i = 0;
	char *szLyric;
	h = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!GetConsoleScreenBufferInfo(h, &csbi))
	{
		return;
	}

	while ((szLyric = GetLyricByPos(i)) != NULL)
	{
		WCHAR wbuf[256] = {0};
		WCHAR *wp = szLyric;
		char buf[256] = {0};
		mbstowcs(wbuf, szLyric, strlen(szLyric));
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, wp, wcslen(szLyric), buf, 256, NULL, NULL);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szLyric, strlen(szLyric), wbuf, 256);
		SetConsoleCursorPosition(h, curWritePos);
		printf("%s", szLyric);
		curWritePos.Y++;
		i++;
	}

	SetConsoleCursorPosition(h, csbi.dwCursorPosition);
}
