#include "pch.h"
#include "MonitorInfoFetcher.h"
#include "WindowsReg.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <string>

constexpr size_t TagLength = 5;
using TagType = std::array<std::byte, TagLength>;

constexpr TagType ModelNameTag = {static_cast<std::byte>(0x00), static_cast<std::byte>(0x00),
                                  static_cast<std::byte>(0x00), static_cast<std::byte>(0xFC),
                                  static_cast<std::byte>(0x00)};

constexpr TagType VendorDataTag = {static_cast<std::byte>(0x00), static_cast<std::byte>(0x00),
                                   static_cast<std::byte>(0x00), static_cast<std::byte>(0xFE),
                                   static_cast<std::byte>(0x00)};

constexpr TagType SerialTag = {static_cast<std::byte>(0x00), static_cast<std::byte>(0x00), static_cast<std::byte>(0x00),
                               static_cast<std::byte>(0xFF), static_cast<std::byte>(0x00)};
namespace detail
{

std::string GetDriverVersionByIndex(uint32_t index)
{
    WindowsReg reg;
    auto rv = reg.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\monitor\\Enum");
    if (!rv)
    {
        return "";
    }
    std::wstring identifier;
    rv = reg.ReadStringValue(std::to_wstring(index).data(), identifier);
    if (!rv)
    {
        return "";
    }

    // Open HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\%s to get "Driver"
    std::wstringstream driver;
    driver << L"SYSTEM\\CurrentControlSet\\Enum\\" << identifier;
    rv = reg.Open(HKEY_LOCAL_MACHINE, driver.str().c_str());
    if (!rv)
    {
        return {};
    }
    std::wstring driverPath;
    rv = reg.ReadStringValue(L"Driver", driverPath);
    if (!rv)
    {
        return {};
    }

    std::wstringstream driverVersionPath;
    driverVersionPath << "System\\CurrentControlSet\\Control\\Class\\" << driverPath;
    rv = reg.Open(HKEY_LOCAL_MACHINE, driverVersionPath.str().c_str());
    if (!rv)
    {
        return {};
    }
    std::string driverVersion;
    rv = reg.ReadStringValue("DriverVersion", driverVersion);

    return rv ? driverVersion : "";
}

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

void Trim(std::string& str)
{
    // extrat the substring from beginning to the first '\0'
    auto p = str.find_first_of('\0');
    if (p != std::string::npos)
    {
        str.erase(p);
    }
    // extrat the substring from beginning to the first '\n'
    p = str.find_first_of('\n');
    if (p != std::string::npos)
    {
        str.erase(p);
    }

    // replace non-alphanumeric character with '.'
    std::replace_if(
        std::begin(str), std::end(str), [](std::string::value_type c) { return !std::isalnum(static_cast<uint8_t>(c)); }, '.');
};

std::string ParseTimingDescriptorBlock(const std::vector<std::byte>& edid, const TagType& tag)
{
    constexpr int BlockSize = 0x12;
    constexpr int NumberOfBlock = 4;
    constexpr int StartOffset = 0x36;
    constexpr int DataLen = 13;

    auto descriptorBlock = std::begin(edid) + StartOffset;
    if (descriptorBlock >= std::end(edid))
    {
        // target to unexpected position, treat as failure and return false instead.
        return "";
    }
    std::string output;
    for (auto currentBlock = descriptorBlock; currentBlock < descriptorBlock + (BlockSize * NumberOfBlock);
         currentBlock += BlockSize)
    {
        if (std::equal(currentBlock, currentBlock + tag.size(), std::begin(tag)))
        {
            // found!
            std::transform(currentBlock + tag.size(), currentBlock + tag.size() + DataLen, std::back_inserter(output),
                           [](std::byte c) { return static_cast<std::string::value_type>(c); });
            break;
        }
    }
    return output;
}

bool IsDellMonitor(const std::vector<std::byte>& edid)
{
    constexpr std::array<std::byte, 2> DellEISAID = {static_cast<std::byte>(0x10), static_cast<std::byte>(0xAC)};
    constexpr auto OffsetToEISA = 8;
    auto EISA = std::begin(edid) + OffsetToEISA;
    if (EISA >= std::end(edid))
    {
        // target to unexpected position, treat as failure and return false instead.
        return false;
    }
    return std::equal(EISA, EISA + DellEISAID.size(), std::begin(DellEISAID));
}

} // namespace detail

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
        auto edid = detail::GetEDIDByIndex(index);
        if (edid.empty())
        {
            // Fail to get edid, continue to next monitor.
            return;
        }
        info.IsDellMonitor = detail::IsDellMonitor(edid);

        auto modelName = detail::ParseTimingDescriptorBlock(edid, ModelNameTag);
        if (!modelName.empty())
        {
            detail::Trim(modelName);
            info.ModelName = std::move(modelName);
        }

        auto serial = detail::ParseTimingDescriptorBlock(edid, SerialTag);
        if (!serial.empty())
        {
            detail::Trim(serial);
            info.Serial = std::move(serial);
        }

        auto vendorData = detail::ParseTimingDescriptorBlock(edid, VendorDataTag);
        if (!vendorData.empty())
        {
            detail::Trim(vendorData);
            info.VendorSpecificData = std::move(vendorData);
        }

        auto driverVersion = detail::GetDriverVersionByIndex(index);
        if (!driverVersion.empty())
        {
            detail::Trim(driverVersion);
            info.DriverVersion = std::move(driverVersion);
        }

        index++;
    });

    return result;
}