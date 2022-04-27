// instdrv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <SetupAPI.h>
#include <stdio.h>
#include <newdev.h>
#include <cfgmgr32.h>

#pragma comment(lib, "setupapi")
#pragma comment(lib, "newdev")

int Error(char const* msg, DWORD err = ::GetLastError()) {
	WCHAR text[256];
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, text, _countof(text), nullptr);
	printf("%s. %ws\n", msg, text);
	return err;
}

int wmain(int argc, const wchar_t* argv[]) {
	if (argc < 2) {
		printf("Usage: instdrv <path_to_inf_file> [hardware_id]\n");
		return 0;
	}

	auto hinf = ::SetupOpenInfFile(argv[1], nullptr, INF_STYLE_WIN4, nullptr);
	if (!hinf)
		return Error("Failed to open file");
	
	INFCONTEXT ctx;
	if (!::SetupFindFirstLine(hinf, L"manufacturer", nullptr, &ctx))
		return Error("Failed to locate manufacturer section");

	WCHAR mfg[256];
	if (!::SetupGetStringField(&ctx, 1, mfg, _countof(mfg), nullptr))
		return Error("Error parsing INF");

	GUID clsGuid;
	WCHAR className[MAX_CLASS_NAME_LEN];
	SetupDiGetINFClass(argv[1], &clsGuid, className, _countof(className), nullptr);

	HDEVINFO hDevInfo = ::SetupDiCreateDeviceInfoList(&clsGuid, nullptr);
	if (!hDevInfo)
		return Error("Failed to create device info list");

	SP_DEVINFO_DATA di{ sizeof(di) };
	if(!::SetupDiCreateDeviceInfo(hDevInfo, className, &clsGuid, nullptr, nullptr, DICD_GENERATE_ID, &di))
		return Error("Failed to create device");

	WCHAR hwid[256]{};
	if (argc < 3) {
		// find hardware ID
		WCHAR arch[16];
		if (::SetupGetStringField(&ctx, 2, arch, _countof(arch), nullptr)) {
			wcscat_s(mfg, L".");
			wcscat_s(mfg, arch);
		}
		if (::SetupFindFirstLine(hinf, mfg, nullptr, &ctx))
			::SetupGetStringField(&ctx, 2, hwid, _countof(hwid), nullptr);
	}

	auto name = argc > 2 ? argv[2] : hwid;
	if (!::SetupDiSetDeviceRegistryProperty(hDevInfo, &di, SPDRP_HARDWAREID, (const BYTE*)name, DWORD((wcslen(name) + 1) * sizeof(WCHAR))))
		return Error("Failed to set hardware ID");

	if (!::SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDevInfo, &di))
		return Error("Failed to register device");

	BOOL reboot;
	if (!::UpdateDriverForPlugAndPlayDevices(nullptr, name, argv[1], INSTALLFLAG_FORCE, &reboot))
		return Error("Failed to update driver");

	::SetupDiDestroyDeviceInfoList(hDevInfo);
	printf("Installation successful!\n");

	::SetupCloseInfFile(hinf);

	return 0;
}

