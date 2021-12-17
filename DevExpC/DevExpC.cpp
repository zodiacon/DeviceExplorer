// DevExpC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "DeviceManager.h"

using namespace WinSys;

std::wstring GuidToString(GUID const& guid) {
	WCHAR name[64];
	return SUCCEEDED(::StringFromGUID2(guid, name, _countof(name))) ? name : L"";
}

void DumpDeviceInterfaces() {
	for (auto& di : DeviceManager::EnumDeviceInterfaces()) {
		printf("%ws %ws (%ws) (%ws)\n", GuidToString(di.Guid).c_str(),
			di.FriendlyName.c_str(), di.DeviceFriendlyName.c_str(), di.DeviceDescription.c_str());
	}
}

void DumpDeviceClasses() {
	GUID guid;
	for (auto& name : DeviceManager::EnumDeviceClasses()) {
		if (FAILED(::CLSIDFromString(name.c_str(), &guid)))
			continue;
		auto props = DeviceManager::GetClassPropertyKeys(guid);
		for (auto& prop : props) {
		}
	}
}

void DumpHardwareProfiles() {
	for (auto& profile : DeviceManager::EnumHardwareProfiles()) {
		printf("Profile %u: %ws\n", profile.Index, profile.FriendlyName.c_str());
	}
}

int main() {
	//DumpDeviceInterfaces();
	DumpHardwareProfiles();
	DumpDeviceClasses();

	return 0;
}

