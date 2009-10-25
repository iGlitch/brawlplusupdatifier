/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * main.cpp
 * Basic template/demonstration of libwiigui capabilities. For a
 * full-featured app using many more extensions, check out Snes9x GX.
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>

#include "FreeTypeGX.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"

#include "fatmounter.h"
#include "sys.h"
#include "gecko.h"

FreeTypeGX *fontSystem;
//Settings Settings;

int
main(int argc, char *argv[])
{
	InitGecko();//uncomment to get gecko output
	gprintf("\nGecko init");
	PAD_Init();
	Wpad_Init();
	Sys_Init();
	InitVideo(); // Initialise video
	InitAudio(); // Initialize audio
	SDCard_Init(); // Initialize file system
	USBDevice_Init(); // Initialize file system

//	Settings.Load();

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	// Initialize font system
	fontSystem = new FreeTypeGX();
	fontSystem->loadFont(NULL, font_ttf, font_ttf_size, 0);
	fontSystem->setCompatibilityMode(FTGX_COMPATIBILITY_DEFAULT_TEVOP_GX_PASSCLR | FTGX_COMPATIBILITY_DEFAULT_VTXDESC_GX_NONE);

	InitThreads();
	MainMenu(MENU_SELECT_IOS);
}
