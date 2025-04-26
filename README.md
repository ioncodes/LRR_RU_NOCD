# Lego Rock Raiders NO-CD Crack (Russian Version)
Apparently the Russian version of Lego Rock Raiders is slightly different compared to the other versions (see below). I ended up writing this NO-CD crack as the group called "Research Realm" that aims to preserve Lego's history needed help with it.

## What's the issue with this version?
1. Apparently a NO-CD crack doesn't exist for this version
2. An upgrade button doesn't work, but that's outside of this repo's scope as I couldn't get ingame to begin with

## How does CD check work?
It's fairly simple really! The game goes through the following steps:
1. Go through a bunch of driver letters (even some that can't even exist, lol)
2. Check the drive's type (it expects DRIVE_CDROM, e.g. 5)
3. Check said drive's properties, specifically the volume name (must be `ROCKRAIDERS`) and the filesystem name (must be `CDFS`)

There is a check for a CD key (using the file `cd.key`) but it looks like that it doesn't do anything with it.

## How does this patch work?
Since the game uses `d3drm.dll` I decided to implement patches inside of a proxy DLL (this project). The DLL hooks `GetDriveTypeA` and `GetVolumeInformationA` and forces overwrites the required information to reach the desired result.
The DLL must be placed in the Rock Raiders installation folder along with a copy of the legitimate `d3drm.dll` but renamed as `d3drm_ori.dll`.
AFAIK, any available version of that DLL should work but for reference, this is the hash of the one I used:

```
layle@pwn:/mnt/c/Program Files (x86)/LEGO Media/Games/Rock Raiders$ sha256sum d3drm_ori.dll
5932f4728c5220ff5e2e5c32573f7033e351005c4fe81a42a20b5025003cba7b  d3drm_ori.dll
```
