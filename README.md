# Sage 🌿

Sage is named after the wisdom it takes to use modern C++,
where compile-time metaprogramming feels like casting spells on the compiler.

---

Sage strives to push as much work as possible onto the compiler rather than
the runtime — template metaprogramming, `constexpr`/`consteval` evaluation,
and compile-time configuration (see [`docs/`](docs/)) are favored over
runtime branching wherever practical.

> The library is distributed as C++ modules rather than traditional headers.
There is currently no header-based fallback — consuming Sage requires a
modules-capable compiler and build system.

## Features

Each submodule has its own documentation under [`docs/`](docs/), including its configuration options.

- [Core](docs/core_docs.md)
- [Logger](docs/logger_docs.md)

## Requirements

- A modules-capable toolchain (no header support currently)
- A C++23 compiler
- CMake 3.28

## Building

Sage builds with CMake:

```
mkdir build && cd build
cmake ..
cmake --build .
```
Sage can optionally be built as a shared library using the `SAGE_BUILD_SHARED_LIBS` flag when compiling.

> Module support requires a CMake version and generator combination capable
of scanning C++ module dependencies (e.g. CMake 3.28+ with Ninja).

## Usage

Add Sage as a subdirectory and link against it:

```cmake
add_subdirectory(<your_libs>/sage)
target_link_libraries(<your_target> PRIVATE sage)
```

## Status

Early and actively developed — APIs may change between versions.

## License

See [LICENSE](LICENSE).
