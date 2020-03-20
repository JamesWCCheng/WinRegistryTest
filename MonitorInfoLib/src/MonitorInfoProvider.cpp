#include "pch.h"
#include "IMonitorInfoFetcher.h"
#include "MonitorInfoProvider.h"
#include <cassert>

MonitorInfoProvider::MonitorInfoProvider(std::unique_ptr<IMonitorInfoFetcher> fetcher) noexcept
    : m_Fetcher(std::move(fetcher))
{
    assert(m_Fetcher != nullptr);
}

MonitorInfoProvider::~MonitorInfoProvider() = default;

std::vector<MonitorInfo> MonitorInfoProvider::GetInformation()
{
    return m_Fetcher->GetInformation();
}