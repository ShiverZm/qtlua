/* config.h.  Manual config for MSVC.  */

//#ifndef QT_VERSION
//#warn "qt/config.h shouldn't be included for your development environment."
//#error "Please make sure the qt/ directory is removed from your build path."
//#endif

/* Visual Studio 2015 and later defines timespec */
//#if defined(QT_VERSION) && (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
//#define _TIMESPEC_DEFINED 1
//#endif


/* Default visibility */
#define DEFAULT_VISIBILITY /**/

/* Enable global message logging */
#define ENABLE_LOGGING 1

/* Uncomment to start with debug message logging enabled */
// #define ENABLE_DEBUG_LOGGING 1

/* Uncomment to enabling logging to system log */
// #define USE_SYSTEM_LOGGING_FACILITY

/* type of second poll() argument */
#define POLL_NFDS_TYPE unsigned int

#define HAVE_GETTIMEOFDAY

/* Windows/WinCE backend */
#if defined(_WIN32_WCE)
#define OS_WINCE 1
#define HAVE_MISSING_H
#else
#define OS_WINDOWS 1
#define HAVE_SIGNAL_H 1
#define HAVE_SYS_TYPES_H 1
#endif
