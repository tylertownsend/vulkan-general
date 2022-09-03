#pragma once

#ifdef __linux__
  #define ENGINE_API 1
#else
  #error Engine only supports linux!
#endif