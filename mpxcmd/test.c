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

// 程序名称
#define MPXCMD_PROC_NAME "mpxcmd"

// 版本号
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
	CUSTOMER_PLAYLIST,
	CUSTOMER_SET,
	CUSTOMER_LAST
}CUSTOMER_CTRL;

typedef enum
{
	PLAYLIST_ADD = 1,
	PLAYLIST_ADDDIR,
	PLAYLIST_SAVEPL,
	PLAYLIST_LAST
}PLAYLIST_CTRL;

typedef enum
{
	SET_VOLUME_PLUS = 1,
	SET_VOLUME_MINUS,
	SET_LAST
}SET_CTRL;

int status = 0;
int i = 0;
UINT g_timeId = 0;
void BeginTimeEvent();
void EndTimeEvent();
char *MakeLyricPathFromSongPath(char *songPath, char *lyricPath);
void PrintLyric();
void PrintVolume();
void setVolumePlus();
void setVolumeMinus();

void print_version()
{
	printf("%s version:%d.%d.%d.%d %s\n", MPXCMD_PROC_NAME, MPXCMD_MAJOR_VERSION,
		MPXCMD_MINOR_VERSION, MPXCMD_REVISION_VERSION, MPXCMD_BUILD_VERSION, __DATE__);
}

int print_defaultplaylist()
{
	int total = 0;
	int i = 0;
	COORD pos = {0,11};
	HANDLE hOut = NULL;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
	if (!GetConsoleScreenBufferInfo(hOut, &csbi))
	{
		return 0;
	}
	
	total = GetDefaultPlaylistTotalItem();
	if (total == 0)
	{
		return -1;
	}

	SetConsoleCursorPosition(hOut, pos);

	while (i < total)
	{
		char *p;
		p = GetItemFromDefaultPlaylist(i);
		printf("%d:%s\n", i+1, p);
		i++;
	}

	SetConsoleCursorPosition(hOut, csbi.dwCursorPosition);

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
	printf("%d:playlist\t", CUSTOMER_PLAYLIST);
	printf("%d:set\t", CUSTOMER_SET);
	printf("%d:quit\n", CUSTOMER_QUIT);
}

int AddFile();
int AddDirFile();

void main(int argc, char **argv)
{
	int choice = 0;

	int ch;
	int ret = 0;
	char *p = NULL;

	ret = PlayListInit();
	if (ret == -1)
	{
		printf("init play list error\n");
		return;
	}

	//mpxInit();
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
				
				if (print_defaultplaylist() == -1)
				{
					printf("play list no item\n");
					break;
				}
				printf("input number choice");
				scanf("%d", &i);
				if (i == 0)
				{
					break;
				}
				i--;
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
				EndTimeEvent();
				mpxPlayFile(songpath);
				BeginTimeEvent();
				status = 1;
			}
			break;
		case CUSTOMER_NEXT:
			{
				WCHAR songpath[MAX_PATH] = {0};
				char lyricpath[MAX_PATH] = {0};

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
				status = 1;
			}
			break;
		case CUSTOMER_PLAYLIST:
			{
				int plch;
				system("cls");
				printf("%d:add\t", PLAYLIST_ADD);
				printf("%d:dir\t", PLAYLIST_ADDDIR);
				printf("%d:save\t", PLAYLIST_SAVEPL);
				printf("%d:exit\t", PLAYLIST_LAST);
				do 
				{
					plch = getch();
					plch = plch - '1' + PLAYLIST_ADD;
					switch (plch)
					{
					case PLAYLIST_ADD:
						{
							AddFile();
						}
						break;
					case PLAYLIST_ADDDIR:
						{
							AddDirFile();
						}
						break;
					case PLAYLIST_SAVEPL:
						{
							DefaultPlaylistSave();
							printf("save play list\n");
						}
						break;
					default:
						break;
					}
				} while (plch != PLAYLIST_LAST);
			}
			break;
		case CUSTOMER_SET:
			{
				int setch;
				system("cls");
				printf("%d:vol plus\t", SET_VOLUME_PLUS);
				printf("%d:vol minus\t", SET_VOLUME_MINUS);
				printf("%d:exit", SET_LAST);

				do 
				{
					setch = getch();
					setch = setch - '1' + SET_VOLUME_PLUS;
					switch (setch)
					{
					case SET_VOLUME_PLUS:
						{
							setVolumePlus();
						}
						break;
					case SET_VOLUME_MINUS:
						{
							setVolumeMinus();
						}
						break;
					default:
						break;
					}
				} while (setch != SET_LAST);
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
		system("cls");
		printf("playing:%s\n", p);
		print_note();
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
	COORD pos = {0,7};
	HANDLE hOut = NULL;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	char *p = NULL;

	mpxGetPositions(&curpos, &stoppos);

	if (curpos == stoppos)
	{
		// 播放下一曲
		WCHAR songpath[MAX_PATH] = {0};
		char lyricpath[MAX_PATH] = {0};
		char *p;

		printf("\n");
		i++;
		if (i > GetDefaultPlaylistTotalItem())
		{
			i = 0;
		}

		p = GetItemFromDefaultPlaylist(i);
		if (p == NULL)
		{
			return;
		}
		MakeLyricPathFromSongPath(p, lyricpath);
		LyricDestroy();
		LyricInit(lyricpath);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
		mpxPlayFile(songpath);
		status = 1;
		system("cls");
		printf("playing:%s\n", p);
		print_note();
	}


	hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
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
	PrintVolume();
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

void PrintVolume()
{
	COORD pos = {0,6};
	HANDLE hOut = NULL;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	long lVolume = -1;
	int i = 0;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
	if (!GetConsoleScreenBufferInfo(hOut, &csbi))
	{
		return;
	}
	SetConsoleCursorPosition(hOut, pos);
	while (i++ < 30)
	{
		printf(" ");
	}
	SetConsoleCursorPosition(hOut, pos);
	mpxGetVolume(&lVolume);
	printf("vol:%ld\n", lVolume);
	SetConsoleCursorPosition(hOut, csbi.dwCursorPosition);
}

void setVolumePlus()
{
	long lVolume = 0;
	mpxGetVolume(&lVolume);
	lVolume += 50;
	mpxPutVolume(lVolume);
}

void setVolumeMinus()
{
	long lVolume = 0;
	mpxGetVolume(&lVolume);
	lVolume -= 50;
	mpxPutVolume(lVolume);
}
