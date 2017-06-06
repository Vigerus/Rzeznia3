#include "stdafx.h"

#include <iostream>
#include <memory>
#include <winsock2.h>
#include <Ws2tcpip.h>

#include "winsockHelper.h"

bool RZEZNIA3COMMONS_API winsock_init()
{
   WORD wVersionRequested;
   WSADATA wsaData;
   int err;

   /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
   wVersionRequested = MAKEWORD(2, 2);

   err = WSAStartup(wVersionRequested, &wsaData);
   if (err != 0)
   {
      /* Tell the user that we could not find a usable */
      /* Winsock DLL.                                  */
      printf("WSAStartup failed with error: %d\n", err);
      return 0;
   }

   /* Confirm that the WinSock DLL supports 2.2.*/
   /* Note that if the DLL supports versions greater    */
   /* than 2.2 in addition to 2.2, it will still return */
   /* 2.2 in wVersion since that is the version we      */
   /* requested.                                        */

   if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
   {
      /* Tell the user that we could not find a usable */
      /* WinSock DLL.                                  */
      printf("Could not find a usable version of Winsock.dll\n");
      WSACleanup();
      return 0;
   }
   else
      printf("The Winsock 2.2 dll was found okay\n");

   return 1;
}

void RZEZNIA3COMMONS_API winsock_deinit()
{
   WSACleanup();
}