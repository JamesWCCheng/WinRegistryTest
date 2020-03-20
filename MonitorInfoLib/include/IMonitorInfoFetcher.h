#pragma once

#include <vector>
#include "MonitorInfo.h"

class IMonitorInfoFetcher
{
public:
    virtual ~IMonitorInfoFetcher() = default;
    virtual std::vector<MonitorInfo> GetInformation() = 0;
};