module;

export module sage:core_severity;
export import :core_types;

export namespace sage
{
enum class Severity : sage::types::uint8
{
    TRACE = 1 << 0,
    DEBUG = 1 << 1,
    INFO  = 1 << 2,
    WARN  = 1 << 3,
    ERROR = 1 << 4,
    FATAL = 1 << 5,
};
}  // namespace sage
