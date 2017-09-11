#include "stdafx.h"

#include "CWorld.h"


CWorld::CWorld(VIGAFI::COMMON::API::IContext *pContext) : mp_Context(pContext)
{
};

CWorld::~CWorld() {};

bool CWorld::Init()
{
   rzeznia3commons::LudzikStruct s;
   s.data.x = 0;
   s.data.y = 0;
   m_ludzik.push_back(s);

   return true;
};

bool CWorld::Create()
{
   return true;
};

void CWorld::Draw(unsigned int flags)
{
   double ludzik_size = 64;
   double ludzik_halfsize = ludzik_size / 2;
   for (unsigned int i = 0; i < m_ludzik.size(); ++i)
      mp_Context->GetTextureManager().FindTexture("ludzik")->DrawTransSimple(
         m_ludzik[i].data.x - (m_ludzik[i].data.direction ? ludzik_halfsize : -ludzik_halfsize),
         m_ludzik[i].data.y-ludzik_size,
         m_ludzik[i].data.direction ? ludzik_size : -ludzik_size,
         ludzik_size);

};

void CWorld::DrawUI(unsigned int flags)
{

   //mouse cursor last
   const TVector mouse_coords = mp_Context->GetControls().GetMouseCoords();
   mp_Context->GetTextureManager().FindTexture("cursor")->DrawTransSimple(mouse_coords.x, mouse_coords.y, 8, 8);
};

void CWorld::Calculate(double dt)
{
   if (mp_Context->GetControls().GiveKeyState(SDL_SCANCODE_A))
      m_ludzik[0].controls.keys |= PLAYER_CONTROLS_LEFT;
   else
      m_ludzik[0].controls.keys &= ~PLAYER_CONTROLS_LEFT;

   if (mp_Context->GetControls().GiveKeyState(SDL_SCANCODE_D)) 
      m_ludzik[0].controls.keys |= PLAYER_CONTROLS_RIGHT;
   else
      m_ludzik[0].controls.keys &= ~PLAYER_CONTROLS_RIGHT;

   //send it as well

   for (unsigned int idx = 0; idx < m_ludzik.size(); ++idx)
   {
      if (m_ludzik[idx].controls.keys&PLAYER_CONTROLS_LEFT)
      {
         m_ludzik[idx].data.direction = false;
         m_ludzik[idx].data.x -= dt*100;
      }
      else if (m_ludzik[idx].controls.keys&PLAYER_CONTROLS_RIGHT)
      {
         m_ludzik[idx].data.direction = true;
         m_ludzik[idx].data.x += dt * 100;
      }


   }

};

bool CWorld::LoadFromXML(const char*filename)
{
   return true;
};

bool CWorld::LoadFromXMLNode(MSXML2::IXMLDOMElementPtr node)
{
   return true;
};

void CWorld::CleanUp()
{
};