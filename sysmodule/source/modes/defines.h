#pragma once

// Debug UDP logging with netcat -ul 9999
#define LOGGING_ENABLED (UDP_LOGGING | FILE_LOGGING)

#if UDP_LOGGING && FILE_LOGGING
#pragma error "UDP and file logging are mutually exclusive"
#endif

#define NEEDS_FS (!defined(USB_ONLY) || FILE_LOGGING)
#define NEEDS_SOCKETS (!defined(USB_ONLY) || UDP_LOGGING)

#if LOGGING_ENABLED
	#if FILE_LOGGING
		#include <stdio.h>
		#define LogFunctionImpl(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)
	#else
		void LogFunctionImpl(const char* fmt, ...);
	#endif

	#define LOG(...) do { LogFunctionImpl(__VA_ARGS__); } while (0)
	
	// When building as a sysmodule we ignore any kind of verbose logging
	#define LOG_V(...) do {  } while (0)

	// This is also added to thread stacks so must be 0x1000-aligned	
	#define LOGGING_HEAP_BOOST 0x1000
#else
	#define LOG(...) do { } while (0)
	#define LOG_V(...) do { } while (0)
	#define LOGGING_HEAP_BOOST 0
#endif

#define R_RET_ON_FAIL(x) do { Result rc = x; if (R_FAILED(rc)) return rc; } while (0)
#define R_THROW(x) do { Result r = x; if (R_FAILED(r)) { fatalThrow(r); }  } while(0)

#define SYSDVR_CRASH_MODULEID 0x69

#define ERR_RTSP_VIDEO MAKERESULT(SYSDVR_CRASH_MODULEID, 1)
#define ERR_RTSP_AUDIO MAKERESULT(SYSDVR_CRASH_MODULEID, 2)
#define ERR_TCP_THREAD MAKERESULT(SYSDVR_CRASH_MODULEID, 3)
#define ERR_USB_THREAD MAKERESULT(SYSDVR_CRASH_MODULEID, 5)
#define ERR_USB_UNKMODE MAKERESULT(SYSDVR_CRASH_MODULEID, 19)
#define ERR_USB_OUT_OF_MEMORY MAKERESULT(SYSDVR_CRASH_MODULEID, 20)

#define ERR_IPC_INVHEADER MAKERESULT(SYSDVR_CRASH_MODULEID, 9)
#define ERR_IPC_INVSIZE MAKERESULT(SYSDVR_CRASH_MODULEID, 10)
#define ERR_IPC_INVMAGIC MAKERESULT(SYSDVR_CRASH_MODULEID, 11)
#define ERR_IPC_NOCLIENT MAKERESULT(SYSDVR_CRASH_MODULEID, 12)
#define ERR_IPC_UNKCMD MAKERESULT(SYSDVR_CRASH_MODULEID, 13)
#define ERR_IPC_INVALID_REQUEST MAKERESULT(SYSDVR_CRASH_MODULEID, 22)

#define ERR_MAIN_UNKMODE MAKERESULT(SYSDVR_CRASH_MODULEID, 14)
#define ERR_MAIN_UNKMODESET MAKERESULT(SYSDVR_CRASH_MODULEID, 15)
#define ERR_MAIN_SWITCHING MAKERESULT(SYSDVR_CRASH_MODULEID, 16)
#define ERR_MAIN_UNEXPECTED_MODE MAKERESULT(SYSDVR_CRASH_MODULEID, 22)
#define ERR_MAIN_NOTRUNNING MAKERESULT(SYSDVR_CRASH_MODULEID, 18)
#define ERR_INIT_FAILED MAKERESULT(SYSDVR_CRASH_MODULEID, 21)

#define ERR_HIPC_UNKREQ MAKERESULT(11, 403)

//This is a version for the SysDVR Config app protocol, it's not shown anywhere and not related to the major version
#define SYSDVR_VERSION 12
#define TYPE_MODE_USB 1
#define TYPE_MODE_TCP 2
#define TYPE_MODE_RTSP 4
#define TYPE_MODE_NULL 3

// This is used as NULL values for mode in the IPC code, it's not a valid mode and sysdvr will fatal if it's used
#define TYPE_MODE_INVALID 0

#define CMD_SET_USB 1
#define CMD_SET_TCP 2
#define CMD_SET_OFF 3
#define CMD_SET_RTSP 4

#define CMD_GET_VER 100
#define CMD_GET_MODE 101

// Crash the process so we can get a crash report to figure out what's going on, don't call it outside of dev tools
#define CMD_DEBUG_CRASH 102

// Options to tweak special options dynamically, uses the UserOverrides struct
#define CMD_SET_USER_OVERRIDES 103
#define CMD_GET_USER_OVERRIDES 104

typedef struct {
	// Globally enables or disables overrides, if this is false the following values are ignored in CMD_SET_USER_OVERRIDES
	// If this is the result of GET_USER_OVERRIDES the other values are valid and represent the current options either previously set by the user or the hardcoded defaults.
	char Enabled;
	// Represents how many audio reads (0x1000 bytes) are batched together before being sent to the client, default is 1
	// This is needed to reduce the amount of packets sent over the network, this is capped to the size of MaxABatching (since buffers are statically allocated)
	// More info in the comment in capture.h
	char AudioBatching;
	// Workaround for a specific condition explained in capture.c in CaptureReadVideo, default value is 5, bigger values may cause glitches and visual artifacts,
	// smaller values may increase latency or delay since we're sending more data over the channel.
	// 0 Disables this option
	char StaticDropThreshold;
	// Currently unused
	char Padding;
} UserOverrides;

_Static_assert(sizeof(UserOverrides) == 4, "UserOverrides size is wrong");