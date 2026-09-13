# sage::types

Type aliases defined in `sage::types` (module `sage:types`).

| Type | Category | Underlying Type | Notes |
|---|---|---|---|
| `i8` | Fixed-width signed | 8-bit int | |
| `i16` | Fixed-width signed | 16-bit int | |
| `i32` | Fixed-width signed | 32-bit int | |
| `i64` | Fixed-width signed | 64-bit int | |
| `i128` | Fixed-width signed | 128-bit int | glibc only |
| `u8` | Fixed-width unsigned | 8-bit uint | |
| `u16` | Fixed-width unsigned | 16-bit uint | |
| `u32` | Fixed-width unsigned | 32-bit uint | |
| `u64` | Fixed-width unsigned | 64-bit uint | |
| `u128` | Fixed-width unsigned | 128-bit uint | glibc only |
| `i8_least` | Least-width signed | ≥8-bit int | |
| `i16_least` | Least-width signed | ≥16-bit int | |
| `i32_least` | Least-width signed | ≥32-bit int | |
| `i64_least` | Least-width signed | ≥64-bit int | |
| `u8_least` | Least-width unsigned | ≥8-bit uint | |
| `u16_least` | Least-width unsigned | ≥16-bit uint | |
| `u32_least` | Least-width unsigned | ≥32-bit uint | |
| `u64_least` | Least-width unsigned | ≥64-bit uint | |
| `i8_fast` | Fast-width signed | ≥8-bit int | fastest type ≥ width |
| `i16_fast` | Fast-width signed | ≥16-bit int | fastest type ≥ width |
| `i32_fast` | Fast-width signed | ≥32-bit int | fastest type ≥ width |
| `i64_fast` | Fast-width signed | ≥64-bit int | fastest type ≥ width |
| `u8_fast` | Fast-width unsigned | ≥8-bit uint | fastest type ≥ width |
| `u16_fast` | Fast-width unsigned | ≥16-bit uint | fastest type ≥ width |
| `u32_fast` | Fast-width unsigned | ≥32-bit uint | fastest type ≥ width |
| `u64_fast` | Fast-width unsigned | ≥64-bit uint | fastest type ≥ width |
| `imax` | Max-width signed | largest available int | |
| `umax` | Max-width unsigned | largest available uint | |
| `intptr` | Pointer-sized signed | signed pointer-width int | |
| `uintptr` | Pointer-sized unsigned | unsigned pointer-width int | |
| `usize` | Size | unsigned, pointer-width | result of `sizeof` |
| `isize` | Size | signed, pointer-width | signed counterpart of `usize` |
| `c8` | Character | `char` | 8-bit, enforced via `static_assert` |
| `c16` | Character | `__CHAR16_TYPE__` | |
| `c32` | Character | `__CHAR32_TYPE__` | |
| `b8` | Boolean | `bool` | |
| `f32` | Floating-point | `float` | 32-bit, IEC 559, enforced via `static_assert` |
| `f64` | Floating-point | `double` | 64-bit, IEC 559, enforced via `static_assert` |
| `f80_least` | Floating-point | `long double` | extended precision, width platform-dependent |

## Platform Notes

- On glibc platforms, fixed/least/fast/max-width and pointer/size types come from `<bits/types.h>` (`__int8_t`, `__uint_least32_t`, etc.).
- On non-glibc platforms, these fall back to standard `<cstdint>` types (`std::int8_t`, `std::uintmax_t`, etc.).
- `i128`/`u128` are only defined under glibc (`__int128_t`/`__uint128_t`); no portable fallback is provided.
