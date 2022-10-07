#pragma once

#ifdef __linux__
  #define ENGINE_PLATFORM_LINUX
#else
  #error Engine only supports linux!
#endif