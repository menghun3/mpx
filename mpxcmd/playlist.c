#include <windows.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PLAYLIST "default.m3u"

typedef struct listNode listNode;
struct listNode
{
	char val[MAX_PATH];
	listNode *next;
};

listNode *g_playlisthead = NULL;
listNode *g_playlisttail = NULL;
int bChange = 0;
int g_playlistitemcount = 0;
char g_defaultlistpath[MAX_PATH] = {0};

int MakePlayList()
{
	FILE *fp;
	char defaultlistpath[MAX_PATH] = {0};
	char *pplpath;
	char buf[MAX_PATH] = {0};

	GetModuleFileName(NULL, defaultlistpath, MAX_PATH);
	pplpath = strrchr(defaultlistpath, '\\');
	pplpath[1] = '\0';
	strcpy(g_defaultlistpath, defaultlistpath);
	sprintf(defaultlistpath, "%s%s", defaultlistpath, DEFAULT_PLAYLIST);
	fp = fopen(defaultlistpath, "r");
	if (fp == NULL)
	{
		printf("open %s error\n", DEFAULT_PLAYLIST);
		return -1;
	}

	do 
	{
		char *p;
		listNode *lnode;
		p = fgets(buf, MAX_PATH, fp);
		if (p == NULL)
		{
			break;
		}
		g_playlistitemcount++;
		lnode = malloc(sizeof(listNode));
		memset(lnode->val, 0, MAX_PATH);
		strncpy(lnode->val, buf, strlen(buf) - 1); //����ȡ\n
		lnode->next = NULL;
		if (g_playlisthead ==  NULL)
		{
			g_playlisthead = lnode;
			g_playlisttail = g_playlisthead;
		}
		else
		{
			g_playlisttail->next = lnode;
			g_playlisttail = lnode;
		}
		
	} while (1);
	return 0;
}

int DestroyDefaultPlayList()
{
	listNode *lnode;
	listNode *lnodenext;
	if (g_playlisthead == NULL)
	{
		return 0;
	}

	lnode = g_playlisthead;
	while (lnode)
	{
		lnodenext = lnode->next;
		free(lnode);
		lnode = lnodenext;
	}
	return 0;
}

int PlayListInit()
{
	return MakePlayList();
}

int PlayListDestroy()
{
	return DestroyDefaultPlayList();
}

// pos��0��ʼ����
char *GetItemFromDefaultPlaylist(int pos)
{
	int i = 0;
	listNode *lnode;
	if (g_playlisthead == NULL)
	{
		return NULL;
	}

	if (pos >= g_playlistitemcount)
	{
		return NULL;
	}

	lnode = g_playlisthead;
	
	while (i < pos && lnode != NULL)
	{
		i++;
		lnode = lnode->next;
	}

	if (lnode == NULL)
	{
		return NULL;
	}

	return lnode->val;
}

int GetDefaultPlaylistTotalItem()
{
	return g_playlistitemcount;
}

// ���һ��,itempath����·��
int DefaultPlaylistAddItem(char *itempath)
{
	WIN32_FIND_DATA finddata;
	HANDLE handle;

	handle = FindFirstFile(itempath, &finddata);
	if (INVALID_HANDLE_VALUE == handle)
	{
		return -1;
	}

	if (finddata.cFileName)
	{
		listNode *lnode;
		lnode = malloc(sizeof(listNode));
		memset(lnode->val, 0, MAX_PATH);
		strcpy(lnode->val, itempath);
		lnode->next = NULL;
		if (g_playlisthead ==  NULL)
		{
			g_playlisthead = lnode;
			g_playlisttail = g_playlisthead;
		}
		else
		{
			g_playlisttail->next = lnode;
			g_playlisttail = lnode;
		}
		g_playlistitemcount++;
		bChange++;
	}

	return 0;
}

// ɾ��һ��,pos��0��ʼ
int DefaultPlaylistDeleteItem(int pos)
{
	listNode *lnode;
	listNode *lnodeprev;
	int i = 0;
	if (pos >= g_playlistitemcount || g_playlisthead == NULL)
	{
		return 0;
	}

	bChange++;

	// ֻ��һ��
	if (g_playlistitemcount == 1)
	{
		free(g_playlisthead);
		g_playlisthead = NULL;
		g_playlisttail = NULL;
		g_playlistitemcount--;
		return 0;
	}
	
	// ɾ����һ��
	if (pos == 0)
	{
		lnode = g_playlisthead;
		g_playlisthead = lnode->next;
		free(lnode);
		lnode = NULL;
		return 0;
	}

	// ɾ�����һ��
	if (pos == g_playlistitemcount -1)
	{
		lnode = g_playlisthead;
		while (lnode->next->next)
		{
			lnode = lnode->next;
		}

		g_playlisttail = lnode;
		lnode = lnode->next;
		g_playlisttail->next = NULL;
		free(lnode);
		lnode = NULL;
		return 0;
	}

	// ɾ���м�һ��
	lnodeprev = g_playlisthead;
	while (i < pos - 1)
	{
		lnodeprev = lnodeprev->next;
		i++;
	}

	lnode = lnodeprev->next;

	free(lnode);
	lnode = NULL;
	lnodeprev->next = NULL;
	return 0;
}

int DefaultPlaylistSave()
{
	FILE *fp;
	listNode *lnode;
	char plpath[MAX_PATH] = {0};

	if (bChange == 0 || g_playlistitemcount == 0 || g_playlisthead == NULL)
	{
		return 0;
	}

	sprintf(plpath, "%s%s", g_defaultlistpath, DEFAULT_PLAYLIST);
	fp = fopen(plpath, "w+");
	if (fp == NULL)
	{
		printf("open %s error\n", DEFAULT_PLAYLIST);
		return -1;
	}

	lnode = g_playlisthead;
	while (lnode)
	{
		fputs(lnode->val, fp);
		//fwrite(lnode->val, 1, strlen(lnode->val), fp);
		fputs("\n", fp);
		lnode = lnode->next;
		
	}
	fflush(fp);
	fclose(fp);
	return 0;
}
