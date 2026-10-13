# Sage Core Module Documentation (import sage:core_<module>)

The core modules provide the foundational building blocks used throughout Sage:
portable type aliases, the logging severity enum, numeric limits and size
helpers, and reusable template constraints. Every module is a partition of the
`sage` module and is compiled with C++23 module support.


## Table of Contents

- [Module Overview](#module-overview)
- [Types](#types)
  - [Compile Settings](#compile-settings)
  - [Type Reference](#type-reference)
  - [Compile-Time Guarantees](#compile-time-guarantees)
  - [Platform Notes](#platform-notes)
  - [Basic Usage](#basic-usage)
- [Severity](#severity)
- [Numerics](#numerics)
  - [Integer Limits](#integer-limits)
  - [Floating-Point Limits](#floating-point-limits)
  - [Memory Size Helpers](#memory-size-helpers)
- [Template Types](#template-types)
  - [numeric_type](#numeric_type)
- [Quick Reference](#quick-reference)

---

## Module Overview

| Module | Namespace | Description | Re-exports |
|---|---|---|---|
| `sage:core_types` | `sage::types` | Fixed-width, least-width, fast-width, pointer, size, character, boolean and floating-point type aliases. | — |
| `sage:core_severity` | `sage` | The `sage::Severity` enum used to classify log messages. | `sage:core_types` |
| `sage:core_numerics` | `sage::numerics` | Integer and floating-point limit constants, and binary/decimal memory size helpers. | `sage:core_types`, `sage:core_template_types` |
| `sage:core_template_types` | `sage::template_types` | Reusable concepts for constraining templates. | — |

Because `sage:core_severity` and `sage:core_numerics` re-export their
dependencies, importing either one also makes the `sage::types` aliases
available.

---

## Types

Type aliases defined in `sage::types` (module `sage:core_types`).

### Compile Settings

Like the logger settings, this option is a compile definition supplied by your
build system (e.g. CMake's `-D` flag) when the `sage` module is built; it can
not be `#define`d in source.

| Setting | Type | Description | Default |
|---|---|---|---|
| `SAGE_TYPES_SCOPE_ENABLED` | bool | When defined, the global `using namespace sage::types;` is skipped and every alias must be qualified (e.g. `sage::types::int32`). | `false` (aliases are available unqualified) |


---

### Type Reference

| Type | Category | Underlying Type | Notes |
|---|---|---|---|
| `int8` | Fixed-width signed | 8-bit int | |
| `int16` | Fixed-width signed | 16-bit int | |
| `int32` | Fixed-width signed | 32-bit int | |
| `int64` | Fixed-width signed | 64-bit int | |
| `int128` | Fixed-width signed | 128-bit int | glibc only |
| `uint8` | Fixed-width unsigned | 8-bit uint | |
| `uint16` | Fixed-width unsigned | 16-bit uint | |
| `uint32` | Fixed-width unsigned | 32-bit uint | |
| `uint64` | Fixed-width unsigned | 64-bit uint | |
| `uint128` | Fixed-width unsigned | 128-bit uint | glibc only |
| `int8_least` | Least-width signed | ≥8-bit int | always available |
| `int16_least` | Least-width signed | ≥16-bit int | always available |
| `int32_least` | Least-width signed | ≥32-bit int | always available |
| `int64_least` | Least-width signed | ≥64-bit int | always available |
| `uint8_least` | Least-width unsigned | ≥8-bit uint | always available |
| `uint16_least` | Least-width unsigned | ≥16-bit uint | always available |
| `uint32_least` | Least-width unsigned | ≥32-bit uint | always available |
| `uint64_least` | Least-width unsigned | ≥64-bit uint | always available |
| `int8_fast` | Fast-width signed | ≥8-bit int | fastest type ≥ width |
| `int16_fast` | Fast-width signed | ≥16-bit int | fastest type ≥ width |
| `int32_fast` | Fast-width signed | ≥32-bit int | fastest type ≥ width |
| `int64_fast` | Fast-width signed | ≥64-bit int | fastest type ≥ width |
| `uint8_fast` | Fast-width unsigned | ≥8-bit uint | fastest type ≥ width |
| `uint16_fast` | Fast-width unsigned | ≥16-bit uint | fastest type ≥ width |
| `uint32_fast` | Fast-width unsigned | ≥32-bit uint | fastest type ≥ width |
| `uint64_fast` | Fast-width unsigned | ≥64-bit uint | fastest type ≥ width |
| `int_max` | Max-width signed | largest available int | |
| `uint_max` | Max-width unsigned | largest available uint | |
| `intptr` | Pointer-sized signed | signed pointer-width int | |
| `uintptr` | Pointer-sized unsigned | unsigned pointer-width int | |
| `usize` | Size | unsigned, pointer-width | result of `sizeof` |
| `isize` | Size | signed, pointer-width | signed counterpart of `usize` |
| `char8` | Character | `char` | 8-bit, enforced via `static_assert` |
| `char16` | Character | `__CHAR16_TYPE__` | |
| `char32` | Character | `__CHAR32_TYPE__` | |
| `uchar8` | Unsigned character | unsigned counterpart of `char8` | |
| `uchar16` | Unsigned character | unsigned counterpart of `char16` | |
| `uchar32` | Unsigned character | unsigned counterpart of `char32` | |
| `bool8` | Boolean | `bool` | |
| `byte` | Byte | `uchar8` | raw 8-bit unsigned data |
| `float32` | Floating-point | `float` | 32-bit, IEC 559, enforced via `static_assert` |
| `float64` | Floating-point | `double` | 64-bit, IEC 559, enforced via `static_assert` |
| `float80_least` | Floating-point | `long double` | at least 80-bit; storage is often 128-bit, width platform-dependent |

**Choosing between the integer families:**

| Family | Use when | Example |
|---|---|---|
| Fixed-width (`int32`) | You need an exact size (serialization, binary formats). Fails to compile if the platform has no such type. | `uint32 id;` |
| Least-width (`int32_least`) | You need *at least* a given range and want code that always compiles. | `int32_least counter;` |
| Fast-width (`int32_fast`) | You need *at least* a given range and want the fastest type for arithmetic. | `int32_fast i;` |
| Max-width (`int_max`) | You need the widest integer the platform supports. | `uint_max total;` |

---

### Compile-Time Guarantees

The following checks are enforced with `static_assert`; a platform that does
not satisfy them fails to compile.

| Check | Applies to | Error message |
|---|---|---|
| `sizeof(char) == 1` | `char8` | Platform does not support 8-bit char type |
| `sizeof(float) == 4` | `float32` | Platform does not support 32-bit floating-point type |
| `std::numeric_limits<float>::is_iec559` | `float32` | — |
| `sizeof(double) == 8` | `float64` | Platform does not support 64-bit floating-point type |
| `std::numeric_limits<double>::is_iec559` | `float64` | — |

---

### Platform Notes

- On non-glibc platforms, types fall back to the standard `<cstdint>` / `<cstddef>` types (`std::int8_t`, `std::uintmax_t`, `std::size_t`, etc.).
- `int128`/`uint128` are only defined under glibc (`__int128_t`/`__uint128_t`); no portable fallback is provided.

---

### Basic Usage

By default the aliases are available without qualification:

```cpp
import sage;

int32   count  = 0;
usize   length = sizeof(count);
byte    raw    = 0xFF;
float64 ratio  = 0.5;

int32_least small_counter = 0;  // always compiles
int32_fast  loop_index    = 0;  // fastest type that holds 32 bits
```

With `SAGE_TYPES_SCOPE_ENBALED` defined, qualify each alias:

```cpp
import sage;

sage::types::int32 count = 0;
sage::types::usize length = sizeof(count);
```

---

## Severity

`sage::Severity` (module `sage:core_severity`) classifies log messages. It is
the enum behind the `SAGE_LOG_ENABLED_SEVERITY` bitmask described in the
[Logger documentation](logger_docs.md).

```cpp
enum class Severity : sage::types::uint8;
```

| Enumerator | Value | Bit | Binary |
|---|---|---|---|
| `TRACE` | `1 << 0` | 0 (LSB) | `0b000001` |
| `DEBUG` | `1 << 1` | 1 | `0b000010` |
| `INFO` | `1 << 2` | 2 | `0b000100` |
| `WARN` | `1 << 3` | 3 | `0b001000` |
| `ERROR` | `1 << 4` | 4 | `0b010000` |
| `FATAL` | `1 << 5` | 5 (MSB) | `0b100000` |

Levels run from most to least verbose:

```
TRACE < DEBUG < INFO < WARN < ERROR < FATAL
```

Every enumerator occupies exactly one bit, so levels can be combined into a
mask. Since `Severity` is a scoped enum, convert to `uint8` before combining
or testing bits:

```cpp
import sage;

// Only INFO, WARN, ERROR, FATAL
constexpr uint8 mask =
    static_cast<uint8>(sage::Severity::INFO)  |
    static_cast<uint8>(sage::Severity::WARN)  |
    static_cast<uint8>(sage::Severity::ERROR) |
    static_cast<uint8>(sage::Severity::FATAL);   // 0b111100

constexpr bool is_enabled(sage::Severity level) {
    return (mask & static_cast<uint8>(level)) != 0;
}

static_assert(!is_enabled(sage::Severity::DEBUG));
static_assert(is_enabled(sage::Severity::ERROR));
```

---

## Numerics

Numeric limits and size helpers in `sage::numerics` (module
`sage:core_numerics`). Importing this module also provides `sage::types` and
`sage::template_types`.

### Integer Limits

All integer limits are `constexpr` values of matching types, initialized from
the compiler's predefined limit macros.

| Constant | Type | Value | Source |
|---|---|---|---|
| `INT8_MAX` | `int8` | `127` | `__INT8_MAX__` |
| `INT16_MAX` | `int16` | `32'767` | `__INT16_MAX__` |
| `INT32_MAX` | `int32` | `2'147'483'647` | `__INT32_MAX__` |
| `INT64_MAX` | `int64` | `9'223'372'036'854'775'807` | `__INT64_MAX__` |
| `UINT8_MAX` | `uint8` | `255` | `__UINT8_MAX__` |
| `UINT16_MAX` | `uint16` | `65'535` | `__UINT16_MAX__` |
| `UINT32_MAX` | `uint32` | `4'294'967'295` | `__UINT32_MAX__` |
| `UINT64_MAX` | `uint64` | `18'446'744'073'709'551'615` | `__UINT64_MAX__` |

---

### Floating-Point Limits

| Constant | Type | Value | Source |
|---|---|---|---|
| `FLOAT16_MAX` | `float32` | `65'504` | `__FLT16_MAX__` |
| `FLOAT32_MAX` | `float32` | ≈ `3.4028235e+38` | `__FLT_MAX__` |
| `FLOAT64_MAX` | `float32` | see note | `__DBL_MAX__` |

---

### Memory Size Helpers

Compile-time helpers that convert a count of units into a number of bytes.
Each is a `constexpr` function template, constrained by
[`sage::template_types::numeric_types`](#numeric_types), returning either `usize` or `double`.

```cpp
template<sage::template_types::unsigned_numeric_type T>
constexpr auto KiB(T value) -> usize | double;
```

| Function | Unit | Multiplier | Bytes for `1` |
|---|---|---|---|
| `KiB(value)` | Kibibyte | 1,024 | `1'024` |
| `MiB(value)` | Mebibyte | 1,024² | `1'048'576` |
| `GiB(value)` | Gibibyte | 1,024³ | `1'073'741'824` |
| `KB(value)` | Kilobyte | 1,000 | `1'000` |
| `MB(value)` | Megabyte | 1,000² | `1'000'000` |
| `GB(value)` | Gigabyte | 1,000³ | `1'000'000'000` |

```cpp
import sage;

using namespace sage::numerics;

constexpr usize stack_size  = KiB(64);   //     65'536 bytes
constexpr usize arena_size  = MiB(16);   // 16'777'216 bytes
constexpr usize packet_size = KB(1);     //      1'000 bytes

static_assert(KiB(1) == 1'024);
```

---

## Template Types

Reusable concepts for constraining templates, defined in
`sage::template_types` (module `sage:core_template_types`).

| Concept | Description |
|---|---|
| [`numeric_types`](#numeric_types) | The type supports the full set of integer arithmetic and shift operators. |

---

### numeric_types

```cpp
template<typename T>
concept numeric_type = std::integral<T> || std::floating_point<T>;

template<typename T>
concept unsigned_numeric_type = std::unsigned_integral<T> || std::floating_point<T>;
};
```

A type `T` satisfies `numeric_type` when all of the following hold:

| Requirement | Meaning |
|---|---|
| `value * 1`, `value + 1`, `value - 1`, `value / 1` | The four basic arithmetic operators are valid with an `int` operand. |
| `std::is_arithmetic_v<decltype(value * 1)>` | The result of `value * 1` is an arithmetic type. |

| Type | Satisfies | Reason |
|---|---|---|
| `int`, `unsigned`, `long`, `uint64`, `usize` | ✅ | All operators valid |
| `char`, `bool` | ✅ | Promote to `int` |
| Unscoped `enum` | ✅ | Converts to an integer type |
| `float`, `double` | ✅| Converts to floating-point types |
| Scoped `enum class` | ❌ | No implicit arithmetic |
| Pointers | ❌ | `*` and `/` are not defined |

**Usage**

```cpp
import sage;

template<sage::template_types::numeric_type T>
constexpr auto halve(T value) -> T {
    return value / 2;
}
template<sage::template_types::unsigned_numeric_type T>
constexpr auto add_one(T value) -> T {
    return value + 1;
}

static_assert(halve(10) == 5); // Pass
static_assert(halve("10") == 5); // Fail
static_assert(add_one(3) == 4); // Pass
static_assert(add_one(-5) == -4); // Fail
```

---

## Quick Reference

| Goal | Use |
|---|---|
| Exact-width integer | `int8` … `int64`, `uint8` … `uint64` |
| 128-bit integer (glibc only) | `int128`, `uint128` |
| Smallest portable type holding N bits | `int8_least` … `uint64_least` |
| Fastest type holding N bits | `int8_fast` … `uint64_fast` |
| Widest integer on the platform | `int_max`, `uint_max` |
| Sizes and indices | `usize`, `isize` |
| Pointer-width integer | `intptr`, `uintptr` |
| Characters | `char8`, `char16`, `char32`, `uchar8`, `uchar16`, `uchar32` |
| Raw bytes | `byte` |
| Boolean | `bool8` |
| Floating-point | `float32`, `float64`, `float80_least` |
| Keep aliases out of the global namespace | `SAGE_TYPES_SCOPE_ENBALED` |
| Classify a log message | `sage::Severity` |
| Max value of an integer type | `sage::numerics::INT32_MAX`, `UINT8_MAX`, … |
| Max value of a float type | `sage::numerics::FLOAT32_MAX`, … |
| Binary byte sizes (1,024-based) | `sage::numerics::KiB`, `MiB`, `GiB` |
| Decimal byte sizes (1,000-based) | `sage::numerics::KB`, `MB`, `GB` |
| Constrain a template to integer-like types | `sage::template_types::numeric_type` |
