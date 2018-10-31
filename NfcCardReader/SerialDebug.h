#ifndef __SERIAL_DEBUG__
#define __SERIAL_DEBUG__

// **************************************************
// *** Serial Debugging
// **************************************************
// Uncomment to enable printing debug messages.
#define DEBUG_PRINTER Serial
#ifdef ENABLE_SERIAL_DEBUG
#define DEBUG_PRINT(...)                  \
    {                                     \
        DEBUG_PRINTER.print(__VA_ARGS__); \
    }
#define DEBUG_PRINTLN(...)                  \
    {                                       \
        DEBUG_PRINTER.println(__VA_ARGS__); \
    }
#else
#define DEBUG_PRINT(...) \
    {                    \
    }
#define DEBUG_PRINTLN(...) \
    {                      \
    }
#endif

#endif