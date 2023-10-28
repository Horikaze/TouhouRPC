# TouhouRPC
TouhouRPC is a program that creates a Discord Rich Presence status for Touhou games. 

## DLL files
Make sure the *discord_game_sdk.dll* file is in the same folder as the *TouhouRPC.exe* executable before executing it. If it doesn't, an error message will show up and the program will close.  
You can use the .dll file available in the Discord Game SDK zip file provided by Discord. You will find the dlls under the *lib* folder. ([Download link for v3.2.1](https://dl-game-sdk.discordapp.net/3.2.1/discord_game_sdk.zip))

## Supported games
- Touhou 06 - Embodiment of Scarlet Devil
- Touhou 07 - Perfect Cherry Blossom
- Touhou 08 - Imperishable Night
- Touhou 09 - Phantasmagoria of Flower View
- Touhou 09.5 - Shoot the Bullet
- Touhou 10 - Mountain of Faith
- Touhou 11 - Subterranean Animism
- Touhou 12 - Undefined Fantastic Object
- Touhou 12.5 - Double Spoiler
- Touhou 12.8 - Great Fairy Wars
- Touhou 13 - Ten Desires
- Touhou 14 - Double Dealing Character
- Touhou 14.3 - Impossible Spell Card
- Touhou 15 - Legacy of Lunatic Kingdom
- Touhou 16 - Hidden Stars in Four Seasons
- Touhou 17 - Wily Beast and Weakest Creature
- Touhou 18 - Unconnected Marketeers - Trial v0.02a (you must name the game "th18tr.exe")
- Touhou 18 - Unconnected Marketeers

If you want to see other games or fangames supported, contributions are welcome!

## Display examples
![Th17 Example - Fighting](https://relick.me/touhourpc/1-wbawc-playing.png)
  
![Th07 Example - Menu](https://relick.me/touhourpc/2-pcb-menu.png)
  
![Th13 Example - Spell Card Practice](https://relick.me/touhourpc/3-td-spellcard.png)
  
![Th14 Example - Music Room](https://relick.me/touhourpc/4-ddc-musicroom.png)
  
![Th17 Example - Stage Practice](https://relick.me/touhourpc/5-wbawc-practicing.png)

## Develop and compile yourself
To compile the program by yourself, you will need:
- Visual Studio 2022
- MSVC v143 Build Tools
Open the project in VS2022 and build the solution. You can build for x64 or x86 architectures in Debug or Release mode.

## Additional notes
This program supports games patched using the [Touhou Community Reliant Automatic Patcher](https://github.com/thpatch/thcrap). It also supports games using the vpatch.

## License
TouhouRPC is licensed under the [GNU General Public License v3.0](LICENSE).