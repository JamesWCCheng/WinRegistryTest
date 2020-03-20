#include <MonitorInfoFetcher.h>
#include <MonitorInfoProvider.h>
#include <WindowsReg.h>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;

std::string bin2hex(const char* bin, size_t len)
{
    auto forcelyunsigned = reinterpret_cast<const unsigned char*>(bin);
    std::ostringstream oss;
    for (size_t i = 0; i < len; i++)
    {
        oss << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned int>(forcelyunsigned[i]);
    }
    return oss.str();
}

int main()
{
    WindowsReg reg;
    reg.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\monitor\\Enum");
    uint32_t count = 0;
    auto rv = reg.ReadIntValue(L"Count", count);
    if (rv)
    {
        cout << "count = " << count << endl;
    }
    else
    {
        cout << "ReadIntValue gg" << endl;
    }

    std::wstring str;
    rv = reg.ReadStringValue(L"0", str);
    if (rv)
    {
        wcout << "str  = " << str << endl;
    }
    else
    {
        wcout << "ReadStringValue W gg" << endl;
    }

    std::string strA;
    rv = reg.ReadStringValue("0", strA);
    if (rv)
    {
        cout << "strA  = " << strA << endl;
    }
    else
    {
        cout << "ReadStringValue A gg" << endl;
    }

    //////////////////////////////////////
    // see HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\monitor\Enum value 0 to replace
    // DISPLAY\LGD0632\4&9c63822&0&UID265988
    WindowsReg reg3;
    reg3.Open(HKEY_LOCAL_MACHINE,
              L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\LGD0632\\4&9c63822&0&UID265988\\Device Parameters");
    std::vector<char> blob;
    rv = reg3.ReadBinaryValue(L"EDID", blob);
    if (rv)
    {
        cout << "EDID = " << bin2hex(blob.data(), blob.size()) << endl;
    }
    else
    {
        cout << "ReadBinaryValue gg" << endl;
    }

    // Get Monitor Information //
    MonitorInfoProvider provider(std::make_unique<MonitorInfoFetcher>());
    auto monitors = provider.GetInformation();

    cout << "Monitor count = " << monitors.size() << endl;

    system("PAUSE");
    return 0;
}
