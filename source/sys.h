#ifndef _SYS_H_
#define _SYS_H_


void wiilight(int enable);

void Sys_Init(void);
void Sys_Reboot(void);
void Sys_Shutdown(void);
void Sys_ShutdownToIdel(void);
void Sys_ShutdownToStandby(void);
void Sys_LoadMenu(void);
void Sys_BackToLoader(void);
int Sys_IosReload(int IOS);
s32  Sys_GetCerts(signed_blob **, u32 *);
void ExitApp();

#endif
