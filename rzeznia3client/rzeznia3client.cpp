// rzeznia3client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <memory>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <thread> 
#include <string>
#include <mutex>

#include "../rzeznia3commons/winsockHelper.h"
#include "../rzeznia3commons/rzeznia3commons.h"

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

//move it to configuration later
#define SERVER_PORT 1101

std::mutex mtx;

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
      mtx.unlock();
      break;
   }
}

void socketListener(std::shared_ptr<SOCKET> socket)
{
   char buffer[4];
   int bytecount = 0;

   
   while (true)
   {
      if ((bytecount = recv(*socket, buffer, 4, MSG_PEEK)) == -1)
      {
         fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
         return;
      }
      else if (bytecount == 0)
         break;
      readBody(socket, readHdr(buffer));
//       printf("got a message!\n");
//       char buf[5] = { 0 };
//       recv(*socket, buf, 4, MSG_WAITALL);
//       printf("Received: %s", buf);
   }
}

class RzezniaClient
{
public:
   RzezniaClient() {};
   virtual ~RzezniaClient() {};

   std::shared_ptr<SOCKET> hsock;

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

      hsock = std::make_shared<SOCKET>(socket(AF_INET, SOCK_STREAM, 0));
      if (*hsock == -1) 
      {
         printf("Error initializing socket %d\n", errno);
         return 0;
      }

      p_int = (int*)malloc(sizeof(int));
      *p_int = 1;

      if ((setsockopt(*hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1) ||
         (setsockopt(*hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1)) 
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
      if (connect(*hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) 
      {
         if ((err = errno) != EINPROGRESS) 
         {
            fprintf(stderr, "Error connecting socket %d\n", errno);
            return 0;
         }
      }

      printf("connected.\n");

      new std::thread(socketListener, hsock);

      //int pkt = 1;

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
         std::string tekst;

         std::cin >> tekst;

         rzeznia3commons::ChatStruct chat;

         chat.from = "Viger";
         chat.text = tekst;

         rzeznia3commons::Send_Chat(*hsock, chat);
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

