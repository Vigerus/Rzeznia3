// rzeznia3server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <memory>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <thread>
#include <vector>
#include <sstream>

#include "../rzeznia3commons/winsockHelper.h"
#include "../rzeznia3commons/rzeznia3commons.h"

//move it to configuration later
#define SERVER_PORT 1101



void socketHandler(std::shared_ptr<SOCKET> socket, std::shared_ptr<rzeznia3commons::Player> gracz)
{
   std::shared_ptr<SOCKET> csock = socket;

   char buffer[4];
   int bytecount = 0;
   std::string output, pl;
   //log_packet logp;

   memset(buffer, '\0', 4);

   send(*csock, "dupa", 4, 0);

   while (1) 
   {
      //Peek into the socket and get the packet size
      if ((bytecount = recv(*csock,
         buffer,
         4, MSG_PEEK)) == -1) 
      {
         fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
         printf("Disconnecting %s...", gracz->name.c_str());
         return;
      }
      else if (bytecount == 0)
         break;
      std::cout << "First read byte count is " << bytecount << std::endl;
      //readBody(*csock, readHdr(buffer));
   }

   return;
}

class RzezniaServer
{
public:
   RzezniaServer()
   {
      newplayercount = 0;
   };

   virtual ~RzezniaServer() {};

   unsigned int newplayercount;

   SOCKET hsock;
   std::shared_ptr<SOCKET> csock;
   socklen_t addr_size = 0;
   sockaddr_in sadr;

   std::vector<std::shared_ptr<rzeznia3commons::Player>> m_players;
   std::vector<std::shared_ptr<rzeznia3commons::Ludzik>> m_ludziki;

   bool init()
   {
      int host_port = SERVER_PORT;
      struct sockaddr_in my_addr;

      int * p_int;

      hsock = socket(AF_INET, SOCK_STREAM, 0);
      if (hsock == INVALID_SOCKET)
      {
         printf("Error initializing socket %d\n", WSAGetLastError());
         return 0;
      }
      else
         printf("Socket created successfully.\n");

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
      my_addr.sin_addr.s_addr = INADDR_ANY;

      if (bind(hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1)
      {
         fprintf(stderr, "Error binding to socket, make sure nothing else is listening on this port %d\n", errno);
         return 0;
      }
      if (listen(hsock, 10) == -1)
      {
         fprintf(stderr, "Error listening %d\n", errno);
         return 0;
      }

      //Now lets do the server stuff

      addr_size = sizeof(sockaddr_in);


      return 1;
   }



   void run()
   {
      while (true) {
         printf("waiting for a connection\n");
         csock = std::shared_ptr<SOCKET>(new SOCKET);
         if ((*csock = accept(hsock, (sockaddr*)&sadr, &addr_size)) != -1)
         {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sadr.sin_addr), str, INET_ADDRSTRLEN);
            printf("---------------------\nReceived connection from %s\n", str);

            std::shared_ptr<rzeznia3commons::Player> nowygracz = std::make_shared<rzeznia3commons::Player>();
            HRESULT hCreateGuid = CoCreateGuid(&nowygracz->guid);
            std::ostringstream ss;
            ss << "Player" << std::dec << ++newplayercount;
            nowygracz->name = ss.str();
            
            
            //pthread_create(&thread_id, 0, &SocketHandler, (void*)csock);
            //pthread_detach(thread_id);
            new std::thread(socketHandler, csock, nowygracz);
         }
         else
         {
            fprintf(stderr, "Error accepting %d\n", errno);
         }
      }
   }
};

int _tmain(int argc, _TCHAR* argv[])
{
   ::CoInitialize(0);

   if (!winsock_init())
      return(-1);

   RzezniaServer server;

   if (!server.init())
      return (-1);

   server.run();

   winsock_deinit();

   return 0;
}

