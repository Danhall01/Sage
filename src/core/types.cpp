module;
#include <limits>
#include <type_traits>
#if defined(__GLIBC__)
    #include <bits/types.h>
#else
    #include <cstdint>
#endif
export module sage:core_types;
export namespace sage::types
{
#if defined(__GLIBC__)
// Fixed-width

using int8   = __int8_t;
using int16  = __int16_t;
using int32  = __int32_t;
using int64  = __int64_t;
using int128 = __int128_t;

using uint8   = __uint8_t;
using uint16  = __uint16_t;
using uint32  = __uint32_t;
using uint64  = __uint64_t;
using uint128 = __uint128_t;

// Least-width (Prefer lower size, but use what fits. Will always compile, unlike e.g int32)

using int8_least  = __int_least8_t;
using int16_least = __int_least16_t;
using int32_least = __int_least32_t;
using int64_least = __int_least64_t;

using uint8_least  = __uint_least8_t;
using uint16_least = __uint_least16_t;
using uint32_least = __uint_least32_t;
using uint64_least = __uint_least64_t;

// Fast-width (Prefer faster size over smaller size)

using int8_fast  = __INT_FAST8_TYPE__;
using int16_fast = __INT_FAST16_TYPE__;
using int32_fast = __INT_FAST32_TYPE__;
using int64_fast = __INT_FAST64_TYPE__;

using uint8_fast  = __UINT_FAST8_TYPE__;
using uint16_fast = __UINT_FAST16_TYPE__;
using uint32_fast = __UINT_FAST32_TYPE__;
using uint64_fast = __UINT_FAST64_TYPE__;

// The largest size int supported by the system
using int_max  = __intmax_t;
// The largest size unsigned int supported by the system
using uint_max = __uintmax_t;

using intptr  = __intptr_t;
using uintptr = std::make_unsigned_t<__intptr_t>;

using usize = __SIZE_TYPE__;
using isize = std::make_signed_t<usize>;

#else  // Use Cpp fallbacks for types rather than bits/types
// Fixed-width

using int8  = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8  = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

// Least-width

using int8_least  = std::int_least8_t;
using int16_least = std::int_least16_t;
using int32_least = std::int_least32_t;
using int64_least = std::int_least64_t;

using uint8_least  = std::uint_least8_t;
using uint16_least = std::uint_least16_t;
using uint32_least = std::uint_least32_t;
using uint64_least = std::uint_least64_t;

// Fast-width

using int8_fast  = std::int_fast8_t;
using int16_fast = std::int_fast16_t;
using int32_fast = std::int_fast32_t;
using int64_fast = std::int_fast64_t;

using uint8_fast  = std::uint_fast8_t;
using uint16_fast = std::uint_fast16_t;
using uint32_fast = std::uint_fast32_t;
using uint64_fast = std::uint_fast64_t;

// Max-width

using int_max  = std::intmax_t;
using uint_max = std::uintmax_t;

using intptr  = std::intptr_t;
using uintptr = std::uintptr_t;

using usize = std::size_t;
using isize = std::make_signed_t<usize>;
#endif

// Char type

static_assert(sizeof(char) == 1, "Platform does not support 8-bit char type");

using char8  = char;
using char16 = __CHAR16_TYPE__;
using char32 = __CHAR32_TYPE__;

using uchar8  = std::make_unsigned_t<char8>;
using uchar16 = std::make_unsigned_t<char16>;
using uchar32 = std::make_unsigned_t<char32>;

// Misc

using bool8 = bool;
using byte  = uchar8;

// Floating-point

static_assert(sizeof(float) == 4, "Platform does not support 32-bit floating-point type");
static_assert(std::numeric_limits<float>::is_iec559);
static_assert(sizeof(double) == 8, "Platform does not support 64-bit floating-point type");
static_assert(std::numeric_limits<double>::is_iec559);

using float32       = float;
using float64       = double;
// This type is defined to be at least 80 bit, however on most systems it is 128, there is no way to
// guarantee this size be always consistent
using float80_least = long double;

}  // namespace sage::types

#ifndef SAGE_TYPES_SCOPE_ENABLED
using namespace sage::types;
#endif
