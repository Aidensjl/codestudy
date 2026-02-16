#pragma once

#include <string>

#if defined(_WIN32) && defined(MODULEA_SHARED)
  #if defined(MODULEA_EXPORTS)
    #define MODULEA_API __declspec(dllexport)
  #else
    #define MODULEA_API __declspec(dllimport)
  #endif
#else
  #define MODULEA_API
#endif

MODULEA_API std::string build_messageA();
