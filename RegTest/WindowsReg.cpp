#include "WindowsReg.h"
#include <cassert>

bool WindowsReg::Open(HKEY rootKey, const wchar_t* path, uint32_t mode)
{
    Close();

    constexpr DWORD DefaultOptions = 0;
    auto rv = RegOpenKeyExW(rootKey, path, DefaultOptions, static_cast<REGSAM>(mode), &m_Key);
    return rv == ERROR_SUCCESS;
}

void WindowsReg::Close()
{
    if (m_Key)
    {
        RegCloseKey(m_Key);
        m_Key = nullptr;
    }
}

namespace detail
{
    template<class T> struct DependentFalse : std::false_type {};

    template<class TString>
    bool ReadStringValueImpl(HKEY key, const typename TString::value_type* name, TString& result)
    {
        if (!key)
        {
            return false;
        }

        // Only support RRF_RT_REG_SZ.
        // We haven't support RRF_RT_REG_EXPAND_SZ & RRF_RT_REG_MULTI_SZ yet.
        typename TString::value_type data[MAX_PATH]{};
        DWORD dataSize = sizeof(data);
        LSTATUS rv = E_FAIL;
        if constexpr (std::is_same_v<TString, std::wstring>)
        {
            rv = RegGetValueW(key, nullptr, name, RRF_RT_REG_SZ, nullptr, &data, &dataSize);
        }
        else if constexpr (std::is_same_v<TString, std::string>)
        {
            rv = RegGetValueA(key, nullptr, name, RRF_RT_REG_SZ, nullptr, &data, &dataSize);
        }
        else
        {
            static_assert(DependentFalse<TString>::value, "Unexpected character type.");
        }
        if (rv != ERROR_SUCCESS)
        {
            assert(false);
            return false;
        }
        result = data;
        return true;
    }
}

bool WindowsReg::ReadStringValue(const wchar_t* name, std::wstring& result) const
{
    return detail::ReadStringValueImpl(m_Key, name, result);
}

bool WindowsReg::ReadStringValue(const char* name, std::string& result) const
{
    return detail::ReadStringValueImpl(m_Key, name, result);
}

bool WindowsReg::ReadIntValue(const  wchar_t* name, uint32_t& result) const
{
    if (!m_Key)
    {
        return false;
    }

    DWORD size = sizeof(result);
    auto rv = ::RegGetValueW(m_Key, nullptr, name, RRF_RT_REG_DWORD, nullptr, &result, &size);
    return rv == ERROR_SUCCESS;
}

bool WindowsReg::ReadInt64Value(const  wchar_t* name, uint64_t& result) const
{
    if (!m_Key)
    {
        return false;
    }

    DWORD size = sizeof(result);
    auto rv = ::RegGetValueW(m_Key, nullptr, name, RRF_RT_REG_QWORD, nullptr, &result, &size);
    return rv == ERROR_SUCCESS;
}

bool WindowsReg::ReadBinaryValue(const  wchar_t* name, std::vector<char>& result) const
{
    if (!m_Key)
    {
        return false;
    }

    DWORD size = 0;
    // We need to know the size of the blob first, then we can resize our buffer for storing the blob.
    auto rv = RegGetValueW(m_Key, nullptr, name, RRF_RT_REG_BINARY, nullptr, nullptr, &size);
    if (rv != ERROR_SUCCESS)
    {
        return false;
    }

    std::vector<char> out;
    out.resize(size);
    rv = RegGetValueW(m_Key, nullptr, name, RRF_RT_REG_BINARY, nullptr, out.data(), &size);
    result = std::move(out);

    return rv == ERROR_SUCCESS;
}
