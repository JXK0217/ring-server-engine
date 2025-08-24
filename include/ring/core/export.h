#ifndef __RING_CORE_EXPORT_H__
#define __RING_CORE_EXPORT_H__

#ifdef RING_PLATFORM_WINDOWS
    #ifdef RING_EXPORTS
        #define RING_API __declspec(dllexport)
    #else
        #define RING_API __declspec(dllimport)
    #endif
#else
    #define RING_API __attribute__((visibility("default")))
#endif

#endif // !__RING_CORE_EXPORT_H__