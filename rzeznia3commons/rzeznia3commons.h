// rzeznia3commons.h

#pragma once

#include <guiddef.h>
#include <string>
#include <google/protobuf/message.h>

#ifdef RZEZNIA3COMMONS_EXPORT
#define RZEZNIA3COMMONS_API __declspec(dllexport)
#else
#define RZEZNIA3COMMONS_API __declspec(dllimport)
#endif

namespace rzeznia3commons
{
   enum messageType
   {
      MT_Unknown = 0,
      MT_player = 1,
      MT_chat = 2
   };

#define PLAYER_CONTROLS_LEFT 1
#define PLAYER_CONTROLS_RIGHT 2

   struct PlayerControls
   {
      unsigned int keys;
   };

   struct PlayerData
   {
      double x, y;
      bool direction;
      double hp;
   };

   //single 
   struct PlayerStruct
   {
      GUID guid;
      std::string name;
   };

   //for all
   struct LudzikStruct
   {
      GUID owner;
      std::string name;
      PlayerControls controls;
      PlayerData data;
   };

   struct ChatStruct
   {
      std::string from;
      std::string text;
   };

   void RZEZNIA3COMMONS_API Send_Chat(SOCKET socket, ChatStruct const& chat);

   ChatStruct RZEZNIA3COMMONS_API Read_Chat(SOCKET socket, google::protobuf::uint32 size);
}
