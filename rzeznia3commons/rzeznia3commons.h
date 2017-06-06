// rzeznia3commons.h

#pragma once

#include <guiddef.h>
#include <string>

enum packageType
{
   playerName = 1,
   chat = 2
};


namespace rzeznia3commons
{
   //single 
   struct Player
   {
      GUID guid;
      std::string name;
   };

   //for all
   struct Ludzik
   {
      GUID owner;
      std::string name;
      double x, y;

      double hp;
   };

}
