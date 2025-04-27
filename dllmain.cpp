#include <Windows.h>
#include <iostream>
#include "MinHook.h"

#pragma comment(linker, "/export:D3DRMColorGetAlpha=d3drm_ori.D3DRMColorGetAlpha,@1")
#pragma comment(linker, "/export:D3DRMColorGetBlue=d3drm_ori.D3DRMColorGetBlue,@2")
#pragma comment(linker, "/export:D3DRMColorGetGreen=d3drm_ori.D3DRMColorGetGreen,@3")
#pragma comment(linker, "/export:D3DRMColorGetRed=d3drm_ori.D3DRMColorGetRed,@4")
#pragma comment(linker, "/export:D3DRMCreateColorRGB=d3drm_ori.D3DRMCreateColorRGB,@5")
#pragma comment(linker, "/export:D3DRMCreateColorRGBA=d3drm_ori.D3DRMCreateColorRGBA,@6")
#pragma comment(linker, "/export:D3DRMMatrixFromQuaternion=d3drm_ori.D3DRMMatrixFromQuaternion,@7")
#pragma comment(linker, "/export:D3DRMQuaternionFromRotation=d3drm_ori.D3DRMQuaternionFromRotation,@8")
#pragma comment(linker, "/export:D3DRMQuaternionMultiply=d3drm_ori.D3DRMQuaternionMultiply,@9")
#pragma comment(linker, "/export:D3DRMQuaternionSlerp=d3drm_ori.D3DRMQuaternionSlerp,@10")
#pragma comment(linker, "/export:D3DRMVectorAdd=d3drm_ori.D3DRMVectorAdd,@11")
#pragma comment(linker, "/export:D3DRMVectorCrossProduct=d3drm_ori.D3DRMVectorCrossProduct,@12")
#pragma comment(linker, "/export:D3DRMVectorDotProduct=d3drm_ori.D3DRMVectorDotProduct,@13")
#pragma comment(linker, "/export:D3DRMVectorModulus=d3drm_ori.D3DRMVectorModulus,@14")
#pragma comment(linker, "/export:D3DRMVectorNormalize=d3drm_ori.D3DRMVectorNormalize,@15")
#pragma comment(linker, "/export:D3DRMVectorRandom=d3drm_ori.D3DRMVectorRandom,@16")
#pragma comment(linker, "/export:D3DRMVectorReflect=d3drm_ori.D3DRMVectorReflect,@17")
#pragma comment(linker, "/export:D3DRMVectorRotate=d3drm_ori.D3DRMVectorRotate,@18")
#pragma comment(linker, "/export:D3DRMVectorScale=d3drm_ori.D3DRMVectorScale,@19")
#pragma comment(linker, "/export:D3DRMVectorSubtract=d3drm_ori.D3DRMVectorSubtract,@20")
#pragma comment(linker, "/export:Direct3DRMCreate=d3drm_ori.Direct3DRMCreate,@21")
#pragma comment(linker, "/export:DllCanUnloadNow=d3drm_ori.DllCanUnloadNow,@22")
#pragma comment(linker, "/export:DllGetClassObject=d3drm_ori.DllGetClassObject,@23")

decltype(GetVolumeInformationA)* g_OriginalGetVolumeInformationA = nullptr;
decltype(GetDriveTypeA)* g_OriginalGetDriveTypeA = nullptr;

BOOL WINAPI GetVolumeInformationA_hk(
    LPCSTR lpRootPathName,
    LPSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
)
{
    BOOL result = g_OriginalGetVolumeInformationA(
        lpRootPathName,
        lpVolumeNameBuffer,
        nVolumeNameSize,
        lpVolumeSerialNumber,
        lpMaximumComponentLength,
        lpFileSystemFlags,
        lpFileSystemNameBuffer,
        nFileSystemNameSize
    );

	std::cout << "GetVolumeInformationA(...) -> " << result << std::endl;
	std::cout << "  lpRootPathName: " << lpRootPathName << std::endl;

	if (strcmp(lpRootPathName, "_:\\") == 0)
	{
		// Spoof the volume name and file system name, the russian version for some reason only checks this.
        // It will eventually try to read _:\\cd.key but since it never(?) does anything with the result it wont matter whether it exists or not.
		if (lpVolumeNameBuffer && nVolumeNameSize > 0)
			strncpy_s(lpVolumeNameBuffer, nVolumeNameSize, "ROCKRAIDERS", nVolumeNameSize - 1);
		if (lpFileSystemNameBuffer && nFileSystemNameSize > 0)
			strncpy_s(lpFileSystemNameBuffer, nFileSystemNameSize, "CDFS", nFileSystemNameSize - 1);

		std::cout << "  lpVolumeNameBuffer: " << (lpVolumeNameBuffer ? lpVolumeNameBuffer : "NULL") << std::endl;
		std::cout << "  lpFileSystemNameBuffer: " << (lpFileSystemNameBuffer ? lpFileSystemNameBuffer : "NULL") << std::endl;

		std::cout << "Spoofed CD volume information" << std::endl;
	}

    return result;
}

BOOL WINAPI GetDriveTypeA_hk(LPCSTR lpRootPathName)
{
	BOOL result = g_OriginalGetDriveTypeA(lpRootPathName);
	std::cout << "GetDriveTypeA(\"" << lpRootPathName << "\") -> " << result << std::endl;
        
    // The game will eventually call this but it can't ever exist, so we'll just use this
	if (strcmp(lpRootPathName, "_:\\") == 0)
	{
		std::cout << "Spoofing as CDROM drive" << std::endl;
		return DRIVE_CDROM;
	}

	return result;
}

VOID InstallHooks()
{
#ifdef _DEBUG
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

    if (MH_Initialize() != MH_OK)
        std::cout << "Failed to initialize MinHook" << std::endl;

	if (MH_CreateHookApi(
		L"kernel32.dll",
		"GetVolumeInformationA",
		GetVolumeInformationA_hk,
		reinterpret_cast<LPVOID*>(&g_OriginalGetVolumeInformationA)
	) != MH_OK)
		std::cout << "Failed to create hook for GetVolumeInformationA" << std::endl;
	if (MH_CreateHookApi(
		L"kernel32.dll",
		"GetDriveTypeA",
		GetDriveTypeA_hk,
		reinterpret_cast<LPVOID*>(&g_OriginalGetDriveTypeA)
	) != MH_OK)
		std::cout << "Failed to create hook for GetDriveTypeA" << std::endl;

	MH_EnableHook(MH_ALL_HOOKS);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		InstallHooks();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


