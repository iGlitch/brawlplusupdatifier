/****************************************************************************
 * Network Operations
 * for USB Loader GX
 *
 * HTTP operations
 * Written by dhewg/bushing modified by dimok
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/machine/processor.h>


#include "http.h"


#define PORT            4299

/*** Incomming filesize ***/
u32 infilesize = 0;
u32 uncfilesize = 0;

bool updateavailable = false;
s32 connection;
static s32 socket;
//static bool updatechecked = false;
bool networkinitialized = false;
//static bool checkincomming = false;
static bool waitforanswer = false;
static char IP[16];
char incommingIP[50];
char wiiloadVersion[2];

//static lwp_t networkthread = LWP_THREAD_NULL;
//static bool networkHalt = true;

/****************************************************************************
 * Initialize_Network
 ***************************************************************************/
void Initialize_Network(void) {

    if (networkinitialized) return;

    s32 result;

    result = if_config(IP, NULL, NULL, true);

    if (result < 0) {
        networkinitialized = false;
        return;
    } else {
        networkinitialized = true;
        return;
    }
}

/****************************************************************************
 * Check if network was initialised
 ***************************************************************************/
bool IsNetworkInit(void) {
    return networkinitialized;
}

/****************************************************************************
 * Get network IP
 ***************************************************************************/
char * GetNetworkIP(void) {
    return IP;
}

/****************************************************************************
 * Get incomming IP
 ***************************************************************************/
char * GetIncommingIP(void) {
    return incommingIP;
}

/****************************************************************************
 * Get network IP
 ***************************************************************************/

s32 network_request(const char * request) {
    char buf[1024];
    char *ptr = NULL;

    u32 cnt, size;
    s32 ret;

    /* Send request */
    ret = net_send(connection, request, strlen(request), 0);
    if (ret < 0)
        return ret;

    /* Clear buffer */
    memset(buf, 0, sizeof(buf));

    /* Read HTTP header */
    for (cnt = 0; !strstr(buf, "\r\n\r\n"); cnt++)
        if (net_recv(connection, buf + cnt, 1, 0) <= 0)
            return -20;

    /* HTTP request OK? */
    if (!strstr(buf, "HTTP/1.1 200 OK"))
        return -30;
    /* Retrieve content size */
    ptr = strstr(buf, "Content-Length:");
    if (!ptr)
        return -40;

    sscanf(ptr, "Content-Length: %u", &size);
    return size;
}

s32 network_read(u8 *buf, u32 len) {
    u32 read = 0;
    s32 ret = -1;

    /* Data to be read */
    while (read < len) {
        /* Read network data */
        ret = net_read(connection, buf + read, len - read);
        if (ret < 0)
            return ret;

        /* Read finished */
        if (!ret)
            break;

        /* Increment read variable */
        read += ret;
    }

    return read;
}

/****************************************************************************
 * Download request
 ***************************************************************************/
s32 download_request(const char * url) {


    //Check if the url starts with "http://", if not it is not considered a valid url
    if (strncmp(url, "http://", strlen("http://")) != 0) {
        return -1;
    }

    //Locate the path part of the url by searching for '/' past "http://"
    char *path = strchr(url + strlen("http://"), '/');

    //At the very least the url has to end with '/', ending with just a domain is invalid
    if (path == NULL) {
        return -2;
    }

    //Extract the domain part out of the url
    int domainlength = path - url - strlen("http://");

    if (domainlength == 0) {
        return -3;
    }

    char domain[domainlength + 1];
    strlcpy(domain, url + strlen("http://"), domainlength+1);

    connection = GetConnection(domain);
    if (connection < 0) {
        return -4;
    }

    //Form a nice request header to send to the webserver
    char header[strlen(path)+strlen(domain)+100];
    sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, domain);

    s32 filesize = network_request(header);

    return filesize;
}

void CloseConnection() {

    net_close(connection);

    if (waitforanswer) {
        net_close(socket);
        waitforanswer = false;
    }
}





