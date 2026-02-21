#ifndef RING_CORE_EXPORT_HPP_
#define RING_CORE_EXPORT_HPP_

#ifdef RING_PLATFORM_WINDOWS
    #ifdef RING_EXPORTS
        #define RING_API __declspec(dllexport)
    #else
        #define RING_API __declspec(dllimport)
    #endif
#else
    #if __has_attribute(visibility)
        #define RING_API __attribute__((visibility("default")))
    #else
        #define RING_API
    #endif
#endif

#endif // RING_CORE_EXPORT_HPP_