#include "pch.h"
#include "DriverManager.h"

std::vector<DriverInfo> DriverManager::EnumKernelDrivers(bool runningOnly) {
    wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE));
    if (!hScm)
        return {};

    DWORD size = 0;
    DWORD count = 0;
    ::EnumServicesStatusEx(hScm.get(), SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_ACTIVE | (runningOnly ? 0 : SERVICE_INACTIVE),
        nullptr, 0, &size, &count, nullptr, nullptr);
    if (size == 0)
        return {};

    auto buffer = std::make_unique<BYTE[]>(size);
    if (!::EnumServicesStatusEx(hScm.get(), SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_ACTIVE | (runningOnly ? 0 : SERVICE_INACTIVE),
        buffer.get(), size, &size, &count, nullptr, nullptr))
        return {};

    std::vector<DriverInfo> drivers;
    drivers.reserve(count);
    auto info = (ENUM_SERVICE_STATUS_PROCESS*)buffer.get();
    auto svcBuffer = std::make_unique<BYTE[]>(2048);
    auto config = (QUERY_SERVICE_CONFIG*)svcBuffer.get();

    for (DWORD i = 0; i < count; i++) {
        DriverInfo di;
        auto& dinfo = info[i];
        di.Name = dinfo.lpServiceName;
        di.DisplayName = dinfo.lpDisplayName;
        auto& status = dinfo.ServiceStatusProcess;
        di.Type = (DeviceDriverType)status.dwServiceType;
        di.State = status.dwCurrentState;
        wil::unique_schandle hService(::OpenService(hScm.get(), dinfo.lpServiceName, SERVICE_QUERY_CONFIG));
        if (hService && ::QueryServiceConfig(hService.get(), config, 2048, &size)) {
            di.ImagePath = config->lpBinaryPathName;
        }

        //
        // look up KMDF details (if any)
        //
        CRegKey key;
        key.Open(HKEY_LOCAL_MACHINE, (LR"(System\CurrentControlSet\Services\)" + di.Name + LR"(\Parameters\WDF)").c_str(), KEY_READ);
        if (key) {
            di.Type |= DeviceDriverType::KMDF;
            key.QueryDWORDValue(L"WdfMajorVersion", (DWORD&)di.MajorVersion);
            key.QueryDWORDValue(L"WdfMinorVersion", (DWORD&)di.MinorVersion);
        }
        drivers.push_back(std::move(di));
    }

    return drivers;
}
