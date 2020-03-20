#pragma once

#include <memory>
#include <vector>
#include "MonitorInfo.h"

class IMonitorInfoFetcher;

class MonitorInfoProvider final
{
public:
    MonitorInfoProvider(std::unique_ptr<IMonitorInfoFetcher> fetcher) noexcept;
    ~MonitorInfoProvider();
    MonitorInfoProvider(MonitorInfoProvider&&) = delete;
    MonitorInfoProvider(const MonitorInfoProvider&) = delete;
    MonitorInfoProvider& operator=(const MonitorInfoProvider&) = delete;
    MonitorInfoProvider& operator=(MonitorInfoProvider&&) = delete;

    std::vector<MonitorInfo> GetInformation();

private:
    std::unique_ptr<IMonitorInfoFetcher> m_Fetcher;
};