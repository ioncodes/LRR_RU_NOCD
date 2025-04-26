#include <Windows.h>
#include <iostream>

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

    std::cout << "GetVolumeInformationA("
		<< "lpRootPathName: " << lpRootPathName
        << ", lpVolumeNameBuffer : " << lpVolumeNameBuffer
        << ", nVolumeNameSize : " << nVolumeNameSize
        << ", lpVolumeSerialNumber : " << lpVolumeSerialNumber
        << ", lpMaximumComponentLength : " << lpMaximumComponentLength
        << ", lpFileSystemFlags : " << lpFileSystemFlags
        << ", lpFileSystemNameBuffer : " << lpFileSystemNameBuffer
        << ", nFileSystemNameSize : " << nFileSystemNameSize
		<< ") -> " << result << std::endl;

	if (strcmp(lpRootPathName, "_:\\") == 0)
	{
		std::cout << "Spoofing CD properties" << std::endl;

		// Spoof the volume name and file system name, the russian version for some reason only checks this.
        // It will eventually try to read _:\\cd.key but since it never(?) does anything with the result it wont matter whether it exists or not.
		if (lpVolumeNameBuffer && nVolumeNameSize > 0)
			strncpy_s(lpVolumeNameBuffer, nVolumeNameSize, "ROCKRAIDERS", nVolumeNameSize - 1);
		if (lpFileSystemNameBuffer && nFileSystemNameSize > 0)
			strncpy_s(lpFileSystemNameBuffer, nFileSystemNameSize, "CDFS", nFileSystemNameSize - 1);
	}

    return result;
}

BOOL WINAPI GetDriveTypeA_hk(LPCSTR lpRootPathName)
{
	BOOL result = g_OriginalGetDriveTypeA(lpRootPathName);
	std::cout << "GetDriveTypeA(" << lpRootPathName << ") -> " << result << std::endl;
        
    // The game will eventually call this but it can't ever exist, so we'll just use this
	if (strcmp(lpRootPathName, "_:\\") == 0)
	{
		std::cout << "Spoofing as CDROM drive" << std::endl;
		return DRIVE_CDROM;
	}

	return result;
}

void* CreateTrampoline(void* targetFunction, size_t stolenLength)
{
    void* trampoline = VirtualAlloc(nullptr, stolenLength + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!trampoline)
        return nullptr;

    memcpy(trampoline, targetFunction, stolenLength);

    BYTE* jumpBack = (BYTE*)trampoline + stolenLength;
    intptr_t relAddr = ((BYTE*)targetFunction + stolenLength) - (jumpBack + 5);
    jumpBack[0] = 0xE9; // JMP
    *(intptr_t*)(jumpBack + 1) = relAddr;

    return trampoline;
}

bool HookFunction(const char* moduleName, const char* functionName, void** originalFunctionOut, void* hookFunction)
{
    HMODULE hMod = GetModuleHandleA(moduleName);
    if (!hMod)
        return false;

    void* targetFunction = GetProcAddress(hMod, functionName);
        return false;

    void* trampoline = CreateTrampoline(targetFunction, 12);
    if (!trampoline)
        return false;

    if (originalFunctionOut)
        *originalFunctionOut = trampoline;

    DWORD oldProtect;
    VirtualProtect(targetFunction, 12, PAGE_EXECUTE_READWRITE, &oldProtect);

    BYTE patch[12] = { 0 };
    patch[0] = 0xE9;
    intptr_t relAddr = (BYTE*)hookFunction - (BYTE*)targetFunction - 5;
    *(intptr_t*)(patch + 1) = relAddr;
    memset(patch + 5, 0x90, 7); // NOP padding

    memcpy(targetFunction, patch, 12);

    VirtualProtect(targetFunction, 12, oldProtect, &oldProtect);

    return true;
}

VOID InstallHooks()
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	HookFunction("kernel32.dll", "GetVolumeInformationA", (void**)&g_OriginalGetVolumeInformationA, (void*)GetVolumeInformationA_hk);
	HookFunction("kernel32.dll", "GetDriveTypeA", (void**)&g_OriginalGetDriveTypeA, (void*)GetDriveTypeA_hk);
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


