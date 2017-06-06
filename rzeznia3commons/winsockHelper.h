#pragma once

#ifdef RZEZNIA3COMMONS_EXPORT
#define RZEZNIA3COMMONS_API __declspec(dllexport)
#else
#define RZEZNIA3COMMONS_API __declspec(dllimport)
#endif

bool RZEZNIA3COMMONS_API winsock_init();
void RZEZNIA3COMMONS_API winsock_deinit();