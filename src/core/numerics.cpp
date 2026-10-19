module;
#include <limits>
#include <type_traits>
export module sage:core_numerics;
export import :core_types;
export import :core_template_types;

namespace sage::numerics
{
// clang-format off
export inline constexpr sage::types::int8  INT8_MAX = std::numeric_limits<sage::types::int8>::max();
export inline constexpr sage::types::int16 INT16_MAX = std::numeric_limits<sage::types::int16>::max();
export inline constexpr sage::types::int32 INT32_MAX = std::numeric_limits<sage::types::int32>::max();
export inline constexpr sage::types::int64 INT64_MAX = std::numeric_limits<sage::types::int64>::max();

export inline constexpr sage::types::uint8  UINT8_MAX = std::numeric_limits<sage::types::uint8>::max();
export inline constexpr sage::types::uint64 UINT16_MAX = std::numeric_limits<sage::types::uint16>::max();
export inline constexpr sage::types::uint64 UINT32_MAX = std::numeric_limits<sage::types::uint32>::max();
export inline constexpr sage::types::uint64 UINT64_MAX = std::numeric_limits<sage::types::uint64>::max();

export inline constexpr sage::types::float32       FLOAT32_MAX = std::numeric_limits<sage::types::float32>::max();
export inline constexpr sage::types::float64       FLOAT64_MAX = std::numeric_limits<sage::types::float64>::max();
export inline constexpr sage::types::float64_least FLOAT64_LEAST_MAX = std::numeric_limits<sage::types::float64_least>::max();
// clang-format on


constexpr auto binary_kib = 1'024U;
constexpr auto decimal_kb = 1'000U;

export template<sage::template_types::unsigned_numeric_type T>
constexpr auto KiB(T value)
    -> std::conditional_t<std::is_floating_point_v<T>, sage::types::float64, sage::types::usize>
{ return static_cast<sage::types::usize>(value * binary_kib); }

export template<sage::template_types::unsigned_numeric_type T>
constexpr auto MiB(T value)
    -> std::conditional_t<std::is_floating_point_v<T>, sage::types::float64, sage::types::usize>
{ return static_cast<sage::types::usize>(value * binary_kib * binary_kib); }

export template<sage::template_types::unsigned_numeric_type T>
constexpr auto GiB(T value)
    -> std::conditional_t<std::is_floating_point_v<T>, sage::types::float64, sage::types::usize>
{ return static_cast<sage::types::usize>(value * binary_kib * binary_kib * binary_kib); }


export template<sage::template_types::unsigned_numeric_type T>
constexpr auto KB(T value)
    -> std::conditional_t<std::is_floating_point_v<T>, sage::types::float64, sage::types::usize>
{ return static_cast<sage::types::usize>(value * decimal_kb); }

export template<sage::template_types::unsigned_numeric_type T>
constexpr auto MB(T value)
    -> std::conditional_t<std::is_floating_point_v<T>, sage::types::float64, sage::types::usize>
{ return static_cast<sage::types::usize>(value * decimal_kb * decimal_kb); }

export template<sage::template_types::unsigned_numeric_type T>
constexpr auto GB(T value)
    -> std::conditional_t<std::is_floating_point_v<T>, sage::types::float64, sage::types::usize>
{ return static_cast<sage::types::usize>(value * decimal_kb * decimal_kb * decimal_kb); }
}  // namespace sage::numerics
