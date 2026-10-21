#include "gtest/gtest.h"
#include <bit>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>

import sage;

namespace
{
namespace types          = sage::types;
namespace limits         = sage::limits;
namespace numerics       = sage::numerics;
namespace template_types = sage::template_types;

enum UnscopedEnum
{
    UnscopedA,
    UnscopedB
};
enum class ScopedEnum
{
    A,
    B
};
struct PlainStruct
{
};

// Detection helpers. The tested name is made dependent on a template parameter so a missing
// member or a rejected argument turns into `false` instead of a compile error.
template<typename T>
concept has_lowest = requires { T::lowest(); };
template<typename T>
concept can_limit_of = requires { typename sage::limits::limit_of<T>; };

template<typename T>
concept accepts_KiB = requires(T v) { sage::numerics::KiB(v); };
template<typename T>
concept accepts_MiB = requires(T v) { sage::numerics::MiB(v); };
template<typename T>
concept accepts_GiB = requires(T v) { sage::numerics::GiB(v); };
template<typename T>
concept accepts_KB = requires(T v) { sage::numerics::KB(v); };
template<typename T>
concept accepts_MB = requires(T v) { sage::numerics::MB(v); };
template<typename T>
concept accepts_GB = requires(T v) { sage::numerics::GB(v); };

template<typename E>
concept has_Trace = requires { E::Trace; };
template<typename E>
concept has_Debug = requires { E::Debug; };
template<typename E>
concept has_Info = requires { E::Info; };
template<typename E>
concept has_Warn = requires { E::Warn; };
template<typename E>
concept has_Error = requires { E::Error; };
template<typename E>
concept has_Fatal = requires { E::Fatal; };

constexpr auto kib = [](auto v)
{
    return sage::numerics::KiB(v);
};
constexpr auto mib = [](auto v)
{
    return sage::numerics::MiB(v);
};
constexpr auto gib = [](auto v)
{
    return sage::numerics::GiB(v);
};
constexpr auto kb = [](auto v)
{
    return sage::numerics::KB(v);
};
constexpr auto mb = [](auto v)
{
    return sage::numerics::MB(v);
};
constexpr auto gb = [](auto v)
{
    return sage::numerics::GB(v);
};

// Checks `fn(count)` against the mathematically correct byte count, computed in 64 bit.
template<typename Fn, typename T>
void expect_bytes(const char* name, Fn fn, T count, types::usize multiplier)
{
    static_assert(sizeof(types::usize) >= 8, "expectations need a 64-bit usize");
    const types::usize expected = static_cast<types::usize>(count) * multiplier;
    EXPECT_EQ(fn(count), expected) << name << "(" << static_cast<unsigned long long>(count)
                                   << ") using a " << sizeof(T) * CHAR_BIT << "-bit argument";
}

constexpr types::uint8 bit(sage::Severity severity) { return static_cast<types::uint8>(severity); }

// The mask example from docs/core_docs.md (with the enumerator spelling of the implementation).
constexpr types::uint8 info_and_above = bit(sage::Severity::Info) | bit(sage::Severity::Warn)
                                        | bit(sage::Severity::Error) | bit(sage::Severity::Fatal);
constexpr bool is_enabled(sage::Severity level) { return (info_and_above & bit(level)) != 0; }
static_assert(info_and_above == 0b11'1100);
static_assert(!is_enabled(sage::Severity::Debug));
static_assert(is_enabled(sage::Severity::Error));

// Overload set used to check that unsigned_numeric_type is the more constrained concept.
constexpr int which(template_types::numeric_type auto) { return 0; }
constexpr int which(template_types::unsigned_numeric_type auto) { return 1; }

#define EXPECT_SATISFIES(concept_name, ...) \
    EXPECT_TRUE((template_types::concept_name<__VA_ARGS__>)) << #__VA_ARGS__
#define EXPECT_NOT_SATISFIES(concept_name, ...) \
    EXPECT_FALSE((template_types::concept_name<__VA_ARGS__>)) << #__VA_ARGS__
}  // namespace

// ================================================================================================
// Types
// ================================================================================================

TEST(CoreTypes, FixedWidthIntegersHaveExactSize)
{
    EXPECT_EQ(sizeof(types::int8), 1U);
    EXPECT_EQ(sizeof(types::int16), 2U);
    EXPECT_EQ(sizeof(types::int32), 4U);
    EXPECT_EQ(sizeof(types::int64), 8U);
    EXPECT_EQ(sizeof(types::uint8), 1U);
    EXPECT_EQ(sizeof(types::uint16), 2U);
    EXPECT_EQ(sizeof(types::uint32), 4U);
    EXPECT_EQ(sizeof(types::uint64), 8U);
}

TEST(CoreTypes, FixedWidthIntegersHaveCorrectSignedness)
{
    EXPECT_TRUE(std::is_signed_v<types::int8>);
    EXPECT_TRUE(std::is_signed_v<types::int16>);
    EXPECT_TRUE(std::is_signed_v<types::int32>);
    EXPECT_TRUE(std::is_signed_v<types::int64>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint8>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint16>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint32>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint64>);
}

TEST(CoreTypes, FixedWidthIntegersCoverTheFullRange)
{
    EXPECT_EQ(std::numeric_limits<types::int8>::digits, 7);
    EXPECT_EQ(std::numeric_limits<types::int16>::digits, 15);
    EXPECT_EQ(std::numeric_limits<types::int32>::digits, 31);
    EXPECT_EQ(std::numeric_limits<types::int64>::digits, 63);
    EXPECT_EQ(std::numeric_limits<types::uint8>::digits, 8);
    EXPECT_EQ(std::numeric_limits<types::uint16>::digits, 16);
    EXPECT_EQ(std::numeric_limits<types::uint32>::digits, 32);
    EXPECT_EQ(std::numeric_limits<types::uint64>::digits, 64);
}

TEST(CoreTypes, FixedWidthIntegersWrapAroundLikeTheStandardTypes)
{
    EXPECT_EQ(static_cast<types::uint8>(256), 0);
    EXPECT_EQ(static_cast<types::uint8>(-1), 255);
    EXPECT_EQ(static_cast<types::uint16>(65'536), 0);
    EXPECT_EQ(static_cast<types::uint32>(-1), 4'294'967'295U);
    EXPECT_EQ(static_cast<types::uint64>(-1), 18'446'744'073'709'551'615ULL);
    EXPECT_EQ(static_cast<types::int8>(static_cast<types::uint8>(0x80)), -128);
}

TEST(CoreTypes, FixedWidthIntegersMatchTheStandardTypes)
{
    EXPECT_TRUE((std::is_same_v<types::int8, std::int8_t>));
    EXPECT_TRUE((std::is_same_v<types::int16, std::int16_t>));
    EXPECT_TRUE((std::is_same_v<types::int32, std::int32_t>));
    EXPECT_TRUE((std::is_same_v<types::int64, std::int64_t>));
    EXPECT_TRUE((std::is_same_v<types::uint8, std::uint8_t>));
    EXPECT_TRUE((std::is_same_v<types::uint16, std::uint16_t>));
    EXPECT_TRUE((std::is_same_v<types::uint32, std::uint32_t>));
    EXPECT_TRUE((std::is_same_v<types::uint64, std::uint64_t>));
}

#if defined(__GLIBC__)
TEST(CoreTypes, Int128IsAvailableOnGlibc)
{
    EXPECT_EQ(sizeof(types::int128), 16U);
    EXPECT_EQ(sizeof(types::uint128), 16U);
    EXPECT_LT(static_cast<types::int128>(-1), 0);
    EXPECT_GT(static_cast<types::uint128>(-1), 0);

    const types::uint128 top_bit = static_cast<types::uint128>(1) << 127;
    EXPECT_NE(top_bit, 0);
    EXPECT_EQ(top_bit << 1, 0);  // unsigned wrap-around at exactly 128 bit

    const types::uint128 all_ones = static_cast<types::uint128>(-1);
    EXPECT_EQ(
        static_cast<types::uint64>(all_ones >> 64),
        std::numeric_limits<types::uint64>::max()
    );
    EXPECT_EQ(static_cast<types::uint64>(all_ones), std::numeric_limits<types::uint64>::max());
}
#endif

TEST(CoreTypes, LeastWidthIntegersHoldAtLeastTheirWidth)
{
    EXPECT_GE(sizeof(types::int8_least) * CHAR_BIT, 8U);
    EXPECT_GE(sizeof(types::int16_least) * CHAR_BIT, 16U);
    EXPECT_GE(sizeof(types::int32_least) * CHAR_BIT, 32U);
    EXPECT_GE(sizeof(types::int64_least) * CHAR_BIT, 64U);
    EXPECT_GE(sizeof(types::uint8_least) * CHAR_BIT, 8U);
    EXPECT_GE(sizeof(types::uint16_least) * CHAR_BIT, 16U);
    EXPECT_GE(sizeof(types::uint32_least) * CHAR_BIT, 32U);
    EXPECT_GE(sizeof(types::uint64_least) * CHAR_BIT, 64U);
}

TEST(CoreTypes, LeastWidthIntegersHaveCorrectSignedness)
{
    EXPECT_TRUE(std::is_signed_v<types::int8_least>);
    EXPECT_TRUE(std::is_signed_v<types::int16_least>);
    EXPECT_TRUE(std::is_signed_v<types::int32_least>);
    EXPECT_TRUE(std::is_signed_v<types::int64_least>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint8_least>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint16_least>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint32_least>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint64_least>);
}

TEST(CoreTypes, LeastWidthIntegersMatchTheStandardTypes)
{
    EXPECT_TRUE((std::is_same_v<types::int8_least, std::int_least8_t>));
    EXPECT_TRUE((std::is_same_v<types::int16_least, std::int_least16_t>));
    EXPECT_TRUE((std::is_same_v<types::int32_least, std::int_least32_t>));
    EXPECT_TRUE((std::is_same_v<types::int64_least, std::int_least64_t>));
    EXPECT_TRUE((std::is_same_v<types::uint8_least, std::uint_least8_t>));
    EXPECT_TRUE((std::is_same_v<types::uint16_least, std::uint_least16_t>));
    EXPECT_TRUE((std::is_same_v<types::uint32_least, std::uint_least32_t>));
    EXPECT_TRUE((std::is_same_v<types::uint64_least, std::uint_least64_t>));
}

TEST(CoreTypes, FastWidthIntegersHoldAtLeastTheirWidth)
{
    EXPECT_GE(sizeof(types::int8_fast) * CHAR_BIT, 8U);
    EXPECT_GE(sizeof(types::int16_fast) * CHAR_BIT, 16U);
    EXPECT_GE(sizeof(types::int32_fast) * CHAR_BIT, 32U);
    EXPECT_GE(sizeof(types::int64_fast) * CHAR_BIT, 64U);
    EXPECT_GE(sizeof(types::uint8_fast) * CHAR_BIT, 8U);
    EXPECT_GE(sizeof(types::uint16_fast) * CHAR_BIT, 16U);
    EXPECT_GE(sizeof(types::uint32_fast) * CHAR_BIT, 32U);
    EXPECT_GE(sizeof(types::uint64_fast) * CHAR_BIT, 64U);
}

TEST(CoreTypes, FastWidthIntegersHaveCorrectSignedness)
{
    EXPECT_TRUE(std::is_signed_v<types::int8_fast>);
    EXPECT_TRUE(std::is_signed_v<types::int16_fast>);
    EXPECT_TRUE(std::is_signed_v<types::int32_fast>);
    EXPECT_TRUE(std::is_signed_v<types::int64_fast>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint8_fast>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint16_fast>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint32_fast>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint64_fast>);
}

TEST(CoreTypes, FastWidthIntegersAreNeverNarrowerThanLeastWidth)
{
    EXPECT_GE(sizeof(types::int8_fast), sizeof(types::int8_least));
    EXPECT_GE(sizeof(types::int16_fast), sizeof(types::int16_least));
    EXPECT_GE(sizeof(types::int32_fast), sizeof(types::int32_least));
    EXPECT_GE(sizeof(types::int64_fast), sizeof(types::int64_least));
    EXPECT_GE(sizeof(types::uint8_fast), sizeof(types::uint8_least));
    EXPECT_GE(sizeof(types::uint16_fast), sizeof(types::uint16_least));
    EXPECT_GE(sizeof(types::uint32_fast), sizeof(types::uint32_least));
    EXPECT_GE(sizeof(types::uint64_fast), sizeof(types::uint64_least));
}

TEST(CoreTypes, MaxWidthIntegersAreTheWidestAvailable)
{
    EXPECT_TRUE((std::is_same_v<types::int_max, std::intmax_t>));
    EXPECT_TRUE((std::is_same_v<types::uint_max, std::uintmax_t>));
    EXPECT_TRUE(std::is_signed_v<types::int_max>);
    EXPECT_TRUE(std::is_unsigned_v<types::uint_max>);
    EXPECT_GE(sizeof(types::int_max), sizeof(types::int64));
    EXPECT_GE(sizeof(types::uint_max), sizeof(types::uint64));
    EXPECT_GE(sizeof(types::int_max), sizeof(types::int64_fast));
    EXPECT_GE(sizeof(types::uint_max), sizeof(types::uint64_least));
    EXPECT_EQ(sizeof(types::int_max), sizeof(types::uint_max));
}

TEST(CoreTypes, PointerSizedIntegersMatchThePointerWidth)
{
    EXPECT_EQ(sizeof(types::intptr), sizeof(void*));
    EXPECT_EQ(sizeof(types::uintptr), sizeof(void*));
    EXPECT_TRUE(std::is_signed_v<types::intptr>);
    EXPECT_TRUE(std::is_unsigned_v<types::uintptr>);
}

TEST(CoreTypes, PointerSizedIntegersRoundTripPointers)
{
    int  value = 0;
    int* ptr   = &value;

    const auto as_unsigned = reinterpret_cast<types::uintptr>(ptr);
    EXPECT_EQ(reinterpret_cast<int*>(as_unsigned), ptr);

    const auto as_signed = reinterpret_cast<types::intptr>(ptr);
    EXPECT_EQ(reinterpret_cast<int*>(as_signed), ptr);

    EXPECT_EQ(reinterpret_cast<types::uintptr>(static_cast<void*>(nullptr)), 0U);
}

TEST(CoreTypes, SizeTypesMatchSizeof)
{
    EXPECT_TRUE((std::is_same_v<types::usize, decltype(sizeof(int))>));
    EXPECT_TRUE((std::is_same_v<types::usize, std::size_t>));
    EXPECT_TRUE(std::is_unsigned_v<types::usize>);
    EXPECT_EQ(sizeof(types::usize), sizeof(void*));
}

TEST(CoreTypes, IsizeIsTheSignedCounterpartOfUsize)
{
    EXPECT_TRUE(std::is_signed_v<types::isize>);
    EXPECT_EQ(sizeof(types::isize), sizeof(types::usize));
    EXPECT_TRUE((std::is_same_v<types::isize, std::make_signed_t<types::usize>>));
    EXPECT_EQ(sizeof(types::isize), sizeof(std::ptrdiff_t));

    const types::isize negative = -1;
    EXPECT_LT(negative, 0);
    EXPECT_EQ(static_cast<types::usize>(negative), std::numeric_limits<types::usize>::max());
}

TEST(CoreTypes, UsizeHoldsTheSizeOfLargeObjects)
{
    constexpr types::usize big = sizeof(char[1'000'000]);
    EXPECT_EQ(big, 1'000'000U);
}

TEST(CoreTypes, CharacterTypeSizes)
{
    EXPECT_EQ(sizeof(types::char8), 1U);
    EXPECT_EQ(sizeof(types::char16), 2U);
    EXPECT_EQ(sizeof(types::char32), 4U);
    EXPECT_EQ(sizeof(types::uchar8), 1U);
    EXPECT_EQ(sizeof(types::uchar16), 2U);
    EXPECT_EQ(sizeof(types::uchar32), 4U);
}

TEST(CoreTypes, CharacterTypesMapToTheCppCharacterTypes)
{
    EXPECT_TRUE((std::is_same_v<types::char16, char16_t>));
    EXPECT_TRUE((std::is_same_v<types::char32, char32_t>));
}

TEST(CoreTypes, UnsignedCharacterTypesAreUnsignedCounterparts)
{
    EXPECT_TRUE(std::is_unsigned_v<types::uchar8>);
    EXPECT_TRUE(std::is_unsigned_v<types::uchar16>);
    EXPECT_TRUE(std::is_unsigned_v<types::uchar32>);
    EXPECT_TRUE((std::is_same_v<types::uchar8, std::make_unsigned_t<types::char8>>));
    EXPECT_TRUE((std::is_same_v<types::uchar16, std::make_unsigned_t<types::char16>>));
    EXPECT_TRUE((std::is_same_v<types::uchar32, std::make_unsigned_t<types::char32>>));
    EXPECT_EQ(std::numeric_limits<types::uchar8>::max(), 0xFFU);
    EXPECT_EQ(std::numeric_limits<types::uchar16>::max(), 0xFF'FFU);
    EXPECT_EQ(std::numeric_limits<types::uchar32>::max(), 0xFF'FF'FF'FFU);
}

TEST(CoreTypes, Bool8IsBool)
{
    EXPECT_TRUE((std::is_same_v<types::bool8, bool>));
    EXPECT_TRUE(static_cast<types::bool8>(2));
    EXPECT_FALSE(static_cast<types::bool8>(0));
}

TEST(CoreTypes, ByteIsRawUnsigned8BitData)
{
    EXPECT_TRUE((std::is_same_v<types::byte, types::uchar8>));
    EXPECT_EQ(sizeof(types::byte), 1U);
    EXPECT_TRUE(std::is_unsigned_v<types::byte>);
    EXPECT_EQ(std::numeric_limits<types::byte>::min(), 0U);
    EXPECT_EQ(std::numeric_limits<types::byte>::max(), 255U);

    EXPECT_EQ(static_cast<types::byte>(256), 0);
    EXPECT_EQ(static_cast<types::byte>(-1), 255);
    types::byte raw = 0xFF;
    ++raw;
    EXPECT_EQ(raw, 0);
}

TEST(CoreTypes, ByteCanAliasObjectRepresentations)
{
    const types::uint32 value = 0x01'02'03'04U;
    types::byte         bytes[sizeof(value)];
    std::memcpy(bytes, &value, sizeof(value));

    types::uint32 round_trip = 0;
    std::memcpy(&round_trip, bytes, sizeof(round_trip));
    EXPECT_EQ(round_trip, value);
}

TEST(CoreTypes, Float32IsIeee754Single)
{
    EXPECT_TRUE((std::is_same_v<types::float32, float>));
    EXPECT_EQ(sizeof(types::float32), 4U);
    EXPECT_TRUE(std::numeric_limits<types::float32>::is_iec559);
    EXPECT_EQ(std::numeric_limits<types::float32>::digits, 24);
}

TEST(CoreTypes, Float64IsIeee754Double)
{
    EXPECT_TRUE((std::is_same_v<types::float64, double>));
    EXPECT_EQ(sizeof(types::float64), 8U);
    EXPECT_TRUE(std::numeric_limits<types::float64>::is_iec559);
    EXPECT_EQ(std::numeric_limits<types::float64>::digits, 53);
}

TEST(CoreTypes, Float64LeastIsLongDouble)
{
    EXPECT_TRUE((std::is_same_v<types::float64_least, long double>));
    EXPECT_GE(sizeof(types::float64_least), sizeof(types::float64));
    EXPECT_GE(
        std::numeric_limits<types::float64_least>::digits,
        std::numeric_limits<types::float64>::digits
    );
}

#if defined(__GLIBC__)
// "At least 80-bit" holds on glibc targets. The docs also note that on Windows `long double` is
// just `double`, so this is deliberately not asserted there.
TEST(CoreTypes, Float64LeastHasExtendedPrecisionOnGlibc)
{
    EXPECT_GE(std::numeric_limits<types::float64_least>::digits, 64);
    EXPECT_GE(sizeof(types::float64_least), 10U);
}
#endif

TEST(CoreTypes, AliasesAreUsableInConstantExpressions)
{
    constexpr types::int32       a = 1;
    constexpr types::uint8       b = 255;
    constexpr types::usize       c = sizeof(a);
    constexpr types::byte        d = 0xFF;
    constexpr types::float64     e = 0.5;
    constexpr types::int32_least f = 3;
    constexpr types::int32_fast  g = 4;
    EXPECT_EQ(a + b + c + d + f + g, 1 + 255 + 4 + 255 + 3 + 4);
    EXPECT_DOUBLE_EQ(e, 0.5);
}

#ifndef SAGE_TYPES_SCOPE_ENABLED
TEST(CoreTypes, AliasesAreAvailableUnqualifiedByDefault)
{
    int32       count  = 0;
    usize       length = sizeof(count);
    byte        raw    = 0xFF;
    float64     ratio  = 0.5;
    int32_least small  = 0;
    int32_fast  loop   = 0;
    EXPECT_EQ(length, 4U);
    EXPECT_EQ(raw, 255);
    EXPECT_DOUBLE_EQ(ratio, 0.5);
    EXPECT_EQ(count + small + loop, 0);
}
#endif

// ================================================================================================
// Severity
// ================================================================================================

TEST(CoreSeverity, UnderlyingTypeIsUint8)
{
    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<sage::Severity>, types::uint8>));
    EXPECT_EQ(sizeof(sage::Severity), 1U);
}

TEST(CoreSeverity, EnumeratorsHaveTheDocumentedValues)
{
    EXPECT_EQ(bit(sage::Severity::Trace), 0b00'0001);
    EXPECT_EQ(bit(sage::Severity::Debug), 0b00'0010);
    EXPECT_EQ(bit(sage::Severity::Info), 0b00'0100);
    EXPECT_EQ(bit(sage::Severity::Warn), 0b00'1000);
    EXPECT_EQ(bit(sage::Severity::Error), 0b01'0000);
    EXPECT_EQ(bit(sage::Severity::Fatal), 0b10'0000);
}

TEST(CoreSeverity, EveryEnumeratorOccupiesExactlyOneBit)
{
    for (auto level :
         {sage::Severity::Trace,
          sage::Severity::Debug,
          sage::Severity::Info,
          sage::Severity::Warn,
          sage::Severity::Error,
          sage::Severity::Fatal})
    {
        EXPECT_EQ(std::popcount(bit(level)), 1) << "value " << static_cast<int>(bit(level));
    }
}

TEST(CoreSeverity, BitsDoNotOverlap)
{
    const sage::Severity levels[] = {
        sage::Severity::Trace,
        sage::Severity::Debug,
        sage::Severity::Info,
        sage::Severity::Warn,
        sage::Severity::Error,
        sage::Severity::Fatal
    };
    for (std::size_t i = 0; i < std::size(levels); ++i)
        for (std::size_t j = i + 1; j < std::size(levels); ++j)
            EXPECT_EQ(bit(levels[i]) & bit(levels[j]), 0) << i << " overlaps " << j;
}

TEST(CoreSeverity, LevelsRunFromMostToLeastVerbose)
{
    EXPECT_LT(sage::Severity::Trace, sage::Severity::Debug);
    EXPECT_LT(sage::Severity::Debug, sage::Severity::Info);
    EXPECT_LT(sage::Severity::Info, sage::Severity::Warn);
    EXPECT_LT(sage::Severity::Warn, sage::Severity::Error);
    EXPECT_LT(sage::Severity::Error, sage::Severity::Fatal);
    EXPECT_GE(sage::Severity::Fatal, sage::Severity::Warn);
    EXPECT_FALSE(sage::Severity::Trace >= sage::Severity::Debug);
}

TEST(CoreSeverity, AllLevelsCombineToTheFullMask)
{
    const types::uint8 all = bit(sage::Severity::Trace) | bit(sage::Severity::Debug)
                             | bit(sage::Severity::Info) | bit(sage::Severity::Warn)
                             | bit(sage::Severity::Error) | bit(sage::Severity::Fatal);
    EXPECT_EQ(all, 0b11'1111);
    EXPECT_EQ(all, 0x3F);
    EXPECT_EQ(all, 63);
    EXPECT_EQ(std::popcount(all), 6);
}

TEST(CoreSeverity, DocumentedMaskSelectsInfoAndAbove)
{
    EXPECT_EQ(info_and_above, 0x3C);
    EXPECT_EQ(info_and_above, 60);
    EXPECT_FALSE(is_enabled(sage::Severity::Trace));
    EXPECT_FALSE(is_enabled(sage::Severity::Debug));
    EXPECT_TRUE(is_enabled(sage::Severity::Info));
    EXPECT_TRUE(is_enabled(sage::Severity::Warn));
    EXPECT_TRUE(is_enabled(sage::Severity::Error));
    EXPECT_TRUE(is_enabled(sage::Severity::Fatal));
}

TEST(CoreSeverity, IsAScopedEnum)
{
    EXPECT_TRUE(std::is_scoped_enum_v<sage::Severity>);
    EXPECT_FALSE((std::is_convertible_v<sage::Severity, int>));
    EXPECT_FALSE((std::is_convertible_v<types::uint8, sage::Severity>));
}

TEST(CoreSeverity, ValuesRoundTripThroughTheirUnderlyingType)
{
    for (types::uint8 value : {1, 2, 4, 8, 16, 32})
        EXPECT_EQ(bit(static_cast<sage::Severity>(value)), value);
}

TEST(CoreSeverity, EnumeratorsAreSpelledInUpperCase_PerDocs)
{
    EXPECT_TRUE(has_Trace<sage::Severity>);
    EXPECT_TRUE(has_Debug<sage::Severity>);
    EXPECT_TRUE(has_Info<sage::Severity>);
    EXPECT_TRUE(has_Warn<sage::Severity>);
    EXPECT_TRUE(has_Error<sage::Severity>);
    EXPECT_TRUE(has_Fatal<sage::Severity>);
}

// ================================================================================================
// Numerics: limit_of
// ================================================================================================

TEST(CoreLimits, SignedIntegerBounds)
{
    EXPECT_EQ(limits::int8::max(), 127);
    EXPECT_EQ(limits::int8::min(), -128);
    EXPECT_EQ(limits::int16::max(), 32'767);
    EXPECT_EQ(limits::int16::min(), -32'768);
    EXPECT_EQ(limits::int32::max(), 2'147'483'647);
    EXPECT_EQ(limits::int32::min(), -2'147'483'647 - 1);
    EXPECT_EQ(limits::int64::max(), 9'223'372'036'854'775'807LL);
    EXPECT_EQ(limits::int64::min(), -9'223'372'036'854'775'807LL - 1);
}

TEST(CoreLimits, UnsignedIntegerBounds)
{
    EXPECT_EQ(limits::uint8::max(), 255U);
    EXPECT_EQ(limits::uint8::min(), 0U);
    EXPECT_EQ(limits::uint16::max(), 65'535U);
    EXPECT_EQ(limits::uint16::min(), 0U);
    EXPECT_EQ(limits::uint32::max(), 4'294'967'295U);
    EXPECT_EQ(limits::uint32::min(), 0U);
    EXPECT_EQ(limits::uint64::max(), 18'446'744'073'709'551'615ULL);
    EXPECT_EQ(limits::uint64::min(), 0U);
}

TEST(CoreLimits, IntegerLowestEqualsMin)
{
    EXPECT_EQ(limits::int8::lowest(), limits::int8::min());
    EXPECT_EQ(limits::int16::lowest(), limits::int16::min());
    EXPECT_EQ(limits::int32::lowest(), limits::int32::min());
    EXPECT_EQ(limits::int64::lowest(), limits::int64::min());
    EXPECT_EQ(limits::uint8::lowest(), 0U);
    EXPECT_EQ(limits::uint16::lowest(), 0U);
    EXPECT_EQ(limits::uint32::lowest(), 0U);
    EXPECT_EQ(limits::uint64::lowest(), 0U);
}

TEST(CoreLimits, IntegerEpsilonIsZero)
{
    EXPECT_EQ(limits::int8::epsilon(), 0);
    EXPECT_EQ(limits::int16::epsilon(), 0);
    EXPECT_EQ(limits::int32::epsilon(), 0);
    EXPECT_EQ(limits::int64::epsilon(), 0);
    EXPECT_EQ(limits::uint8::epsilon(), 0U);
    EXPECT_EQ(limits::uint16::epsilon(), 0U);
    EXPECT_EQ(limits::uint32::epsilon(), 0U);
    EXPECT_EQ(limits::uint64::epsilon(), 0U);
}

TEST(CoreLimits, IntegerLimitsWrapOnOverflow)
{
    EXPECT_EQ(static_cast<types::uint8>(limits::uint8::max() + 1), limits::uint8::min());
    EXPECT_EQ(static_cast<types::uint32>(limits::uint32::max() + 1U), limits::uint32::min());
    EXPECT_EQ(static_cast<types::uint64>(limits::uint64::max() + 1U), limits::uint64::min());
    EXPECT_EQ(static_cast<types::int8>(limits::int8::max() + 1), limits::int8::min());
}

TEST(CoreLimits, Float32Bounds)
{
    EXPECT_EQ(limits::float32::max(), 0x1.fffffep127F);
    EXPECT_EQ(limits::float32::min(), 0x1p-126F);  // smallest positive normal, as in <limits>
    EXPECT_EQ(limits::float32::lowest(), -limits::float32::max());
    EXPECT_EQ(limits::float32::epsilon(), 0x1p-23F);
    EXPECT_GT(limits::float32::min(), 0.0F);
    EXPECT_LT(limits::float32::lowest(), 0.0F);
}

TEST(CoreLimits, Float64Bounds)
{
    EXPECT_EQ(limits::float64::max(), 0x1.fffffffffffffp1023);
    EXPECT_EQ(limits::float64::min(), 0x1p-1022);
    EXPECT_EQ(limits::float64::lowest(), -limits::float64::max());
    EXPECT_EQ(limits::float64::epsilon(), 0x1p-52);
    EXPECT_GT(limits::float64::min(), 0.0);
    EXPECT_LT(limits::float64::lowest(), 0.0);
}

TEST(CoreLimits, Float64LeastBounds)
{
    EXPECT_EQ(limits::float64_least::max(), std::numeric_limits<long double>::max());
    EXPECT_EQ(limits::float64_least::min(), std::numeric_limits<long double>::min());
    EXPECT_EQ(limits::float64_least::lowest(), -limits::float64_least::max());
    EXPECT_EQ(limits::float64_least::epsilon(), std::numeric_limits<long double>::epsilon());
    EXPECT_GT(limits::float64_least::epsilon(), 0.0L);
    EXPECT_GE(limits::float64_least::max(), static_cast<long double>(limits::float64::max()));
}

TEST(CoreLimits, FloatingPointLimitsAreFiniteAndOrdered)
{
    EXPECT_FALSE(std::isinf(limits::float32::max()));
    EXPECT_FALSE(std::isinf(limits::float64::max()));
    EXPECT_LT(limits::float32::epsilon(), 1.0F);
    EXPECT_LT(limits::float64::epsilon(), limits::float32::epsilon());
    EXPECT_GT(1.0F + limits::float32::epsilon(), 1.0F);
    EXPECT_EQ(1.0F + limits::float32::epsilon() / 2, 1.0F);
    EXPECT_GT(1.0 + limits::float64::epsilon(), 1.0);
    EXPECT_EQ(1.0 + limits::float64::epsilon() / 2, 1.0);
}

TEST(CoreLimits, AliasesAreLimitOfTheMatchingType)
{
    EXPECT_TRUE((std::is_same_v<limits::int8, limits::limit_of<types::int8>>));
    EXPECT_TRUE((std::is_same_v<limits::int16, limits::limit_of<types::int16>>));
    EXPECT_TRUE((std::is_same_v<limits::int32, limits::limit_of<types::int32>>));
    EXPECT_TRUE((std::is_same_v<limits::int64, limits::limit_of<types::int64>>));
    EXPECT_TRUE((std::is_same_v<limits::uint8, limits::limit_of<types::uint8>>));
    EXPECT_TRUE((std::is_same_v<limits::uint16, limits::limit_of<types::uint16>>));
    EXPECT_TRUE((std::is_same_v<limits::uint32, limits::limit_of<types::uint32>>));
    EXPECT_TRUE((std::is_same_v<limits::uint64, limits::limit_of<types::uint64>>));
    EXPECT_TRUE((std::is_same_v<limits::float32, limits::limit_of<types::float32>>));
    EXPECT_TRUE((std::is_same_v<limits::float64, limits::limit_of<types::float64>>));
    EXPECT_TRUE((std::is_same_v<limits::float64_least, limits::limit_of<types::float64_least>>));
}

TEST(CoreLimits, ReturnTypesMatchTheLimitedType)
{
    EXPECT_TRUE((std::is_same_v<decltype(limits::int8::max()), types::int8>));
    EXPECT_TRUE((std::is_same_v<decltype(limits::uint16::min()), types::uint16>));
    EXPECT_TRUE((std::is_same_v<decltype(limits::int64::lowest()), types::int64>));
    EXPECT_TRUE((std::is_same_v<decltype(limits::float32::epsilon()), types::float32>));
    EXPECT_TRUE((std::is_same_v<decltype(limits::float64::max()), types::float64>));
}

TEST(CoreLimits, LimitsMatchStdNumericLimitsForEveryType)
{
    EXPECT_EQ(limits::int8::max(), std::numeric_limits<signed char>::max());
    EXPECT_EQ(limits::int16::min(), std::numeric_limits<short>::min());
    EXPECT_EQ(limits::int32::max(), std::numeric_limits<int>::max());
    EXPECT_EQ(limits::int64::min(), std::numeric_limits<long long>::min());
    EXPECT_EQ(limits::uint8::max(), std::numeric_limits<unsigned char>::max());
    EXPECT_EQ(limits::uint16::max(), std::numeric_limits<unsigned short>::max());
    EXPECT_EQ(limits::uint32::max(), std::numeric_limits<unsigned>::max());
    EXPECT_EQ(limits::uint64::max(), std::numeric_limits<unsigned long long>::max());
    EXPECT_EQ(limits::float32::epsilon(), std::numeric_limits<float>::epsilon());
    EXPECT_EQ(limits::float64::min(), std::numeric_limits<double>::min());
}

TEST(CoreLimits, LimitsAreConstexprAndNoexcept)
{
    constexpr auto max32 = limits::int32::max();
    constexpr auto eps64 = limits::float64::epsilon();
    EXPECT_EQ(max32, std::numeric_limits<types::int32>::max());
    EXPECT_GT(eps64, 0.0);

    static_assert(noexcept(limits::int32::max()));
    static_assert(noexcept(limits::int32::min()));
    static_assert(noexcept(limits::int32::lowest()));
    static_assert(noexcept(limits::int32::epsilon()));
    static_assert(limits::uint8::max() == 255);
}

TEST(CoreLimits, WorksForOtherNumericTypesToo)
{
    EXPECT_TRUE(limits::limit_of<bool>::max());
    EXPECT_FALSE(limits::limit_of<bool>::min());
    EXPECT_FALSE(limits::limit_of<bool>::lowest());
    EXPECT_EQ(limits::limit_of<char8_t>::max(), 255U);
    EXPECT_EQ(limits::limit_of<char16_t>::max(), 65'535U);
    EXPECT_EQ(limits::limit_of<char32_t>::max(), 4'294'967'295U);
    EXPECT_EQ(limits::limit_of<types::uchar8>::max(), 255U);
    EXPECT_EQ(limits::limit_of<types::usize>::max(), std::numeric_limits<std::size_t>::max());
}

TEST(CoreLimits, RejectsNonNumericTypes)
{
    EXPECT_TRUE(can_limit_of<int>);
    EXPECT_TRUE(can_limit_of<double>);
    EXPECT_TRUE(can_limit_of<bool>);
    EXPECT_TRUE(can_limit_of<long double>);
    EXPECT_FALSE(can_limit_of<std::string>);
    EXPECT_FALSE(can_limit_of<int*>);
    EXPECT_FALSE(can_limit_of<void*>);
    EXPECT_FALSE(can_limit_of<ScopedEnum>);
    EXPECT_FALSE(can_limit_of<PlainStruct>);
    EXPECT_FALSE(can_limit_of<void>);
}

TEST(CoreLimits, ProvidesSmallest_PerDocs)
{
    EXPECT_TRUE(has_lowest<limits::int32>);
    EXPECT_TRUE(has_lowest<limits::float64>);
}

// ================================================================================================
// Numerics: memory size helpers
// ================================================================================================

TEST(CoreMemorySizes, OneUnitInBytes)
{
    EXPECT_EQ(numerics::KiB(1U), 1'024U);
    EXPECT_EQ(numerics::MiB(1U), 1'048'576U);
    EXPECT_EQ(numerics::GiB(1U), 1'073'741'824U);
    EXPECT_EQ(numerics::KB(1U), 1'000U);
    EXPECT_EQ(numerics::MB(1U), 1'000'000U);
    EXPECT_EQ(numerics::GB(1U), 1'000'000'000U);
}

TEST(CoreMemorySizes, ZeroUnitsAreZeroBytes)
{
    EXPECT_EQ(numerics::KiB(0U), 0U);
    EXPECT_EQ(numerics::MiB(0U), 0U);
    EXPECT_EQ(numerics::GiB(0U), 0U);
    EXPECT_EQ(numerics::KB(0U), 0U);
    EXPECT_EQ(numerics::MB(0U), 0U);
    EXPECT_EQ(numerics::GB(0U), 0U);
    EXPECT_DOUBLE_EQ(numerics::KiB(0.0), 0.0);
    EXPECT_DOUBLE_EQ(numerics::GB(0.0F), 0.0);
}

TEST(CoreMemorySizes, UnitsScaleLinearly)
{
    for (types::uint32 n : {1U, 2U, 3U, 7U, 100U, 1'023U, 1'024U, 1'025U})
    {
        EXPECT_EQ(numerics::KiB(n), static_cast<types::usize>(n) * 1'024U) << n;
        EXPECT_EQ(numerics::MiB(n), static_cast<types::usize>(n) * 1'048'576U) << n;
        EXPECT_EQ(numerics::KB(n), static_cast<types::usize>(n) * 1'000U) << n;
        EXPECT_EQ(numerics::MB(n), static_cast<types::usize>(n) * 1'000'000U) << n;
    }
}

TEST(CoreMemorySizes, UnitsAreRelatedByTheirBase)
{
    EXPECT_EQ(numerics::KiB(1U) * 1'024U, numerics::MiB(1U));
    EXPECT_EQ(numerics::MiB(1U) * 1'024U, numerics::GiB(1U));
    EXPECT_EQ(numerics::KB(1U) * 1'000U, numerics::MB(1U));
    EXPECT_EQ(numerics::MB(1U) * 1'000U, numerics::GB(1U));
    EXPECT_GT(numerics::KiB(1U), numerics::KB(1U));
    EXPECT_GT(numerics::MiB(1U), numerics::MB(1U));
    EXPECT_GT(numerics::GiB(1U), numerics::GB(1U));
    EXPECT_EQ(numerics::KiB(1'024U), numerics::MiB(1U));
    EXPECT_EQ(numerics::MiB(1'024U), numerics::GiB(1U));
    EXPECT_EQ(numerics::KB(1'000U), numerics::MB(1U));
    EXPECT_EQ(numerics::MB(1'000U), numerics::GB(1U));
}

TEST(CoreMemorySizes, IntegerArgumentsReturnUsize)
{
    EXPECT_TRUE((std::is_same_v<decltype(numerics::KiB(types::uint8{1})), types::usize>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::MiB(types::uint16{1})), types::usize>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::GiB(types::uint32{1})), types::usize>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::KB(types::uint64{1})), types::usize>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::MB(types::usize{1})), types::usize>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::GB(1UL)), types::usize>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::GB(1ULL)), types::usize>));
}

TEST(CoreMemorySizes, FloatingPointArgumentsReturnFloat64)
{
    EXPECT_TRUE((std::is_same_v<decltype(numerics::KiB(1.0F)), types::float64>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::MiB(1.0)), types::float64>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::GiB(1.0L)), types::float64>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::KB(1.0F)), types::float64>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::MB(1.0)), types::float64>));
    EXPECT_TRUE((std::is_same_v<decltype(numerics::GB(1.0L)), types::float64>));
}

TEST(CoreMemorySizes, FloatingPointArgumentsScaleToWholeBytes)
{
    EXPECT_DOUBLE_EQ(numerics::KiB(0.5), 512.0);
    EXPECT_DOUBLE_EQ(numerics::KiB(1.5), 1'536.0);
    EXPECT_DOUBLE_EQ(numerics::MiB(0.5F), 524'288.0);
    EXPECT_DOUBLE_EQ(numerics::GiB(0.25), 268'435'456.0);
    EXPECT_DOUBLE_EQ(numerics::KB(2.5), 2'500.0);
    EXPECT_DOUBLE_EQ(numerics::MB(0.5L), 500'000.0);
    EXPECT_DOUBLE_EQ(numerics::GB(1.5), 1'500'000'000.0);
}

TEST(CoreMemorySizes, FloatingPointResultsBeyondThirtyTwoBitAreExact)
{
    EXPECT_DOUBLE_EQ(numerics::GiB(4.0), 4'294'967'296.0);
    EXPECT_DOUBLE_EQ(numerics::GB(5.0), 5'000'000'000.0);
    EXPECT_DOUBLE_EQ(numerics::MiB(5'000.0), 5'242'880'000.0);
}

TEST(CoreMemorySizes, AreEvaluatedAtCompileTime)
{
    constexpr types::usize stack_size  = numerics::KiB(64U);
    constexpr types::usize arena_size  = numerics::MiB(16U);
    constexpr types::usize packet_size = numerics::KB(1U);
    constexpr types::usize big         = numerics::GiB(types::uint64{8});

    static_assert(stack_size == 65'536);
    static_assert(arena_size == 16'777'216);
    static_assert(packet_size == 1'000);
    static_assert(big == 8'589'934'592ULL);

    char buffer[numerics::KiB(1U)];  // usable as an array bound
    EXPECT_EQ(sizeof(buffer), 1'024U);
}

TEST(CoreMemorySizes, SmallIntegerTypesDoNotOverflowWhenTheResultFitsIn32Bit)
{
    expect_bytes("KiB", kib, types::uint8{255}, 1'024);
    expect_bytes("KiB", kib, types::uint16{65'535}, 1'024);
    expect_bytes("KB", kb, types::uint8{255}, 1'000);
    expect_bytes("KB", kb, types::uint16{65'535}, 1'000);
    expect_bytes("MiB", mib, types::uint8{255}, 1'048'576);
    expect_bytes("MB", mb, types::uint8{255}, 1'000'000);
    expect_bytes("GiB", gib, types::uint8{3}, 1'073'741'824);
    expect_bytes("GB", gb, types::uint8{4}, 1'000'000'000);
}

TEST(CoreMemorySizes, SixtyFourBitArgumentsDoNotOverflowAboveFourGiB)
{
    expect_bytes("KiB", kib, types::uint64{4'194'304}, 1'024);
    expect_bytes("MiB", mib, types::uint64{4'096}, 1'048'576);
    expect_bytes("GiB", gib, types::uint64{4}, 1'073'741'824);
    expect_bytes("GiB", gib, types::uint64{16'384}, 1'073'741'824);
    expect_bytes("KB", kb, types::uint64{5'000'000}, 1'000);
    expect_bytes("MB", mb, types::uint64{5'000}, 1'000'000);
    expect_bytes("GB", gb, types::uint64{5}, 1'000'000'000);
    expect_bytes("GB", gb, types::usize{16}, 1'000'000'000);
}

TEST(CoreMemorySizes, ThirtyTwoBitArgumentsAtTheLastNonOverflowingValueAreExact)
{
    expect_bytes("KiB", kib, types::uint32{4'194'303}, 1'024);
    expect_bytes("MiB", mib, types::uint32{4'095}, 1'048'576);
    expect_bytes("GiB", gib, types::uint32{3}, 1'073'741'824);
    expect_bytes("KB", kb, types::uint32{4'294'967}, 1'000);
    expect_bytes("MB", mb, types::uint32{4'294}, 1'000'000);
    expect_bytes("GB", gb, types::uint32{4}, 1'000'000'000);
}

TEST(CoreMemorySizes, ThirtyTwoBitArgumentsAboveFourGiBDoNotWrap)
{
    expect_bytes("KiB", kib, types::uint32{4'194'304}, 1'024);
    expect_bytes("MiB", mib, types::uint32{4'096}, 1'048'576);
    expect_bytes("GiB", gib, types::uint32{4}, 1'073'741'824);
    expect_bytes("GiB", gib, types::uint32{5}, 1'073'741'824);
    expect_bytes("GiB", gib, types::uint32{1'000}, 1'073'741'824);
    expect_bytes("KB", kb, types::uint32{4'294'968}, 1'000);
    expect_bytes("MB", mb, types::uint32{4'295}, 1'000'000);
    expect_bytes("GB", gb, types::uint32{5}, 1'000'000'000);
    expect_bytes("GB", gb, types::uint32{4'294'967'295U}, 1'000'000'000);
}

TEST(CoreMemorySizes, NarrowArgumentsWhoseResultExceedsFourGiBDoNotWrap)
{
    expect_bytes("MiB", mib, types::uint16{65'535}, 1'048'576);
    expect_bytes("MB", mb, types::uint16{65'535}, 1'000'000);
    expect_bytes("GiB", gib, types::uint8{255}, 1'073'741'824);
    expect_bytes("GB", gb, types::uint8{255}, 1'000'000'000);
    expect_bytes("GiB", gib, types::uint16{65'535}, 1'073'741'824);
}

TEST(CoreMemorySizes, FloatingPointResultsAreNotOffByOneFromTruncation)
{
    EXPECT_DOUBLE_EQ(numerics::KB(1.001), 1'001.0);
    EXPECT_DOUBLE_EQ(numerics::MB(1.001), 1'001'000.0);
    EXPECT_DOUBLE_EQ(numerics::GB(1.001), 1'001'000'000.0);
}

TEST(CoreMemorySizes, FloatingPointResultsKeepTheirFractionalPart)
{
    EXPECT_DOUBLE_EQ(numerics::KiB(0.001), 1.024);
    EXPECT_DOUBLE_EQ(numerics::KB(0.0015), 1.5);
    EXPECT_DOUBLE_EQ(numerics::MiB(0.0000005), 0.524288);
}

TEST(CoreMemorySizes, RejectNonNumericArguments)
{
    EXPECT_FALSE(accepts_KiB<std::string>);
    EXPECT_FALSE(accepts_MiB<const char*>);
    EXPECT_FALSE(accepts_GiB<int*>);
    EXPECT_FALSE(accepts_KB<ScopedEnum>);
    EXPECT_FALSE(accepts_MB<PlainStruct>);
    EXPECT_FALSE(accepts_GB<std::nullptr_t>);
}

TEST(CoreMemorySizes, RejectSignedIntegerArguments)
{
    EXPECT_FALSE(accepts_KiB<types::int8>);
    EXPECT_FALSE(accepts_MiB<types::int32>);
    EXPECT_FALSE(accepts_GiB<types::int64>);
    EXPECT_FALSE(accepts_KB<short>);
    EXPECT_FALSE(accepts_MB<long>);
    EXPECT_FALSE(accepts_GB<types::isize>);
}

TEST(CoreMemorySizes, AcceptUnsignedIntegerAndFloatingPointArguments)
{
    EXPECT_TRUE(accepts_KiB<types::uint8>);
    EXPECT_TRUE(accepts_MiB<types::uint16>);
    EXPECT_TRUE(accepts_GiB<types::uint32>);
    EXPECT_TRUE(accepts_KB<types::uint64>);
    EXPECT_TRUE(accepts_MB<types::usize>);
    EXPECT_TRUE(accepts_GB<unsigned long long>);
    EXPECT_TRUE(accepts_KiB<float>);
    EXPECT_TRUE(accepts_MiB<double>);
    EXPECT_TRUE(accepts_GiB<long double>);
}

// ================================================================================================
// Template types
// ================================================================================================

TEST(CoreTemplateTypes, NumericTypeAcceptsIntegers)
{
    EXPECT_SATISFIES(numeric_type, int);
    EXPECT_SATISFIES(numeric_type, unsigned);
    EXPECT_SATISFIES(numeric_type, short);
    EXPECT_SATISFIES(numeric_type, long);
    EXPECT_SATISFIES(numeric_type, long long);
    EXPECT_SATISFIES(numeric_type, signed char);
    EXPECT_SATISFIES(numeric_type, unsigned char);
    EXPECT_SATISFIES(numeric_type, types::int8);
    EXPECT_SATISFIES(numeric_type, types::int64);
    EXPECT_SATISFIES(numeric_type, types::uint64);
    EXPECT_SATISFIES(numeric_type, types::usize);
    EXPECT_SATISFIES(numeric_type, types::isize);
    EXPECT_SATISFIES(numeric_type, types::intptr);
    EXPECT_SATISFIES(numeric_type, types::uintptr);
    EXPECT_SATISFIES(numeric_type, types::byte);
}

TEST(CoreTemplateTypes, NumericTypeAcceptsCharactersAndBool)
{
    EXPECT_SATISFIES(numeric_type, char);
    EXPECT_SATISFIES(numeric_type, wchar_t);
    EXPECT_SATISFIES(numeric_type, char8_t);
    EXPECT_SATISFIES(numeric_type, char16_t);
    EXPECT_SATISFIES(numeric_type, char32_t);
    EXPECT_SATISFIES(numeric_type, bool);
    EXPECT_SATISFIES(numeric_type, types::bool8);
}

TEST(CoreTemplateTypes, NumericTypeAcceptsFloatingPoint)
{
    EXPECT_SATISFIES(numeric_type, float);
    EXPECT_SATISFIES(numeric_type, double);
    EXPECT_SATISFIES(numeric_type, long double);
    EXPECT_SATISFIES(numeric_type, types::float32);
    EXPECT_SATISFIES(numeric_type, types::float64);
    EXPECT_SATISFIES(numeric_type, types::float64_least);
}

TEST(CoreTemplateTypes, NumericTypeAcceptsCvQualifiedTypes)
{
    EXPECT_SATISFIES(numeric_type, const int);
    EXPECT_SATISFIES(numeric_type, volatile int);
    EXPECT_SATISFIES(numeric_type, const volatile unsigned);
    EXPECT_SATISFIES(numeric_type, const double);
}

TEST(CoreTemplateTypes, NumericTypeRejectsScopedEnums)
{
    EXPECT_NOT_SATISFIES(numeric_type, ScopedEnum);
    EXPECT_NOT_SATISFIES(numeric_type, sage::Severity);
    EXPECT_NOT_SATISFIES(numeric_type, std::byte);
}

TEST(CoreTemplateTypes, NumericTypeRejectsPointers)
{
    EXPECT_NOT_SATISFIES(numeric_type, int*);
    EXPECT_NOT_SATISFIES(numeric_type, const char*);
    EXPECT_NOT_SATISFIES(numeric_type, void*);
    EXPECT_NOT_SATISFIES(numeric_type, std::nullptr_t);
    EXPECT_NOT_SATISFIES(numeric_type, int (*)(int));
}

TEST(CoreTemplateTypes, NumericTypeRejectsOtherTypes)
{
    EXPECT_NOT_SATISFIES(numeric_type, std::string);
    EXPECT_NOT_SATISFIES(numeric_type, PlainStruct);
    EXPECT_NOT_SATISFIES(numeric_type, UnscopedEnum);
    EXPECT_NOT_SATISFIES(numeric_type, void);
    EXPECT_NOT_SATISFIES(numeric_type, int[3]);
    EXPECT_NOT_SATISFIES(numeric_type, char[3]);
}

TEST(CoreTemplateTypes, UnsignedNumericTypeAcceptsUnsignedIntegers)
{
    EXPECT_SATISFIES(unsigned_numeric_type, unsigned);
    EXPECT_SATISFIES(unsigned_numeric_type, unsigned char);
    EXPECT_SATISFIES(unsigned_numeric_type, unsigned short);
    EXPECT_SATISFIES(unsigned_numeric_type, unsigned long);
    EXPECT_SATISFIES(unsigned_numeric_type, unsigned long long);
    EXPECT_SATISFIES(unsigned_numeric_type, types::uint8);
    EXPECT_SATISFIES(unsigned_numeric_type, types::uint16);
    EXPECT_SATISFIES(unsigned_numeric_type, types::uint32);
    EXPECT_SATISFIES(unsigned_numeric_type, types::uint64);
    EXPECT_SATISFIES(unsigned_numeric_type, types::usize);
    EXPECT_SATISFIES(unsigned_numeric_type, types::uintptr);
    EXPECT_SATISFIES(unsigned_numeric_type, types::byte);
    EXPECT_SATISFIES(unsigned_numeric_type, char8_t);
    EXPECT_SATISFIES(unsigned_numeric_type, char16_t);
    EXPECT_SATISFIES(unsigned_numeric_type, char32_t);
    EXPECT_SATISFIES(unsigned_numeric_type, const unsigned);
}

TEST(CoreTemplateTypes, UnsignedNumericTypeAcceptsFloatingPoint)
{
    EXPECT_SATISFIES(unsigned_numeric_type, float);
    EXPECT_SATISFIES(unsigned_numeric_type, double);
    EXPECT_SATISFIES(unsigned_numeric_type, long double);
    EXPECT_SATISFIES(unsigned_numeric_type, types::float32);
    EXPECT_SATISFIES(unsigned_numeric_type, types::float64);
    EXPECT_SATISFIES(unsigned_numeric_type, types::float64_least);
}

TEST(CoreTemplateTypes, UnsignedNumericTypeRejectsSignedIntegers)
{
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, int);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, short);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, long);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, long long);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, signed char);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, types::int8);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, types::int16);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, types::int32);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, types::int64);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, types::isize);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, types::intptr);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, const int);
}

TEST(CoreTemplateTypes, UnsignedNumericTypeRejectsNonNumericTypes)
{
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, ScopedEnum);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, UnscopedEnum);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, sage::Severity);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, unsigned*);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, const char*);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, std::nullptr_t);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, std::string);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, PlainStruct);
    EXPECT_NOT_SATISFIES(unsigned_numeric_type, void);
}

TEST(CoreTemplateTypes, EveryUnsignedNumericTypeIsAlsoANumericType)
{
    constexpr auto implies = []<typename T>()
    {
        return !template_types::unsigned_numeric_type<T> || template_types::numeric_type<T>;
    };
    EXPECT_TRUE(implies.operator()<unsigned>());
    EXPECT_TRUE(implies.operator()<types::uint8>());
    EXPECT_TRUE(implies.operator()<types::uint64>());
    EXPECT_TRUE(implies.operator()<char32_t>());
    EXPECT_TRUE(implies.operator()<float>());
    EXPECT_TRUE(implies.operator()<long double>());
    EXPECT_TRUE(implies.operator()<int>());
    EXPECT_TRUE(implies.operator()<std::string>());
}

TEST(CoreTemplateTypes, UnsignedNumericTypeIsTheMoreConstrainedConcept)
{
    EXPECT_EQ(which(5), 0);    // only numeric_type
    EXPECT_EQ(which(-5L), 0);  // only numeric_type
    EXPECT_EQ(which(5U), 1);   // both, the unsigned overload is more constrained
    EXPECT_EQ(which(types::byte{1}), 1);
    EXPECT_EQ(which(1.5), 1);  // floating point satisfies both
}
