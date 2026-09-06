module;

#include <cstdint>
export module sage:severity;

export namespace sage
{
enum class Severity : std::uint8_t
{
    TRACE = 1 << 0,
    DEBUG = 1 << 1,
    INFO  = 1 << 2,
    WARN  = 1 << 3,
    ERROR = 1 << 4,
    FATAL = 1 << 5,
};
}
