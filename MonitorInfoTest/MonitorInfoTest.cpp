#include <MonitorInfoFetcher.h>
#include <MonitorInfoProvider.h>
#include <WindowsReg.h>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

ostream& operator<<(ostream& out, const MonitorInfo& info)
{
    out << "Is Dell Monitor? " << std::boolalpha << info.IsDellMonitor << endl;
    if (info.ModelName)
    {
        out << "ModelName =  " << *info.ModelName << endl;
    }
    if (info.Serial)
    {
        out << "Serial = " << *info.Serial << endl;
    }
    if (info.ServiceTag)
    {
        out << "ServiceTag = " << *info.ServiceTag << endl;
    }
    if (info.VendorSpecificData)
    {
        out << "VendorSpecificData = " << *info.VendorSpecificData << endl;
    }
    if (info.DriverVersion)
    {
        out << "DriverVersion = " << *info.DriverVersion << endl;
    }
    return out;
}

std::string bin2hex(const unsigned char* bin, size_t len)
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
    // Get Monitor Information
    MonitorInfoProvider provider(std::make_unique<MonitorInfoFetcher>());
    auto monitors = provider.GetInformation();

    cout << "Monitor count = " << monitors.size() << endl;

    for (const auto& info : monitors)
    {
        cout << info << endl;
    }

    system("PAUSE");
    return 0;
}
