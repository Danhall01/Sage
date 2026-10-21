module;

export module sage:core.severity;
export import :core.types;

export namespace sage
{
enum class Severity : sage::types::byte
{
    Trace = 1 << 0,
    Debug = 1 << 1,
    Info  = 1 << 2,
    Warn  = 1 << 3,
    Error = 1 << 4,
    Fatal = 1 << 5,
};
}  // namespace sage
