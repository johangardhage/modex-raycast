# retro-gardens

The Gardens of Imagination raycasting engine, brought back from the 90's.

![Screenshot](/screenshots/raycast.png "raycast")
![Screenshot](/screenshots/raycastlight.png "raycastlight")

## Prerequisites

To build the demo programs, you must first install the following tools:

- [GCC 7.1](https://gcc.gnu.org/) (or later)
- [Ninja 1.5](https://ninja-build.org/) (or later)
- [SDL 2.0.4](https://www.libsdl.org/) (or later)

### Install dependencies

#### openSUSE

`$ sudo zypper install ninja gcc-c++ libSDL2-devel`

#### Ubuntu

`$ sudo apt install ninja-build g++ libsdl2-dev`

#### Windows

Manually download and install [MSYS2](https://www.msys2.org/).

Run `View advanced system settings` from the Start menu and add a PATH to `C:\msys64\mingw64\bin`.

Run `MSYS2 MSYS` from the Start menu. Update the package database and base packages with `pacman -Syu`.

Run `MSYS2 MSYS` from the Start menu again. Update the rest of the base packages with `pacman -Syu`.

Install the development tools with `pacman -S git mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja mingw-w64-x86_64-SDL2`.

Close the `MSYS2 MSYS` window and run `MSYS MinGW 64-bit` from the Start menu.

Clone the git repository with `git clone https://github.com/johangardhage/retro-gardens.git`.

Finally, to be able to build the demos on Windows, edit the file `build.ninja` and uncomment the line [#  command = $cc $in $windows -o $out.](build.ninja#L10).

## Build instructions

To build the demo programs, run:

`$ ninja`

A `build` directory will be created, containing the demo programs.

## Usage

```
Usage: raycast [OPTION]...

Options:
 -h, --help           Display this text and exit
 -w, --window         Render in a window
     --fullwindow     Render in a fullscreen window
 -f, --fullscreen     Render in fullscreen
 -v, --vsync          Enable sync to vertical refresh
     --novsync        Disable sync to vertical refresh
 -l, --linear         Render using linear filtering
     --nolinear       Render using nearest pixel sampling
 -c, --showcursor     Show mouse cursor
     --nocursor       Hide mouse cursor
     --showfps        Show frame rate in window title
     --nofps          Hide frame rate
     --capfps=VALUE   Limit frame rate to the specified VALUE
```

## License

Licensed under MIT license. See [LICENSE](LICENSE) for more information.

## Authors

* Johan Gardhage
* General ideas and code from Christopher Lampton's book [Gardens of Imagination](https://www.amazon.com/Gardens-Imagination-Programming-Maze-Games/dp/187873959X) (1994 - Waite Group)
