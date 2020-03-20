#include "pch.h"
#include "MonitorInfoFetcher.h"
#include "WindowsReg.h"

#include <algorithm>

std::vector<MonitorInfo> MonitorInfoFetcher::GetInformation()
{
    // Step1: Get the current monitor count by registry.
    WindowsReg reg;
    reg.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\monitor\\Enum");
    uint32_t count = 0;
    auto rv = reg.ReadIntValue(L"Count", count);

    if (!rv)
    {
        return {};
    }
    // Step2: Allocate the Monitor info as returning result.
    std::vector<MonitorInfo> result;
    result.resize(count);

    // Step3: Fill the information into result.
    uint32_t index = 0;
    std::for_each(
        std::begin(result),
        std::end(result),
        [&index](MonitorInfo& info)
        {
            info.Index = index;
            info.DriverVersion = "123.456";
            index++;
        }
    );

    return result;
}