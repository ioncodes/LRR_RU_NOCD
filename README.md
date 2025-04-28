# Lego Rock Raiders NOCD Crack (Russian Version)
Apparently the Russian version made by [Noviy Disk](https://kb.rockraidersunited.com/Noviy_Disk) of Lego Rock Raiders is slightly different compared to the other versions (see below). I ended up writing this NOCD crack as the group called "The Research Realm" that aims to preserve Lego's history needed help with it. The current release is considered to be fully functional and allows you the enjoy the game fully. The crack has been tested using the installer files provided by "The Research Realm" and can be found [here](https://lrr.researchrealm.net/).

## What's the issue with this version?
1. Apparently a NOCD crack doesn't exist for this version
2. An upgrade button doesn't work (and the game may sometimes crash)
   -> turns out this was caused due to a CD check performed in a separate DLL called `DECO_24.DLL`

## How does CD check work?
It's fairly simple really! The game goes through the following steps:
1. Go through a bunch of driver letters
2. Check the drive's type (it expects `DRIVE_CDROM`, e.g. 5)
3. Check said drive's properties, specifically the volume name (must be `ROCKRAIDERS`) and the filesystem name (must be `CDFS`)
4. Periodically and before upgrading the base the game calls into a "legit sounding" function called `ProgressiveDecompress` which actually reads the table of contents of the CD drive and then compares the returned value to something predetermined
5. Calculate checksum of either `LegoRR.exe` or `DECO_24.DLL`, in 5 different locations (such as before upgrading the base) to ensure the game hasn't been tampered with

The DRM looks for a file called `$(DRIVE_LETTER):\Data\cd.key` and passes the drive letter to `ProgressiveDecompress`, but since we're hooking the function in `DECO_24.DLL` to return predetermined values it will never fail, hence, the file is *not* required for successful execution using this NOCD crack.  

## How does this patch work?
Since the game uses `d3drm.dll` I decided to implement patches inside of a proxy DLL (this project). The DLL hooks `GetDriveTypeA`, `GetVolumeInformationA` and `ProgressiveDecompress` to return expected information. Additionally, the DLL also patches out all known offsets of checksum checks. 

## Installation
The DLL must be placed in the Rock Raiders installation folder along with a copy of the legitimate `d3drm.dll` but renamed as `d3drm_ori.dll`.
As far as I know, any available version of that DLL should work. For future reference, the files from my installation match the following hashes:

```
layle@pwn:/mnt/c/Program Files (x86)/Lego Rock Raiders$ sha256sum d3drm_ori.dll LegoRR.exe DECO_24.DLL
9325747683e9961beaf4a203098df159dd4e9858e9cc24f417a2fc10451528cb  d3drm_ori.dll
af9a4fa8007d19ca137ce7257f01be77b8446f7417bada5f2ffe6c0a4868d009  LegoRR.exe
82f5e4adb72090edc5a3710cd194602f02b0e31d337b8c63e2c5e29f11edded3  DECO_24.DLL
```

## Screenshot
![image](https://github.com/user-attachments/assets/83bbbc4f-f4bf-4750-a31e-f970b316d88e)
