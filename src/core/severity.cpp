module;

export module sage:core_severity;
export import :core_types;

export namespace sage
{
enum class Severity : sage::types::uint8
{
    Trace = 1 << 0,
    Debug = 1 << 1,
    Info  = 1 << 2,
    Warn  = 1 << 3,
    Error = 1 << 4,
    Fatal = 1 << 5,
};
}  // namespace sage
