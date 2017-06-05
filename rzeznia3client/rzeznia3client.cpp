// rzeznia3client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <memory>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <thread> 

#include "../rzeznia3commons/winsockHelper.h"

//move it to configuration later
#define SERVER_PORT 1101

class RzezniaClient
{
public:
   RzezniaClient() {};
   virtual ~RzezniaClient() {};

   SOCKET hsock;

   bool init()
   {
      int host_port = SERVER_PORT;
      char* host_name = "127.0.0.1";

      struct sockaddr_in my_addr;

      char buffer[1024];
      int bytecount;
      int buffer_len = 0;

      int * p_int;
      int err;

      hsock = socket(AF_INET, SOCK_STREAM, 0);
      if (hsock == -1) 
      {
         printf("Error initializing socket %d\n", errno);
         return 0;
      }

      p_int = (int*)malloc(sizeof(int));
      *p_int = 1;

      if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1) ||
         (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1)) 
      {
         printf("Error setting options %d\n", errno);
         free(p_int);
         return 0;
      }
      free(p_int);

      my_addr.sin_family = AF_INET;
      my_addr.sin_port = htons(host_port);

      memset(&(my_addr.sin_zero), 0, 8);
      inet_pton(AF_INET, host_name, &my_addr.sin_addr);
      if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) 
      {
         if ((err = errno) != EINPROGRESS) 
         {
            fprintf(stderr, "Error connecting socket %d\n", errno);
            return 0;
         }
      }

      printf("connected.\n");


      int pkt = 1;

      //bytecount = send(hsock, (const char *)&pkt, 4, 0);

//       for (int i = 0; i < 10000; i++) 
//       {
//          for (int j = 0; j < 10; j++) 
//          {
// 
//             if ((bytecount = send(hsock, (void *)pkt, siz, 0)) == -1) 
//             {
//                fprintf(stderr, "Error sending data %d\n", errno);
//                return 0;
//             }
//             printf("Sent bytes %d\n", bytecount);
//             usleep(1);
//          }
//       }
//       delete pkt;

      return 1;
   }

   void run()
   {

      while (true)
      {
         Sleep(1000);
      }
   }
};

int _tmain(int argc, _TCHAR* argv[])
{
   ::CoInitialize(0);

   if (!winsock_init())
      return(-1);

   RzezniaClient client;

   if (!client.init())
      return (-1);

   client.run();

   winsock_deinit();
  
    return 0;
}

