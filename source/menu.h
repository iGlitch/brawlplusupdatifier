/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#include <ogcsys.h>

#define THREAD_SLEEP    100
//#define KBSIZE          1024.0
//#define MBSIZE          1048576.0
//#define GBSIZE          1073741824.0

void InitThreads();
void ExitGUIThreads();
void MainMenu (int menuitem);

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	MENU_SELECT_IOS,
	
};

#endif
