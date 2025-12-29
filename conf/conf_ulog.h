/**
 * Configuration file for ULOG logger
 */

// Define the project preferred log level
#ifndef ULOG_LEVEL
#  ifdef NDEBUG
#     define ULOG_LEVEL ULOG_LEVEL_INFO
#  else
#     define ULOG_LEVEL ULOG_LEVEL_DEBUG3
#  endif
#endif   // ULOG_LEVEL
