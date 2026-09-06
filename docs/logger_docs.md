# Sage Logger Module Documentation (import sage:log)

A lightweight, compile-time configurable logging module. Behaviour is primarily tuned through compile definitions; however, a custom logger can be used when the default does not fit your needs.


## Table of Contents

- [Compile Settings](#compile-settings)
- [Severity Levels](#severity-levels)
- [Basic Usage](#basic-usage)
- [Customizing Behavior via Compile Flags](#customizing-behavior-via-compile-flags)
- [Building a Custom Logger](#building-a-custom-logger)
- [Quick Reference](#quick-reference)

---

## Compile Settings

All settings are compile definitions supplied by your build system (e.g.
CMake's `-D` flag); however, can not be `#define`d in source — see
[Customizing Behavior via Compile Flags](#customizing-behavior-via-compile-flags)
for the full setup.

| Setting | Type | Description | Default |
|---|---|---|---|
| `SAGE_LOG_ENABLED_SEVERITY` | bitmask | One bit per `sage::Severity` level (`TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`). `1` = enabled, `0` = disabled. | All enabled |
| `SAGE_LOG_NO_FORMATTING` | bool | Disables internal extra formatting applied to strings. | `false` |
| `SAGE_LOG_NO_COLORS` | bool | Disables colored output to stdout/stderr. (File output is never colored regardless of this setting.) | `false` |
| `SAGE_LOG_WRITE_TO_FILE` | bool | Enables writing log output to `last_run.log` on every log call. | `false` |
| `SAGE_LOG_THROW_ON_ERROR` | bool | Throws a `std::runtime_error` whenever an `ERROR` level log is emitted. | `false` |
| `SAGE_LOG_NO_DEFAULT_LOGGER` | bool | Disables the built-in `sage::log` module so you can supply your own custom logger (see [Building a Custom Logger](#building-a-custom-logger)). | `false` |

---

## Severity Levels

`sage::Severity` defines six levels, from most to least verbose:

```
TRACE < DEBUG < INFO < WARN < ERROR < FATAL
```

Each level corresponds to a single bit in the `SAGE_LOG_ENABLED_SEVERITY`
bitmask. Setting a bit to `1` enables that level; `0` disables it. This lets
you strip out unwanted levels (like `TRACE`/`DEBUG`) from release builds without
touching call sites.

NOTE: Since `SAGE_LOG_ENABLED_SEVERITY` is a compile definition, it can't be set
with `#define` in your source — it's supplied by the build system and baked
in at compile time. With CMake, pass it on the command line (or set it in
`CMakeLists.txt`) and forward it as a compile definition:

```
cmake -DSAGE_LOG_ENABLED_SEVERITY=0x3F ..
```

```cmake
# CMakeLists.txt
target_compile_definitions(your_target PRIVATE
    SAGE_LOG_ENABLED_SEVERITY=${SAGE_LOG_ENABLED_SEVERITY}
)
```

A plain decimal value (e.g. `63` for all enabled) doesn't make the underlying bit pattern
obvious at a glance. Binary literal are easier to read and edit when available.

**Bit reference:**

| Bit | Level |
|---|---|
| 0 (LSB) | `TRACE` |
| 1 | `DEBUG` |
| 2 | `INFO` |
| 3 | `WARN` |
| 4 | `ERROR` |
| 5 (MSB) | `FATAL` |

**Same value, three formats:**

| Format | All enabled | Only INFO and above |
|---|---|---|
| Binary *(most readable)* | `0b111111` | `0b111100` |
| Hex | `0x3F` | `0x3C` |
| Decimal | `63` | `60` |

```
# All six levels enabled
cmake -DSAGE_LOG_ENABLED_SEVERITY=0x3F ..

# Only INFO, WARN, ERROR, FATAL (TRACE/DEBUG disabled) — hex
cmake -DSAGE_LOG_ENABLED_SEVERITY=0x3C ..

# Same result, spelled as a binary literal (bit-by-bit is easiest to verify)
cmake -DSAGE_LOG_ENABLED_SEVERITY=0b111100 ..
```

> Binary literals (`0b...`) are not always available, but since the value is just
> substituted as text into the macro, either format compiles fine.

---

## Basic Usage

With the default logger enabled (i.e. `SAGE_LOG_NO_DEFAULT_LOGGER` **not**
set), the library exposes `sage::log` directly:

```cpp
#include <sage/log.hpp>

int main() {
    sage::log::trace("Starting up...");
    sage::log::debug("Loaded config from disk");
    sage::log::info("Application initialized");
    sage::log::warn("Config value missing, using default");
    sage::log::error("Failed to connect to service");
    sage::log::fatal("Unrecoverable state, shutting down");
}
```

Only calls whose severity bit is enabled in `SAGE_LOG_ENABLED_SEVERITY`
will produce output — disabled levels compile down to no-ops.

---

## Customizing Behavior via Compile Flags

All settings are compile definitions, so they're supplied by your build
system, not `#define`d in source. With CMake, pass them on the command line:

```
cmake -DSAGE_LOG_ENABLED_SEVERITY=0x3C \
      -DSAGE_LOG_WRITE_TO_FILE=1 \
      -DSAGE_LOG_NO_COLORS=1 \
      -DSAGE_LOG_THROW_ON_ERROR=1 \
      -DSAGE_LOG_NO_FORMATTING=1 \
      ..
```

...and (optionally) forward each cache variable to the compiler in `CMakeLists.txt`:

```cmake
target_compile_definitions(your_target PRIVATE
    SAGE_LOG_ENABLED_SEVERITY=${SAGE_LOG_ENABLED_SEVERITY}
    SAGE_LOG_WRITE_TO_FILE=${SAGE_LOG_WRITE_TO_FILE}
    SAGE_LOG_NO_COLORS=${SAGE_LOG_NO_COLORS}
    SAGE_LOG_THROW_ON_ERROR=${SAGE_LOG_THROW_ON_ERROR}
    SAGE_LOG_NO_FORMATTING=${SAGE_LOG_NO_FORMATTING}
)
```

A few common configurations:

**Silence debug noise in release builds**
```
# TRACE and DEBUG disabled; INFO, WARN, ERROR, FATAL remain enabled
cmake -DSAGE_LOG_ENABLED_SEVERITY=0x3C ..
```

**Persist logs to disk for post-mortem debugging**
```
# Output is written/appended to `last_run.log`
cmake -DSAGE_LOG_WRITE_TO_FILE=1 ..
```

**Disable ANSI colors** (useful when piping stdout to a file or a CI log
viewer that doesn't render escape codes)
```
cmake -DSAGE_LOG_NO_COLORS=1 ..
```
> Note: file output (`last_run.log`) never contains color codes, regardless
> of this setting.

**Turn errors into exceptions** (useful for tests or strict fail-fast builds)
```
# Any sage::log::error(...) call now throws std::runtime_error
cmake -DSAGE_LOG_THROW_ON_ERROR=1 ..
```

**Disable internal string formatting**
```
# Strings are passed through as-is, without the library's extra formatting
cmake -DSAGE_LOG_NO_FORMATTING=1 ..
```

---

## Building a Custom Logger

If the default `sage::log` doesn't fit your needs (e.g. you need a specific write function, or different configs per module), you can replace it entirely.

### 1. Enable custom logger support

```
cmake -DSAGE_LOG_NO_DEFAULT_LOGGER=1 ..
```

This disables the built-in `sage::log` module and is **required** before a
custom logger can be defined using the `sage::log` alias.

### 2. Define a compile-time config struct

```cpp
struct MyConfig : sage::_backend::log::config {
    // Override / define config members as needed, e.g.:
    // uint8_t enabled_severity = 0b111100;
    // bool colors = true;
    // bool write_to_file = true;
    // void (*std_writer)(sage::Severity, std::string_view) = &my_stdout_writer;
    // void (*file_writer)(std::string_view)                = &my_file_writer;
};
```

### 3. Alias the logger with your config

```cpp
namespace sage {
    using log = sage::_backend::log::logger<MyConfig>;
}
```

Note: The `sage` namespace wrapper is optional — you're free to alias the logger
under any name/namespace you like:

```cpp
using my_logger = sage::_backend::log::logger<MyConfig>;

my_logger::info("Using a custom-named logger");
```
In this case, there is no need to set the `SAGE_LOG_NO_DEFAULT_LOGGER` flag.


### Full example

Build system:
```
cmake -DSAGE_LOG_NO_DEFAULT_LOGGER=1 -DSAGE_LOG_WRITE_TO_FILE=1 ..
```

Source:
```cpp
#include <sage/log.hpp>

// Any unset member functions will retain their default values
sage::_backend::log::config MyConfig {
    .enabled_severity = 0b111100; // no TRACE/DEBUG
    .write_to_file    = true;
};

namespace sage {
    using log = sage::_backend::log::logger<MyConfig>;
}

int main() {
    sage::log::info("Custom logger ready");
    sage::log::error("Something went wrong");
}
```

> Because `SAGE_LOG_NO_DEFAULT_LOGGER` disables the built-in `sage::log`,
> defining your own alias under `sage::log` doesn't conflict with anything —
> the name is free to reuse.

---

## Quick Reference

| Goal | Setting |
|---|---|
| Turn off specific severity levels | `SAGE_LOG_ENABLED_SEVERITY` bitmask |
| Strip extra string formatting | `SAGE_LOG_NO_FORMATTING` |
| No ANSI colors on stdout | `SAGE_LOG_NO_COLORS` |
| Write logs to `last_run.log` | `SAGE_LOG_WRITE_TO_FILE` |
| Throw on `ERROR` logs | `SAGE_LOG_THROW_ON_ERROR` |
| Replace the default logger | `SAGE_LOG_NO_DEFAULT_LOGGER` + custom `config` struct |
