#pragma once

#include "IWorld.h"
#include "API\include\IContext.h"

#include "../rzeznia3commons/rzeznia3commons.h"

#include <vector>

class CWorld : virtual public IWorld

{
public:
   CWorld(VIGAFI::COMMON::API::IContext *pContext);
   virtual ~CWorld();

   virtual bool Init();
   virtual bool Create();

   //getters
   virtual void Draw(unsigned int flags);
   virtual void DrawUI(unsigned int flags);

   virtual void Calculate(double dt);

   virtual bool LoadFromXML(const char*filename);
   virtual bool LoadFromXMLNode(MSXML2::IXMLDOMElementPtr node);

   virtual void CleanUp();
protected:
   VIGAFI::COMMON::API::IContext *mp_Context;

   std::vector<rzeznia3commons::LudzikStruct> m_ludzik;
};