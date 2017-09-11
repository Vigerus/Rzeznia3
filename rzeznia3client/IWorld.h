#ifndef _RZEZNIA3_IWORLD_HEADER_
#define _RZEZNIA3_IWORLD_HEADER_

#include "API\include\IDrawable.h"
#include "API\include\IXMLLoadable.h"

class IWorld : virtual public VIGAFI::GRAPHICS::API::IDrawable, virtual public VIGAFI::UTILS::API::IXMLLoadable
{
public:
   virtual ~IWorld() {};

   virtual bool Init() = 0;

   virtual bool Create() = 0;

   virtual void Calculate(double dt) = 0;

   virtual void DrawUI(unsigned int flags) = 0;

   virtual void CleanUp() = 0;
};


#endif