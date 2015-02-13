codingame-great-escape
------------

[![Linux Build Status](https://travis-ci.org/SRombauts/codingame-great-escape.svg)](https://travis-ci.org/SRombauts/codingame-great-escape)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/github/SRombauts/codingame-great-escape?svg=true)](https://ci.appveyor.com/project/SbastienRombauts/codingame-great-escape)

My attempt at the Multiplayer CodinGame contest "The Great Escape".

Copyright (c) 2015 Sébastien Rombauts (sebastien.rombauts@gmail.com)

## Building & testing with CMake

### Get Google Test submodule

```bash
git submodule init
git submodule update
```

### Typical generic build (see also "build.bat" or "./build.sh")

```bash
mkdir build
cd build
cmake ..        # cmake .. -G "Visual Studio 10"    # for Visual Studio 2010
cmake --build . # make
ctest .         # make test
```

### Debug build

```bash
mkdir Debug
cd Debug
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build . # make
ctest .         # make test
```

### Release build

```bash
mkdir Release
cd Release
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . # make
ctest .         # make test
```

### Continuous Integration

This project is continuously tested under Ubuntu Linux with the gcc and clang compilers
using the Travis CI community service with the above CMake building and testing procedure.

Detailed results can be seen online: https://travis-ci.org/SRombauts/codingame-great-escape

