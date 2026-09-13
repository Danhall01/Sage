module;
#include <limits>
#include <type_traits>
#if defined(__GLIBC__)
    #include <bits/types.h>
#else
    #include <cstdint>
#endif
export module sage:types;
namespace sage::types
{
#if defined(__GLIBC__)
// Fixed-width
using i8   = __int8_t;
using i16  = __int16_t;
using i32  = __int32_t;
using i64  = __int64_t;
using i128 = __int128_t;

using u8   = __uint8_t;
using u16  = __uint16_t;
using u32  = __uint32_t;
using u64  = __uint64_t;
using u128 = __uint128_t;

// Least-width
using i8_least  = __int_least8_t;
using i16_least = __int_least16_t;
using i32_least = __int_least32_t;
using i64_least = __int_least64_t;

using u8_least  = __uint_least8_t;
using u16_least = __uint_least16_t;
using u32_least = __uint_least32_t;
using u64_least = __uint_least64_t;

// Fast-width
using i8_fast  = __INT_FAST8_TYPE__;
using i16_fast = __INT_FAST16_TYPE__;
using i32_fast = __INT_FAST32_TYPE__;
using i64_fast = __INT_FAST64_TYPE__;

using u8_fast  = __UINT_FAST8_TYPE__;
using u16_fast = __UINT_FAST16_TYPE__;
using u32_fast = __UINT_FAST32_TYPE__;
using u64_fast = __UINT_FAST64_TYPE__;

// Max-width
using imax = __intmax_t;
using umax = __uintmax_t;

using intptr  = __intptr_t;
using uintptr = std::make_unsigned_t<__intptr_t>;

using usize = __SIZE_TYPE__;
using isize = std::make_signed_t<usize>;

#else  // Use Cpp fallbacks for types rather than bits/types
// Fixed-width
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Least-width
using i8_least  = std::int_least8_t;
using i16_least = std::int_least16_t;
using i32_least = std::int_least32_t;
using i64_least = std::int_least64_t;

using u8_least  = std::uint_least8_t;
using u16_least = std::uint_least16_t;
using u32_least = std::uint_least32_t;
using u64_least = std::uint_least64_t;

// Fast-width
using i8_fast  = std::int_fast8_t;
using i16_fast = std::int_fast16_t;
using i32_fast = std::int_fast32_t;
using i64_fast = std::int_fast64_t;

using u8_fast  = std::uint_fast8_t;
using u16_fast = std::uint_fast16_t;
using u32_fast = std::uint_fast32_t;
using u64_fast = std::uint_fast64_t;

// Max-width
using imax = std::intmax_t;
using umax = std::uintmax_t;

using intptr  = std::intptr_t;
using uintptr = std::uintptr_t;

using usize = std::size_t;
using isize = std::make_signed_t<usize>;
#endif

// Char type
static_assert(sizeof(char) == 1, "Platform does not support 8-bit char type");
using c8  = char;
using c16 = __CHAR16_TYPE__;
using c32 = __CHAR32_TYPE__;

// Boolean
using b8 = bool;

// Floating-point
static_assert(sizeof(float) == 4, "Platform does not support 32-bit floating-point type");
static_assert(std::numeric_limits<float>::is_iec559);
static_assert(sizeof(double) == 8, "Platform does not support 64-bit floating-point type");
static_assert(std::numeric_limits<double>::is_iec559);
using f32       = float;
using f64       = double;
using f80_least = long double;
}  // namespace sage::types
