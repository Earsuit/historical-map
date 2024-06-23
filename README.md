# Historical map

## General tool chian requirements 

1. [Conan 2](https://conan.io/) 
2. cmake >= 3.23
3. Parenthesized initialization of aggregates feature is required, GCC > 10 on Linux, clang = 17.0.6 on MacOS, MSVC >= 19.28 on Windows.

## Compile on Linux

### Setup envirnment

Config the profile if you don't have:

`conan profile detect --force` 

and set the C++ standard to 20 by modifying `settings.compiler.cppstd=gnu20`.

### Compile

1. `conan install . --build=missing -s build_type=Release -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True`
2. `cmake --preset conan-release`
3. `cd build/Release`
4. `cmake --build . -j`
5. Compile translation: `cmake --build . -j --target translation`
6. `ctest` if you want to run test

## Compile on MacOS

### Setup envirnment
1. Download clang from Homebrew: `brew install llvm` 
2. Add Homebrew clang to your path: `export PATH="/opt/homebrew/opt/llvm/bin:$PATH"`
3. Set `CC` and `CXX` to the Homebrew clang: `export CC=/opt/homebrew/opt/llvm/bin/clang && export CXX=/opt/homebrew/opt/llvm/bin/clang++`
4. Config the profile if you don't have: `conan profile detect --force`  and set the C++ standard to 20 by modifying `settings.compiler.cppstd=gnu20`.

### Compile

1. `conan install . --build=missing -s build_type=Release`
2. `cmake --preset conan-release`
3. `cd build/Release`
4. `cmake --build . -j`
5. Compile translation: `cmake --build . -j --target translation`
6. `ctest` if you want to run test

## Compile on Windows

### Setup envirnment

Config the profile if you don't have: `conan profile detect --force`  and set the C++ standard to 20 by modifying `settings.compiler.cppstd=20`.

### Compile

1. `conan install . --build=missing -s build_type=Release`
2. `cmake --preset conan-default`
3. `cd build/`
4. `cmake --build . -j --config Release`
5. `cmake --build . -j --config Release --target translation`
6. `ctest` if you want to run test
