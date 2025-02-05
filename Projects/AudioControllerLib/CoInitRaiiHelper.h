#pragma once

#include <combaseapi.h>

namespace ed {
class CoInitRaiiHelper {
public:
    explicit CoInitRaiiHelper(bool apartment = false) : coInitialized_(false)
    {
        // Attempt to init COM as STA
        if (const HRESULT ciResult =
                CoInitializeEx(
                    nullptr
                    , (apartment ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED) | COINIT_DISABLE_OLE1DDE
                )
            ; SUCCEEDED(ciResult)
        )
            coInitialized_ = true; // COM initialized or already initialized.
    }

    ~CoInitRaiiHelper()
    {
        if (coInitialized_ == true)
            CoUninitialize();
    }

    CoInitRaiiHelper(const CoInitRaiiHelper &) = delete;
    CoInitRaiiHelper & operator=(const CoInitRaiiHelper &) = delete;
    CoInitRaiiHelper(CoInitRaiiHelper &&) = delete;
    CoInitRaiiHelper & operator=(CoInitRaiiHelper &&) = delete;

private:
    bool coInitialized_;
};
}
