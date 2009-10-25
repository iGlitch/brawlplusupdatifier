#include <stdio.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>

//#include "network/networkops.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "fatmounter.h"
#include "sys.h"


//Wiilight stuff
/*static vu32 *_wiilight_reg = (u32*)0xCD0000C0;
void wiilight(int enable) {             // Toggle wiilight (thanks Bool for wiilight source)
    u32 val = (*_wiilight_reg&~0x20);
    if(enable) val |= 0x20;
    *_wiilight_reg=val;
}*/

u8 shutdown = 0;
u8 reset = 0;

void __Sys_ResetCallback(void)
{
	reset = 1;
}

void __Sys_PowerCallback(void)
{
	shutdown = 1;
}


void Sys_Init(void)
{
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void ExitApp()
{
	ExitGUIThreads();
	StopGX();
	ShutdownAudio();

	SDCard_deInit();
	USBDevice_deInit();
}


void Sys_Reboot(void)
{
	ExitApp();
	STM_RebootSystem();
}

#define ShutdownToDefault	0
#define ShutdownToIdle		1
#define ShutdownToStandby	2

static void _Sys_Shutdown(int SHUTDOWN_MODE)
{
	ExitApp();
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Shutdown();

	if((CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE &&  SHUTDOWN_MODE != ShutdownToStandby) || SHUTDOWN_MODE == ShutdownToIdle) {
		s32 ret;

		ret = CONF_GetIdleLedMode();
		if(ret >= 0 && ret <= 2)
			STM_SetLedMode(ret);

		STM_ShutdownToIdle();
	} else {
		STM_ShutdownToStandby();
	}
}

void Sys_Shutdown(void)
{
	_Sys_Shutdown(ShutdownToDefault);
}

void Sys_ShutdownToIdel(void)
{
	_Sys_Shutdown(ShutdownToIdle);
}

void Sys_ShutdownToStandby(void)
{
	_Sys_Shutdown(ShutdownToStandby);
}

void Sys_LoadMenu(void)
{
	ExitApp();
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void Sys_BackToLoader(void)
{
	if (*((u32*) 0x80001800))
	{
		ExitApp();
		exit(0);
	}
	// Channel Version
	Sys_LoadMenu();
}
