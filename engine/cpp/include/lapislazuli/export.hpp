#pragma once

// Shared-library visibility is opt-in. Static builds need no import/export decoration.
#if defined(_WIN32) && defined(LAPISLAZULI_SHARED)
  #if defined(LAPISLAZULI_BUILDING_LIBRARY)
    #define LAPISLAZULI_API __declspec(dllexport)
  #else
    #define LAPISLAZULI_API __declspec(dllimport)
  #endif
#elif defined(__GNUC__) && defined(LAPISLAZULI_SHARED)
  #define LAPISLAZULI_API __attribute__((visibility("default")))
#else
  #define LAPISLAZULI_API
#endif
