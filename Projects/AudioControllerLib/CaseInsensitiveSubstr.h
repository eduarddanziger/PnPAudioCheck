#pragma once

#include <xutility>

inline bool FindSubstrCaseInsensitive(const std::wstring & str, const std::wstring & substr)
{
    const auto it =
        std::ranges::search
        (str
         , substr
         , [](wchar_t ch1, wchar_t ch2)
         {
             return std::toupper(ch1) == std::toupper(ch2);
         }
        ).begin();
    return it != str.end();
}
