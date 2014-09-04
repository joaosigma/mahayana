#pragma once

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN

	#define WINVER 0x0601
	#define _WIN32_WINNT 0x0601
	#define NTDDI_VERSION 0x06010000

	#define NOGDICAPMASKS
	#define NOMENUS
	#define NOICONS
	#define NOKEYSTATES
	#define NOSYSCOMMANDS
	#define NOSHOWWINDOW
	#define NOCTLMGR
	#define NOMEMMGR
	#define NOOPENFILE
	#define NOWH
	#define NODEFERWINDOWPOS

#else
	
#endif