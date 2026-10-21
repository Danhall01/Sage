module;
#include <concepts>

export module sage:core.template_types;

namespace sage::template_types
{
// Ensure the value is numeric type (float, int, etc)
export template<typename T>
concept numeric_type = std::integral<T> || std::floating_point<T>;

export template<typename T>
concept unsigned_numeric_type = std::unsigned_integral<T> || std::floating_point<T>;

}  // namespace sage::template_types
