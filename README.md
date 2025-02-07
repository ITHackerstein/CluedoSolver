# Cluedo Solver

Cluedo Solver is a tool designed to help you win [Cluedo](https://en.wikipedia.org/wiki/Cluedo) games by making deductions about the suggestions made in them.

## Releases

You can find precompiled releases for Windows and Linux in the [Releases](https://github.com/ITHackerstein/CluedoSolver/releases/) section in the format of an `.exe` for Windows and an `.AppImage` for Linux.

## Building from scratch

### Requirements

* [GCC](https://gcc.gnu.org)/[Clang](https://clang.llvm.org/)
	* In Windows you can't use [MSVC](https://visualstudio.microsoft.com/vs/features/cplusplus/) as it doesn't support [compound statements](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html). As an alternative you can use [MinGW-w64](https://www.mingw-w64.org/) which is tested and known to work with this project.
* [CMake](https://cmake.org/)
* [Git](https://git-scm.com/)

### On Linux

```shell
git clone --recursive https://github.com/ITHackerstein/CluedoSolver.git
cd CluedoSolver
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### On Windows

```shell
git clone --recursive https://github.com/ITHackerstein/CluedoSolver.git
cd CluedoSolver
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
cmake --build build
```

### Notes

The built executable will be available at `build/src/CluedoSolver`.

To speed up the build you can also use [Ninja](https://ninja-build.org/) as your build system of choice by specifying the following flag to CMake:
```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
```
or you can specify the number of jobs to execute in parallel by specifying the following flags when building:
```shell
cmake --build -j4
```
The number of jobs you can use depends on your system CPU, make sure to check before running the command.
