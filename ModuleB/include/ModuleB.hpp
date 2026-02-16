#pragma once

#include <string>

#if defined(_WIN32) && defined(MODULEB_SHARED)
  #if defined(MODULEB_EXPORTS)
    #define MODULEB_API __declspec(dllexport)
  #else
    #define MODULEB_API __declspec(dllimport)
  #endif
#else
  #define MODULEB_API
#endif

MODULEB_API std::string build_messageB();
