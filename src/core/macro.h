#ifndef __RING_CORE_MACRO_H__
#define __RING_CORE_MACRO_H__

#ifdef RING_PLATFORM_WINDOWS
    #ifdef RING_BUILD_DLL
        #define RING_API __declspec(dllexport)
    #else
        #define RING_API __declspec(dllimport)
    #endif
#else
    #define RING_API
#endif

#endif // !__RING_CORE_MACRO_H__