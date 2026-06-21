<p align="center">
  <img width="600" height="290" alt="banoipatch" src="https://github.com/user-attachments/assets/9f52505e-8053-4880-ab1d-ee6c6f249863" />
</p>


<p align="center">
  A mod to fix a large amount of issues with Dead Island (2011)
</p>

## Features 

* **Audio Issues Fixed:** Fixed the awful audio screeching by capping the audio sample rate to 48000Hz.
* **Native PS4/PS5 & Switch Controller Support:** The game now supports PS4/5 and Nintendo Switch controllers using SDL3.
* **Anisotropic Filtering:** Forced 16x anisotropic filtering, improving texture quality.
* **Removed DirectInput Controller Suppot:** Disables legacy DirectInput controller handling, reducing stuttering and improving startup times.
* **Removed .pak CRC Checks:** Allows you to edit the .pak files without the game crashing.
* **Fixed Menu Mouse Input:** The low sensitivity on menus at resolutions above 720p are now fixed.
* **Screenshot Hotkey:** Allows you to take screenshots using the F5 key, screenshots will be saved in either `Dead Island\DI\out\ScreenShots` or `Documents\deadisland\out\ScreenShots`
* **Increased FOV:** Allows you to increase or decrease the FOV using the + or - keys in game, or set it in the `BanoiPatch.ini`.
* **Fix for 32 thread CPUs:** The game has a chance to crash when using CPUs with 32 threads, it has been patched.
* **ASI Loader:** Automatically loads .asi plugins, which are just renamed DLLs
* **(Optional) Developer Menu:** Enables the developer menu without having to replace the `game_x86_rwdi.dll`.
* **(Optional) Improved Shadow Quality:** All static objects are now included in shadow maps, along with generally improved quality.
* **(Optional) Improved Draw Distance:** Vastly improved draw distance and pop in, at the cost of performance and some minor visual issues in some maps.
  
*Features marked with (Optional) can all be toggled with the `BanoiPatch.ini`*

## Install Guide

> [!NOTE]  
> Requires Dead Island: Game of the Year Edition (1.3), and does not support Dead Island: Definitive Edition.
> 
> Requires at least Windows 7 SP1 x86, though may work on older
>
> **Download** [BanoiPatch](https://github.com/seamusduncmcgrath/BanoiPatch/releases/latest/download/xinput1_3.dll)
> 
> Simply add the xinput1_3.dll file into the game folder next to `DeadIslandGame.exe`.

## Config File
Optional BanoiPatch settings can be enabled or adjusted using BanoiPatch.ini. Create this file in the game directory (next to DeadIslandGame.exe) and add the following:

```ini
[Settings]

;You can change the FOV multipler with this, I recommend 1.2
FOVMultiplier=1.0

;Enables the developer menu
EnableDevMenu=1

;All static objects are now included in shadow maps, along with generally improved quality
ImprovedShadows=1

;Vastly improves draw distance, though some maps have visual issues with it
ImproveDrawDistance=0

;Adds proper borders to windowed mode
FixWindowed=0
```

## Credits
- [SDL3](https://github.com/libsdl-org/SDL) for the PS4/PS5 and switch controller support.
- [MinHook](https://github.com/tsudakageyu/minhook) for hooking.
- [12brendon34](https://github.com/12brendon34) for general help.