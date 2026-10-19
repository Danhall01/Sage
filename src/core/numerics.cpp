module;
#include <limits>
#include <type_traits>
export module sage:core_numerics;
export import :core_types;
export import :core_template_types;

export namespace sage::limits
{
template<sage::template_types::numeric_type T>
struct limit_of
{
  public:
    [[nodiscard]]
    static constexpr T max() noexcept
    { return std::numeric_limits<T>::max(); }
    [[nodiscard]]
    static constexpr T min() noexcept
    { return std::numeric_limits<T>::min(); }
    [[nodiscard]]
    static constexpr T lowest() noexcept
    { return std::numeric_limits<T>::lowest(); }
    [[nodiscard]]
    static constexpr T epsilon() noexcept
    { return std::numeric_limits<T>::epsilon(); }
};

using int8  = limit_of<sage::types::int8>;
using int16 = limit_of<sage::types::int16>;
using int32 = limit_of<sage::types::int32>;
using int64 = limit_of<sage::types::int64>;

using uint8  = limit_of<sage::types::uint8>;
using uint16 = limit_of<sage::types::uint16>;
using uint32 = limit_of<sage::types::uint32>;
using uint64 = limit_of<sage::types::uint64>;

using float32       = limit_of<sage::types::float32>;
using float64       = limit_of<sage::types::float64>;
using float64_least = limit_of<sage::types::float64_least>;
}  // namespace sage::limits

namespace sage::numerics
{
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
