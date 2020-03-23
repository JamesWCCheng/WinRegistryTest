#pragma once

#include <optional>
#include <string>

struct MonitorInfo
{
    uint32_t Index = 0;
    bool IsDellMonitor = false;
    std::optional<std::string> ModelName;
    std::optional<std::string> Serial;
    std::optional<std::string> ServiceTag;
    std::optional<std::string> VendorSpecificData;
    std::optional<std::string> DriverVersion;
};