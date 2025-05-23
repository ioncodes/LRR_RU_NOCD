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

using ProgressiveDecompress_t = DWORD(__stdcall*)(CHAR driveletter, __int16 value);

decltype(GetVolumeInformationA)* g_OriginalGetVolumeInformationA = nullptr;
decltype(GetDriveTypeA)* g_OriginalGetDriveTypeA = nullptr;
ProgressiveDecompress_t* g_OriginalProgressiveDecompress = nullptr;

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

    if (strcmp(lpRootPathName, "C:\\") == 0)
    {
        // Spoof the volume name and file system name, the russian version for some reason only checks this.
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

UINT WINAPI GetDriveTypeA_hk(LPCSTR lpRootPathName)
{
    UINT result = g_OriginalGetDriveTypeA(lpRootPathName);

    // The game will eventually call this but it can't ever exist, so we'll just use this
    if (strcmp(lpRootPathName, "C:\\") == 0)
    {
        std::cout << "Spoofing as CDROM drive" << std::endl;
        result = DRIVE_CDROM;
    }

    std::cout << "GetDriveTypeA(\"" << lpRootPathName << "\") -> " << result << std::endl;

    return result;
}

DWORD __stdcall ProgressiveDecompress_hk(CHAR driveletter, __int16 value)
{
    // value == 2 => 004386AA | 817D F8 1B324100 | cmp dword ptr ss:[ebp-8],legorr.41321B
    // value == 3 => .text:00479893                cmp     [ebp+var_4], 43002Fh

    DWORD result = 0;

    switch (value)
    {
    case 2:
        result = 0x0041321B;
        break;
    case 3:
        result = 0x0043002F;
        break;
    default:
        result = (DWORD)"LAYL"; // Shoudl never happen, at least not for LRR
        break;
    }

    std::cout << "ProgressiveDecompress(\'" << driveletter << "\', " << value << ") -> " << std::hex << result << std::endl;

    return result;
}

VOID NopChecksumCheck(BYTE* pAddress, size_t size)
{
    /*
        ANTIDEBUG/ANTIPATCH (rough estimation):
        for (int i = 0; i < 0x000771E4; i++) {
            checksum += *(DWORD*)(0x401000 + i);
        }
        checksum -= KNOWN_VALUE;
        [ESP] -= checksum;

        [ESP] -> points to next function call, will break if checksum isn't 0

        pattern: 2B 42 04 29 04 24 33 C0 64 89 00
        new finding: the checksums are also performed against DECO_24.dll
        universal pattern: 03 06 46 49 75 FA
    */

    DWORD oldProtect;
    VirtualProtect(pAddress, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    for (size_t i = 0; i < size; ++i)
        pAddress[i] = 0x90; // NOP
    VirtualProtect(pAddress, size, oldProtect, &oldProtect);
}

//// We don't need this since we now hook DECO_24.DLL
//LONG CALLBACK VectoredHandler(EXCEPTION_POINTERS* ExceptionInfo)
//{
//	static int hitCounter = 0;
//
//	// This exception address occurs anyways, let's just reuse it
//	if (ExceptionInfo->ExceptionRecord->ExceptionAddress == (PVOID)0x0047983A && hitCounter > 5 && hitCounter != 0xFF)
//	{
//		// Write "LAYL" to the flag, anything non-zero works
//		BYTE* pLol = reinterpret_cast<BYTE*>(0x0076D160);
//		pLol[0] = 'L';
//		pLol[1] = 'A';
//		pLol[2] = 'Y';
//		pLol[3] = 'L';
//
//		hitCounter = 0xFF; // Oracle to not trigger this again
//
//		std::cout << "Patched upgrade base flag" << std::endl;
//	}
//	else if (ExceptionInfo->ExceptionRecord->ExceptionAddress == (PVOID)0x0047983A && hitCounter != 0xFF)
//	{
//		std::cout << "Hit exception " << std::hex << ExceptionInfo->ExceptionRecord->ExceptionAddress << " [" << hitCounter << "]. Waiting..." << std::endl;
//		hitCounter++;
//	}
//
//	// There should be another handler available
//	return EXCEPTION_CONTINUE_SEARCH;
//}

VOID ApplyPatches()
{
#ifdef _DEBUG
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

    // *** NOCD CRACK ***

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

    if (MH_CreateHookApi(
        L"DECO_24.dll",
        "ProgressiveDecompress_24",
        ProgressiveDecompress_hk,
        reinterpret_cast<LPVOID*>(&g_OriginalProgressiveDecompress)
    ) != MH_OK)
        std::cout << "Failed to create hook for ProgressiveDecompress" << std::endl;

    MH_EnableHook(MH_ALL_HOOKS);

    // *** CHECKSUM PATCHES ***

    NopChecksumCheck(reinterpret_cast<BYTE*>(0x00478212), 3); // WinMain
    std::cout << "Patched checksum check (WinMain)" << std::endl;

    NopChecksumCheck(reinterpret_cast<BYTE*>(0x00479916), 3); // TimerFunc
    std::cout << "Patched checksum check (TimerFunc)" << std::endl;

    NopChecksumCheck(reinterpret_cast<BYTE*>(0x0043872B), 3); // Right before upgrading the base
    std::cout << "Patched checksum check (Base Upgrade)" << std::endl;

    NopChecksumCheck(reinterpret_cast<BYTE*>(0x004386F1), 3); // DECO_24.dll Checksum 1
    std::cout << "Patched checksum check (DECO_24.DLL Checksum 1)" << std::endl;

    NopChecksumCheck(reinterpret_cast<BYTE*>(0x004798DC), 3); // DECO_24.dll Checksum 2
    std::cout << "Patched checksum check (DECO_24.DLL Checksum 2)" << std::endl;

    // *** PATCH UPGRADE BASE (OLD) ***

    //// Invert the condition for upgrading the base, IDK what the condition is but it works
    //BYTE* pAddress = reinterpret_cast<BYTE*>(0x004386A8);
    //if (pAddress)
    //{
    //	DWORD oldProtect;
    //	VirtualProtect(pAddress, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
    //	*pAddress = 0x74; // JZ
    //	VirtualProtect(pAddress, 1, oldProtect, &oldProtect);
    //	std::cout << "Patched upgrade base condition" << std::endl;
    //}

    // Add VEH to patch the upgrade base conditional after the init code at 0x004781C3
    //AddVectoredExceptionHandler(1, VectoredHandler);


}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {

    case DLL_PROCESS_ATTACH:
        ApplyPatches();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


