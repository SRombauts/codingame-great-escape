codingame-great-escape
----------------------

[![Join the chat at https://gitter.im/SRombauts/codingame-great-escape](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/SRombauts/codingame-great-escape?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Travis CI Linux Build Status](https://travis-ci.org/SRombauts/codingame-great-escape.svg)](https://travis-ci.org/SRombauts/codingame-great-escape "Travis CI Linux Build Status")
[![AppVeyor Windows Build status](https://ci.appveyor.com/api/projects/status/github/SRombauts/codingame-great-escape?svg=true)](https://ci.appveyor.com/project/SbastienRombauts/codingame-great-escape "AppVeyor Windows Build status")

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

### Debug build for Unix Makefiles

```bash
mkdir Debug
cd Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug   # -G "Unix Makefiles"
cmake --build . # make
ctest .         # make test
```

### Release build

```bash
mkdir Release
cd Release
cmake .. -DCMAKE_BUILD_TYPE=Release  # -G "Unix Makefiles"
cmake --build . # make
ctest .         # make test
```

### Continuous Integration

This project is continuously tested under Ubuntu Linux with the gcc and clang compilers
using the Travis CI community service with the above CMake building and testing procedure.
It is also tested in the same way under Windows Server 2012 R2 with Visual Studio 2013 compiler
using the AppVeyor countinuous integration service.

Detailed results can be seen online:
 - https://travis-ci.org/SRombauts/codingame-great-escape
 - https://ci.appveyor.com/project/SbastienRombauts/codingame-great-escape

