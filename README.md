# Historical map

## Compile on Linux

According to [Compiler support for C++20](https://en.cppreference.com/w/cpp/compiler_support/20),
GCC > 10 is required to use Parenthesized initialization of aggregates .

### Setup envirnment

`sudo apt-get install libgl1-mesa-dev libglew-dev libglfw3-dev libcurl4-openssl-dev libgtk-3-dev -y`

### Compile

1. `mkdir build && cd build`
2. `cmake ..` or `cmake -DGTEST_ENABLE=ON ..` if you want to enable test
3. `make -j`
4. `ctest` if you want to run test

## Compile on MacOS

According to [Compiler support for C++20](https://en.cppreference.com/w/cpp/compiler_support/20), 
the Apple clang doesn't support Parenthesized initialization of aggregates, so we have to install
clang (>16) from Homebrew or MacPort.

### Setup envirnment
1. Download clang from Homebrew: `brew install llvm` 
2. Add Homebrew clang to your path: `export PATH="/opt/homebrew/opt/llvm/bin:$PATH"`
3. Set `CC` and `CXX` to the Homebrew clang: `export CC=/opt/homebrew/opt/llvm/bin/clang && export CXX=/opt/homebrew/opt/llvm/bin/clang++`
4. Install dependencies from brew: `brew install glew glfw cmake`
5. Pull dependencies for git submodules: `git submodule init && git submodule update`

### Compile

1. `mkdir build && cd build`
2. `cmake ..` or `cmake -DGTEST_ENABLE=ON ..` if you want to enable test
3. `make -j`
4. `ctest` if you want to run test