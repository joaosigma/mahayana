#pragma once
#ifndef __HPLATFORM_WIN32__
#define __HPLATFORM_WIN32__

#if defined(_WIN32)
	//we don't need most of the stuff in the windows headers
	#define WIN32_LEAN_AND_MEAN

	//include Windows XP SP2 or superior services
	#define _WIN32_WINNT 0x0501
	#define WINVER 0x0501
	#define NTDDI_VERSION NTDDI_WINXPSP1

#else
	
#endif

#endif