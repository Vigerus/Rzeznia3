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

#include "SDL_main.h"
#include "SDL.h"
#include "SDL_image.h"
#include <SDL_opengl.h>
#include "vigafi.h"

#include "../rzeznia3commons/winsockHelper.h"
#include "../rzeznia3commons/rzeznia3commons.h"

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>


#include "CWorld.h"

//move it to configuration later
#define SERVER_PORT 5556

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

//////////////////////////////////////////
//
//////////////////////////////////////////

int HSIZE = 1024, VSIZE = 768, DEPTH = 32, FLAGS = 0;

unsigned int flags = VIGAFI2_DRAWABLE_FLAG_PHYSX;

class RzezniaClient
{
private:
   int notquit;
public:
   RzezniaClient() { notquit = 1; };
   virtual ~RzezniaClient() {};

   std::shared_ptr<SOCKET> hsock;

   bool net_init()
   {
      char* host_port = "5556";
      char* host_name = "viger.ddns.net";
      
      struct addrinfo *result = NULL;
      DWORD dwRetval = getaddrinfo(host_name, host_port, NULL, &result);
      if (dwRetval != 0) 
      {
         printf("getaddrinfo failed with error: %d\n", dwRetval);

         return false;
      }
      struct addrinfo *ptr = result;
      bool found = false;
      struct sockaddr_in my_addr;

      while (ptr)
      {
         if (ptr->ai_family == AF_INET)
         {
            my_addr = *(struct sockaddr_in*)(ptr->ai_addr);
            found = true;
         }

         if (found)
            break;
         ptr = ptr->ai_next;
      }

      freeaddrinfo(result);

      if (!found)
         return 0;
      

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

//       my_addr.sin_family = AF_INET;
//       my_addr.sin_port = htons(host_port);
// 
//       memset(&(my_addr.sin_zero), 0, 8);
//       inet_pton(AF_INET, host_name, &my_addr.sin_addr);
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

   void main_poll(VIGAFI::MISC::API::IControls &controls)
   {
      controls.Poll();

      SDL_Event event;

      if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_QUIT, SDL_QUIT) == 1) notquit = 0;
   };

   void main_cycle(
      VIGAFI::COMMON::API::IContext *pContext
      ,IWorld *pWorld
      ,double dt
   )
   {
      if (pContext->GetControls().GiveKeyState(SDL_SCANCODE_J)) flags = VIGAFI2_DRAWABLE_FLAG_PHYSX;
      if (pContext->GetControls().GiveKeyState(SDL_SCANCODE_K)) flags = VIGAFI2_DRAWABLE_FLAG_SIMPLE;
      if (pContext->GetControls().GiveKeyState(SDL_SCANCODE_L)) flags = VIGAFI2_DRAWABLE_FLAG_FULL;

      pWorld->Calculate(dt);
   }


   void main_draw(
      VIGAFI::COMMON::API::IContext *pContext
      ,IWorld *pWorld
   )
   {
      //clear
      glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//       glMatrixMode(GL_MODELVIEW);
//       glPushMatrix();
//       pWorld->Draw(flags);
//       glPopMatrix();

      pContext->GetVideoManager().GLEnter2DMode();
      glPushMatrix();
      glTranslatef(HSIZE / 2, VSIZE / 2, 0);
      pWorld->Draw(flags);
      glPopMatrix();
      pContext->GetVideoManager().GLLeave2DMode();

      pContext->GetVideoManager().GLEnter2DMode();
      pWorld->DrawUI(flags);
      pContext->GetVideoManager().GLLeave2DMode();

      //Update screen
      pContext->GetVideoManager().SwapBuffers();
   }

   void main_cleanup(
      VIGAFI::COMMON::API::IContext *pContext
      ,IWorld *pWorld
   )
   {
      //Bring out the dead
      pWorld->CleanUp();
   }


   void run(
      VIGAFI::COMMON::API::IContext *pContext
      ,IWorld *pWorld
   )
   {
      DWORD start = 0, end = 0;
      double delta;

      end = timeGetTime();

      double maxDelta = 1.0f / 40.0f;

      while (notquit)
      {
         delta = ((double)end - (double)start) / 1000.0f;

         start = timeGetTime();

         main_poll(pContext->GetControls());

         if (delta > maxDelta)
            delta = maxDelta;

         main_cycle(pContext, pWorld, delta);

         main_draw(pContext, pWorld);

         main_cleanup(pContext, pWorld);

         Sleep(1);

         end = timeGetTime();
      }

//       while (true)
//       {
//          std::string tekst;
// 
//          std::cin >> tekst;
// 
//          rzeznia3commons::ChatStruct chat;
// 
//          chat.from = "Viger";
//          chat.text = tekst;
// 
//          rzeznia3commons::Send_Chat(*hsock, chat);
//       }
   }
};

int _tmain(int argc, _TCHAR* argv[])
{
   ::CoInitialize(0);

   //create context
   VIGAFI::COMMON::API::IContext *pContext = CreateVIGAFIContext();

   pContext->Init();
   pContext->GetVideoManager().Init(HSIZE, VSIZE, false);
   pContext->GetVideoManager().InitGL(VIGAFI::MISC::API::IVideoManager::vp_2dworld);

   pContext->GetTextureManager().LoadTextureToList("d:\\progs\\terradom\\cursor.tga", "cursor", VIGAFI::GRAPHICS::API::TexType_0);

   pContext->GetTextureManager().LoadTextureToList("d:\\progs\\terradom\\ludzik.tga", "ludzik", VIGAFI::GRAPHICS::API::TexType_0);

   //net init
   if (!winsock_init())
      return(-1);

   RzezniaClient client;

   if (!client.net_init())
      return (-1);

   SDL_ShowCursor(0);

   //create World
   IWorld *pWorld = new CWorld(pContext);

   if (!pWorld->Init())
      return (-1);

   client.run(pContext, pWorld);

   winsock_deinit();

   ::CoUninitialize();
  
    return 0;
}

