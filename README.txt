STREET MAYHEM PSP — SOURCE PROJECT
==================================

This is an original PSP-compatible open-city driving sandbox prototype.
It is inspired by the broad arcade sandbox genre, not a copy of Payback 2.
It includes:
- A small 3D city built from optimized low-poly geometry
- Third-person player car with acceleration, braking and steering
- Eight roaming rival traffic bots
- Simple chase/roam bot behavior and collision scoring
- PSP controls and PPSSPP compatibility
- No external assets required

BUILD
-----
Requires the PSPSDK toolchain.

1. Extract this ZIP.
2. Open a terminal in this folder.
3. Run: make
4. The build creates EBOOT.PBP in the project folder.
5. Copy EBOOT.PBP into a folder on your PSP memory stick:
   /PSP/GAME/STREETMAYHEM/EBOOT.PBP
   For PPSSPP, browse to the EBOOT.PBP and launch it.

CONTROLS
--------
D-pad UP    accelerate
D-pad DOWN  brake / reverse
D-pad LEFT  steer left
D-pad RIGHT steer right
SELECT      pause
X / START   start from title screen

NOTES
-----
This is a playable foundation, not a finished commercial-scale game.
The city and cars are procedural low-poly models to keep memory use modest.
For a true high-detail PSP release, add authored textures, audio, animation,
mission scripting, and optimized environment meshes. PSP hardware has strict
memory and fill-rate limits, so "high graphics" means carefully optimized art.
