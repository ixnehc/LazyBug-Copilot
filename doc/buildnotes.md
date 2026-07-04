# Build Notes

This document explains how to set up the development environment and build **LazyBug-Copilot**.

---

## Prerequisites

### 1. Visual Studio 2022

- Download and install **[Visual Studio 2022](https://visualstudio.microsoft.com/vs/)** (Community, Professional, or Enterprise).
- During installation, select the following workloads:
  - **Desktop development with C++**
  - **.NET desktop development**
- Ensure the **MFC (Microsoft Foundation Classes)** component is installed:
  - In the Visual Studio Installer, go to the **Individual components** tab.
  - Search for **C++ MFC for latest v143 build tools (x86 & x64)** and make sure it is checked.

### 2. Git

- Required to clone the third-party library repositories below.

---

## Third-Party Dependencies

The project uses the following libraries. Each must be downloaded (and where applicable, compiled), then its **headers** and **library files** placed under the `extlib/` directory.

| Library | Version | Source | Type |
|---------|---------|--------|------|
| Boost | 1.90 | https://www.boost.org/ | Headers + static/dynamic libs |
| curl | latest stable | https://github.com/curl/curl | Headers + static lib |
| LLVM / libclang | 20.x | https://github.com/llvm/llvm-project | Headers + import lib |
| Lucene++ | 3.0.9 | https://github.com/luceneplusplus/LucenePlusPlus | Headers + static lib |
| nlohmann/json | ≥ 3.11 | https://github.com/nlohmann/json | Header-only |
| tree-sitter | latest | https://github.com/tree-sitter/tree-sitter | Headers + static lib |

### Expected extlib Layout

After copying, the `extlib/` directory should look like this:

```
extlib/
├── boost_1_90/                  # Boost libraries
│   ├── boost/                   #   (headers — root of include tree)
│   ├── libboost_*.lib           #   (static libs)
│   └── boost_*.dll / boost_*.lib  # (dynamic libs + import libs)
├── curl/
│   ├── include/
│   │   └── curl/                #   curl/*.h
│   └── lib/
│       ├── libcurl.lib
│       └── libcurl_static.lib
├── LLVM/
│   ├── include/
│   │   ├── clang-c/             #   clang C API headers
│   │   └── llvm-c/              #   LLVM C API headers
│   └── lib/
│       └── libclang.lib
├── lucene/
│   ├── include/
│   │   └── lucene++/            #   lucene++ headers
│   └── lib/
│       └── lucene++.lib
├── nlohmann_jason/
│   └── nlohmann/                #   json.hpp (single-include or multi-file)
└── treesitter/
    ├── include/                 #   tree_sitter/ headers
    └── lib/
        ├── tree-sitter.lib
        └── tree-sitter-cpp.lib
```

### Building Each Dependency

#### Boost (1.90)

```bash
# Download from https://www.boost.org/users/download/
# Or clone:
git clone --branch boost-1.90.0 https://github.com/boostorg/boost.git

# Bootstrap and build (x64, static + dynamic, VS2022 toolset)
bootstrap.bat
b2.exe toolset=msvc-14.3 address-model=64 link=static,shared runtime-link=shared,static -j8
```

Copy:
- `boost/` (header tree) → `extlib/boost_1_90/boost/`
- `stage/lib/*.lib` → `extlib/boost_1_90/`

#### curl

```bash
git clone https://github.com/curl/curl.git
cd curl
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCURL_USE_SCHANNEL=ON
cmake --build build --config Release
```

Copy:
- `include/curl/` → `extlib/curl/include/curl/`
- `build/lib/libcurl.lib` → `extlib/curl/lib/`

#### LLVM / libclang

```bash
git clone --branch llvmorg-20.1.0 --depth 1 https://github.com/llvm/llvm-project.git
mkdir llvm-project\build && cd llvm-project\build
cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="X86" -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_ZLIB=OFF
cmake --build . -j8 --target clang
```

Copy:
- `llvm-project/clang/include/clang-c/` → `extlib/LLVM/include/clang-c/`
- `llvm-project/llvm/include/llvm-c/` → `extlib/LLVM/include/llvm-c/`
- `build/lib/libclang.lib` → `extlib/LLVM/lib/`

#### Lucene++ (3.0.9)

```bash
git clone https://github.com/luceneplusplus/LucenePlusPlus.git
cd LucenePlusPlus
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Copy:
- `include/lucene++/` → `extlib/lucene/include/lucene++/`
- `build/src/core/Release/lucene++.lib` → `extlib/lucene/lib/`

#### nlohmann/json

```bash
git clone https://github.com/nlohmann/json.git
```

Copy:
- `single_include/nlohmann/` → `extlib/nlohmann_jason/nlohmann/`
  (or `include/nlohmann/` if using the multi-header version)

#### tree-sitter

```bash
git clone https://github.com/tree-sitter/tree-sitter.git
cd tree-sitter
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Copy:
- `lib/include/tree_sitter/` → `extlib/treesitter/include/`
- `build/tree-sitter.lib` → `extlib/treesitter/lib/`
- `build/tree-sitter-cpp.lib` (if building tree-sitter-cpp separately)

---

## Build

1. Open **`LazyBugPlugIn.sln`** in Visual Studio 2022.
2. Select the desired configuration (**Debug** or **Release**) and platform (**x64**).
3. Right-click the **`LazyBugPlugInVSIX`** project in Solution Explorer and select **Build** (or **Rebuild**).

The compiled VSIX extension package will be generated under `LazyBugPlugIn/LazyBugPlugInVSIX/bin/`.

---

## Notes

- All third-party libraries must be built for **x64** and use the **v143 (MSVC 2022)** toolset to remain ABI-compatible.
- The project links against the **static** (non-DLL) variants of libraries by default, except `libclang.dll` and `libcurl.dll` which are loaded dynamically at runtime.
- If you encounter MFC-related errors during compilation, verify the MFC component is installed in the Visual Studio Installer.
