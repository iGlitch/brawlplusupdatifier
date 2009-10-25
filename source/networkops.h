/****************************************************************************
 * Network Operations
 * for USB Loader GX
 *
 * HTTP operations
 * Written by dhewg/bushing modified by dimok
 ****************************************************************************/
#ifndef _NETWORKOPS_H_
#define _NETWORKOPS_H_

#define NETWORKBLOCKSIZE       5*1024      //5KB

void Initialize_Network(void);
bool IsNetworkInit(void);
char * GetNetworkIP(void);
char * GetIncommingIP(void);
s32 network_request(const char * request);
s32 download_request(const char * url);
void CloseConnection();
s32 network_read(u8 *buf, u32 len);


#endif
