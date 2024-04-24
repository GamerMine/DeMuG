# Emu_Gameboy

This project was created to have a better understanding of how the GameBoy (DMG) is working internally.
It is not my first emulator, I have done a Space Invader emulator that you can find as a repo on my Github account
as Emu_Intel8080.

## Dependencies

### Debian/Ubuntu
- **gcc >= 9** 
- **g++ >= 9**
- **git**
- **cmake >= 3.25** 
- **build-essentials** 
- **libx11-dev** 
- **libxinerama-dev** 
- **libgl-dev**
- **libxcursor-dev** 
- **libxrandr-dev** 
- **libxi-dev**
- **libwayland-dev**
- **libxkbcommon-dev**
- **libgtk-3-dev**

You can use the following exemple command to install them:<br>
`sudo apt install gcc g++ git cmake build-essentials libx11-dev libxinerama-dev libgl-dev
libxcursor-dev libxrandr-dev libxi-dev`

### Fedora
- **gcc >= 9**
- **g++ >= 9**
- **git**
- **cmake >= 3.25**
- **libX11-devel** 
- **libXrandr-devel** 
- **libXinerama-devel** 
- **libXcursor-devel** 
- **libXi-devel** 
- **mesa-libGL-devel**
- **wayland-protocols-devel**

You can use the following exemple command to install them:<br>
`sudo dnf install gcc g++ git cmake libX11-devel libXrandr-devel libXinerama-devel 
libXcursor-devel libXi-devel mesa-libGL-devel`

### MacOS
- [**XCode**](https://apps.apple.com/fr/app/xcode/id497799835?mt=12) dependencies
Install the following packages (using homebrew for exemple):
- [**git**](https://formulae.brew.sh/formula/git#default)
- [**cmake**](https://formulae.brew.sh/formula/cmake#default) >= 3.25
- [**gcc**](https://formulae.brew.sh/formula/gcc#default) supporting C++20

### Windows
- **gcc** supporting C++20. You can use the binaries provided by [WinLibs](https://winlibs.com).<br>
- [**git**](https://git-scm.com/downloads)
- [**cmake**](https://cmake.org/download/) >= 3.25

## Compilation
Clone the repository<br>
`git clone https://github.com/GamerMine/Emu_GameBoy.git --recurse-submodules`

Compile !<br>
`cd Emu_GameBoy && mkdir build && cd build && cmake .. && make`
