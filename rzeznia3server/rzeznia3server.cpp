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
#include <mutex>

#include "../rzeznia3commons/winsockHelper.h"
#include "../rzeznia3commons/rzeznia3commons.h"

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

//move it to configuration later
#define SERVER_PORT 5556

std::mutex mtx;

std::vector<std::pair<std::shared_ptr<rzeznia3commons::PlayerStruct>, std::shared_ptr<SOCKET>>> m_player_list;

std::pair<rzeznia3commons::messageType, google::protobuf::uint32> readHdr(char *header_buf)
{
   google::protobuf::uint32 msg_type;
   google::protobuf::uint32 size;
   google::protobuf::io::ArrayInputStream ais(header_buf, 8);
   google::protobuf::io::CodedInputStream coded_input(&ais);
   coded_input.ReadVarint32(&msg_type);
   coded_input.ReadVarint32(&size);//Decode the HDR and get the size

   switch (msg_type)
   {
      case rzeznia3commons::MT_chat:
         return std::make_pair(rzeznia3commons::MT_chat, size);
      break;
   }

   return std::make_pair(rzeznia3commons::MT_Unknown, size);
}

void readBody(std::shared_ptr<SOCKET> socket, std::pair<rzeznia3commons::messageType, int> header)
{
   switch (header.first)
   {
      case rzeznia3commons::MT_chat:
         rzeznia3commons::ChatStruct chat = rzeznia3commons::Read_Chat(*socket, header.second);
         mtx.lock();
         std::cout << chat.from << ": " << chat.text << std::endl;

         std::for_each(m_player_list.begin(), m_player_list.end(),
            [&](std::pair<std::shared_ptr<rzeznia3commons::PlayerStruct>, std::shared_ptr<SOCKET>> const &el) 
         {
            rzeznia3commons::Send_Chat(*el.second, chat);
         });
         mtx.unlock();
      break;
   }
}

void socketHandler(std::shared_ptr<SOCKET> socket, std::shared_ptr<rzeznia3commons::PlayerStruct> gracz)
{
   std::shared_ptr<SOCKET> csock = socket;

   char buffer[8];
   int bytecount = 0;
   std::string output, pl;
   //log_packet logp;

   memset(buffer, 0, 8);
  

   while (1) 
   {
      //Peek into the socket and get the packet size
      if ((bytecount = recv(*csock, buffer, 8, MSG_PEEK)) == -1) 
      {
         fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
         printf("Disconnecting %s...", gracz->name.c_str());
         return;
      }
      else if (bytecount == 0)
         break;
      //std::cout << "First read byte count is " << bytecount << std::endl;
      readBody(csock, readHdr(buffer));
   }

   return;
}

void connectionsHandler(SOCKET hsock, socklen_t addr_size, sockaddr_in sadr)
{
   std::shared_ptr<SOCKET> csock;
   unsigned int newplayercount = 0;

   while (true) {
      printf("waiting for a connection\n");
      csock = std::shared_ptr<SOCKET>(new SOCKET);
      if ((*csock = accept(hsock, (sockaddr*)&sadr, &addr_size)) != -1)
      {
         char str[INET_ADDRSTRLEN];
         inet_ntop(AF_INET, &(sadr.sin_addr), str, INET_ADDRSTRLEN);
         printf("---------------------\nReceived connection from %s\n", str);

         std::shared_ptr<rzeznia3commons::PlayerStruct> nowygracz = std::make_shared<rzeznia3commons::PlayerStruct>();
         HRESULT hCreateGuid = CoCreateGuid(&nowygracz->guid);
         std::ostringstream ss;
         ss << "Player" << std::dec << ++newplayercount;
         nowygracz->name = ss.str();

         mtx.lock();
         m_player_list.push_back(std::make_pair(nowygracz, csock));
         mtx.unlock();

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

class RzezniaServer
{
public:
   RzezniaServer()
   {
   };

   virtual ~RzezniaServer() {};

   

   SOCKET hsock;
   
   socklen_t addr_size = 0;
   sockaddr_in sadr;

   std::vector<std::shared_ptr<rzeznia3commons::PlayerStruct>> m_players;
   std::vector<std::shared_ptr<rzeznia3commons::LudzikStruct>> m_ludziki;

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

      new std::thread(connectionsHandler, hsock, addr_size, sadr);

      return 1;
   }



   void run()
   {

      while (true)
      {
         Sleep(33);
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

