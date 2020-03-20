#pragma once

#include "IMonitorInfoFetcher.h"
#include "MonitorInfo.h"
#include <vector>

class MonitorInfoFetcher final : public IMonitorInfoFetcher
{
public:
    MonitorInfoFetcher() noexcept = default;
    ~MonitorInfoFetcher() override = default;
    MonitorInfoFetcher(MonitorInfoFetcher&&) = delete;
    MonitorInfoFetcher(const MonitorInfoFetcher&) = delete;
    MonitorInfoFetcher& operator=(const MonitorInfoFetcher&) = delete;
    MonitorInfoFetcher& operator=(MonitorInfoFetcher&&) = delete;

    std::vector<MonitorInfo> GetInformation() override;
};
