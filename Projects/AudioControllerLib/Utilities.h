#pragma once

#include <mmdeviceapi.h>
#include <set>
#include <sstream>

#ifdef NDEBUG
#undef NDEBUG
#include <cassert>
#define NDEBUG  // NOLINT(clang-diagnostic-unused-macros)
#else
#include <cassert>
#endif

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

inline std::set<std::wstring> Split(const std::wstring & s, const wchar_t delimiter)
{
    std::set<std::wstring> result;
    std::wstringstream ss(s);
    std::wstring item;

    while (getline(ss, item, delimiter))
    {
        result.insert(item);
    }

    return result;
}

inline std::wstring Merge(const std::set<std::wstring> & st, const wchar_t delimiter)
{
    size_t i = 0;
    std::wstring result;
    for (const auto & s : st)
    {
        result += s;
        if (++i < st.size())
        {
            result += delimiter;
        }
    }
    return result;
}
