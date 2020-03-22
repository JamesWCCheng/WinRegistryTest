#include "pch.h"
#include "MonitorInfoFetcher.h"
#include "WindowsReg.h"

#include <algorithm>
#include <array>
#include <sstream>

namespace detail
{
std::vector<std::byte> GetEDIDByIndex(uint32_t index)
{
    WindowsReg reg;
    auto rv = reg.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\monitor\\Enum");
    if (!rv)
    {
        return {};
    }
    std::wstring identifier;
    rv = reg.ReadStringValue(std::to_wstring(index).data(), identifier);
    if (!rv)
    {
        return {};
    }
    std::wstringstream edidPath;
    edidPath << L"SYSTEM\\CurrentControlSet\\Enum\\" << identifier << L"\\Device Parameters";
    rv = reg.Open(HKEY_LOCAL_MACHINE, edidPath.str().c_str());
    if (!rv)
    {
        return {};
    }
    std::vector<std::byte> edid;
    rv = reg.ReadBinaryValue(L"EDID", edid);

    return rv ? edid : std::vector<std::byte>{};
}
} // namespace detail
#include <iostream>
using namespace std;
std::vector<MonitorInfo> MonitorInfoFetcher::GetInformation()
{
    // Step1: Get the current monitor count by registry.
    WindowsReg reg;
    reg.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\monitor\\Enum");
    uint32_t count = 0;
    auto rv = reg.ReadIntValue(L"Count", count);

    if (!rv || count == 0)
    {
        return {};
    }
    // Step2: Allocate the Monitor info as returning result.
    std::vector<MonitorInfo> result;
    result.resize(count);

    // Step3: Fill the information into result.
    uint32_t index = 0;
    std::for_each(std::begin(result), std::end(result), [&index](MonitorInfo& info) {
        info.Index = index;
        info.DriverVersion = "123.456";
        auto edid = detail::GetEDIDByIndex(index);
        std::array<std::byte, 5> Tag = {static_cast<std::byte>(0x00), static_cast<std::byte>(0x00),
                                        static_cast<std::byte>(0x00), static_cast<std::byte>(0xFF),
                                        static_cast<std::byte>(0x00)};
        constexpr int BlockSize = 0x12;
        constexpr int NumberOfBlock = 4;
        constexpr int StartOffset = 0x36;
        constexpr int DataLen = 13;

        auto descriptorBlock = std::begin(edid) + StartOffset;
        std::string modelName;
        for (auto currentBlock = descriptorBlock; currentBlock < descriptorBlock + (BlockSize * NumberOfBlock); currentBlock += BlockSize)
        {
            cout << std::distance(descriptorBlock, currentBlock) << endl;
            if (std::equal(currentBlock, currentBlock + Tag.size(), std::begin(Tag)))
            {
                // found!
                std::transform(currentBlock + Tag.size(), currentBlock + Tag.size() + DataLen, std::back_inserter(modelName),
                   [](std::byte c) { return static_cast<std::string::value_type>(c); });
                break;
            }
        }
        if (!modelName.empty())
        {
            info.ModelName = std::move(modelName);
            cout << *info.ModelName << endl;
        }
        index++;
    });

    return result;
}