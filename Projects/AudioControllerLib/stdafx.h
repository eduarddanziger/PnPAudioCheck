// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifdef _DEBUG
// ReSharper disable once CppInconsistentNaming
#   define _CRTDBG_MAP_ALLOC
#   include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <map>
#include <vector>
#include <queue>
#include <string>

#include "Utilities.h"

#include <TimeUtils.h>
#include <PrefixStream.h>


#define TO_STREAM_LOG_WITH_TIME_PREFIX(v_stream, v_prefix) \
	ed::PrefixOStreamW poss(ed::getLocalTimeAsWideString() + (v_prefix), v_stream, true); \
	poss

#define TO_STREAM_LOG(oss) TO_STREAM_LOG_WITH_TIME_PREFIX(oss, L" : ")

#ifndef _NO_LOG_
//#define PUT_TO_STREAM_LOG(oss, inp) TO_STREAM_LOG(oss) << inp
#define PUT_TO_STREAM_LOG(oss, inp) oss << inp
#else
#define PUT_TO_STREAM_LOG(oss, inp)
#endif //_NO_LOG_

#define LOG_INFO(inp) { std::wostringstream oss; \
                PUT_TO_STREAM_LOG(oss, inp); \
                TraceIt(oss.str().c_str()); }

#define LOG_INFO_COLL(inp, coll) { std::wostringstream oss; \
                PUT_TO_STREAM_LOG(oss, inp); \
                (coll)->TraceIt(oss.str().c_str()); }
