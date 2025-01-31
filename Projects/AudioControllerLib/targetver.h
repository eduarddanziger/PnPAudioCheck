#pragma once

#include <sdkddkver.h>

#undef _WIN32_WINNT                                         // NOLINT(clang-diagnostic-reserved-macro-identifier)
#define _WIN32_WINNT   _WIN32_WINNT_WIN7                    // NOLINT(clang-diagnostic-reserved-macro-identifier)
