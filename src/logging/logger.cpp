module;

#include <concepts>
#include <cstdio>
#include <format>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
export module sage:logger;
export import :severity;
export import :types;

namespace
{
inline void default_write_std(sage::Severity level, std::string_view msg)
{
    auto* write_file = level >= sage::Severity::WARN ? stderr : stdout;
    auto  write_size = std::fwrite(msg.data(), 1, msg.size(), write_file);
}
inline void default_write_file(std::string_view msg)
{
    static FILE* crashfile  = fopen("last_run.log", "w");
    auto         write_size = std::fwrite(msg.data(), 1, msg.size(), crashfile);
}
}  // namespace


// Utility functions
namespace sage::_backend::log
{
export consteval std::string_view format_prefix(sage::Severity severity)
{
    switch (severity)
    {
        case sage::Severity::TRACE:
            return "TRC";
        case sage::Severity::DEBUG:
            return "DBG";
        case sage::Severity::INFO:
            return "INF";
        case sage::Severity::WARN:
            return "WNR";
        case sage::Severity::ERROR:
            return "ERR";
        case sage::Severity::FATAL:
            return "FTL";

        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
export consteval std::string_view format_color(sage::Severity severity)
{
    switch (severity)
    {
        case sage::Severity::TRACE:
            return "\e[0;37m";  // Gray
        case sage::Severity::DEBUG:
            return "\e[0;35m";  // Purple
        case sage::Severity::INFO:
            return "\e[0;32m";  // Green
        case sage::Severity::WARN:
            return "\e[0;33m";  // Yellow
        case sage::Severity::ERROR:
            return "\e[0;31m";  // Red
        case sage::Severity::FATAL:
            return "\e[1;91m";  // DarkRed

        default:
            return "";
    }
    return "";
}

export template<typename... Args>
struct logger_str
{
    std::format_string<Args...> format;
    std::source_location        location;

    template<sage::types::usize N>
    consteval logger_str(
        const char           (&fmt)[N],
        std::source_location whence = std::source_location::current()
    )
        : format(fmt),
          location(whence)
    {
    }
};
export template<typename... Args>
using logger_str_t = logger_str<std::type_identity_t<Args>...>;

export template<typename... Args>
struct logger_runtime_str
{
    std::string_view     format;
    std::source_location location;

    template<typename T>
        requires(!std::is_array_v<std::remove_reference_t<T>>
                 && std::convertible_to<const T&, std::string_view>)
    constexpr logger_runtime_str(
        const T&             fmt,
        std::source_location whence = std::source_location::current()
    )
        : format(fmt),
          location(whence)
    {
    }
};
export template<typename... Args>
using logger_runtime_str_t = logger_runtime_str<std::type_identity_t<Args>...>;


// Logger
export struct Config
{
#ifdef SAGE_LOG_ENABLED_SEVERITY
    sage::types::u8 enabled_severity = SAGE_LOG_ENABLED_SEVERITY;
#else
    sage::types::u8 enabled_severity = static_cast<sage::types::u8>(sage::Severity::TRACE)
                                       | static_cast<sage::types::u8>(sage::Severity::DEBUG)
                                       | static_cast<sage::types::u8>(sage::Severity::INFO)
                                       | static_cast<sage::types::u8>(sage::Severity::WARN)
                                       | static_cast<sage::types::u8>(sage::Severity::ERROR)
                                       | static_cast<sage::types::u8>(sage::Severity::FATAL);
#endif
#ifdef SAGE_LOG_NO_FORMATTING
    bool formatting = false;
#else
    bool formatting = true;
#endif
#ifdef SAGE_LOG_NO_COLORS
    bool colors = false;
#else
    bool colors = true;
#endif
#ifdef SAGE_LOG_WRITE_TO_FILE
    bool write_to_file = true;
#else
    bool write_to_file = false;
#endif
    void (*std_writer)(sage::Severity, std::string_view) = &::default_write_std;
    void (*file_writer)(std::string_view)                = &::default_write_file;
};

export template<Config cfg = Config{}>
class logger
{
  private:
    template<sage::Severity level>
    static void log_impl(std::source_location whence, std::string msg)
    {
        if constexpr ((static_cast<sage::types::u8>(level) & cfg.enabled_severity) == 0)
            return;

        if constexpr (cfg.formatting)
        {
            const std::string_view fpath     = whence.file_name();
            const std::string_view func_name = whence.function_name();
            msg                              = std::format(
                "==[{}:{}:{}]==\n[{}]\t{}",
                fpath.substr(fpath.find_last_of("/\\") + 1),
                func_name.substr(func_name.find_last_of(' ') + 1),
                whence.line(),
                format_prefix(level),
                msg
            );
        }
        if constexpr (cfg.write_to_file)
            cfg.file_writer(msg);

        if constexpr (cfg.colors)
            msg = std::format("{}{}{}", format_color(level), msg, "\e[0m");

        cfg.std_writer(level, msg);
    }

  public:
    template<typename... Args>
    static void trace(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::TRACE>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void trace(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::TRACE>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void debug(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::DEBUG>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void debug(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::DEBUG>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void info(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::INFO>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void info(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::INFO>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void warn(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::WARN>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void warn(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::WARN>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void error(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::ERROR>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
#ifdef SAGE_LOG_THROW_ON_ERROR
        throw std::runtime_error(fmt.format.get().data());
#endif
    }
    template<typename... Args>
    static void error(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::ERROR>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
#ifdef SAGE_LOG_THROW_ON_ERROR
        throw std::runtime_error(fmt.format.get().data());
#endif
    }


    template<typename... Args>
    static void fatal(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::FATAL>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
        abort();
    }
    template<typename... Args>
    static void fatal(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::FATAL>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
        abort();
    }
};
}  // namespace sage::_backend::log


#ifndef SAGE_LOG_NO_DEFAULT_LOGGER
namespace sage
{
export using log = sage::_backend::log::logger<>;
}
#endif
