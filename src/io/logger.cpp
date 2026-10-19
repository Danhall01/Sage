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
export module sage:io_logger;
export import :core_severity;
export import :core_types;

#ifdef SAGE_LOG_ENABLED_SEVERITY
    #define SAGE_INTERNAL_LOG_ENABLED_SEVERITY SAGE_LOG_ENABLED_SEVERITY
#else
    #define SAGE_INTERNAL_ENABLED_SEVERITY (\
            static_cast<sage::types::byte>(sage::Severity::TRACE) \
            | static_cast<sage::types::byte>(sage::Severity::DEBUG) \
            | static_cast<sage::types::byte>(sage::Severity::INFO) \
            | static_cast<sage::types::byte>(sage::Severity::WARN) \
            | static_cast<sage::types::byte>(sage::Severity::ERROR) \
            | static_cast<sage::types::byte>(sage::Severity::FATAL))
#endif

#ifdef SAGE_LOG_NO_FORMATTING
    #define SAGE_INTERNAL_FORMATTING false
#else
    #define SAGE_INTERNAL_FORMATTING true
#endif

#ifdef SAGE_LOG_NO_COLORS
    #define SAGE_INTERNAL_COLORS false
#else
    #define SAGE_INTERNAL_COLORS true
#endif

#ifdef SAGE_LOG_WRITE_TO_FILE
    #define SAGE_INTERNAL_WRITE_TO_FILE true
#else
    #define SAGE_INTERNAL_WRITE_TO_FILE false
#endif

#ifdef SAGE_LOG_THROW_ON_ERROR
    #define SAGE_INTERNAL_THROW_ON_ERROR true
#else
    #define SAGE_INTERNAL_THROW_ON_ERROR false
#endif

#ifdef SAGE_LOG_THROW_ON_FATAL
    #define SAGE_INTERNAL_THROW_ON_FATAL true
#else
    #define SAGE_INTERNAL_THROW_ON_FATAL false
#endif

// Utility functions
namespace sage::_backend::log
{
namespace defaults
{
    inline void write_std(sage::Severity level, const std::string_view& msg)
    {
        auto* write_file = level >= sage::Severity::WRN ? stderr : stdout;
        auto  write_size = std::fwrite(msg.data(), 1, msg.size(), write_file);
    }
    inline void write_file(const std::string_view& msg)
    {
        static FILE* crashfile  = fopen("last_run.log", "w");
        auto         write_size = std::fwrite(msg.data(), 1, msg.size(), crashfile);
    }
    constexpr std::string_view format_prefix(sage::Severity severity)
    {
        switch (severity)
        {
            case sage::Severity::TRC:
                return "TRC";
            case sage::Severity::DBG:
                return "DBG";
            case sage::Severity::Info:
                return "INF";
            case sage::Severity::WRN:
                return "WRN";
            case sage::Severity::ERR:
                return "ERR";
            case sage::Severity::FTL:
                return "FTL";

            default:
                return "N/A";
        }
        std::unreachable();
    }
    constexpr std::string_view format_color(sage::Severity severity)
    {
        switch (severity)
        {
            case sage::Severity::TRC:
                return "\033[0;37m";  // Gray
            case sage::Severity::DBG:
                return "\033[0;35m";  // Purple
            case sage::Severity::Info:
                return "\033[0;32m";  // Green
            case sage::Severity::WRN:
                return "\033[0;33m";  // Yellow
            case sage::Severity::ERR:
                return "\033[0;31m";  // Red
            case sage::Severity::FTL:
                return "\033[1;91m";  // DarkRed

            default:
                return "";
        }
        std::unreachable();
    }
}  // namespace defaults

export struct Config
{
    sage::types::byte enabled_severity = SAGE_INTERNAL_ENABLED_SEVERITY;
    bool              formatting       = SAGE_INTERNAL_FORMATTING;
    bool              colors           = SAGE_INTERNAL_COLORS;
    bool              write_to_file    = SAGE_INTERNAL_WRITE_TO_FILE;

    bool throw_on_error = SAGE_INTERNAL_THROW_ON_ERROR;
    bool throw_on_fatal = SAGE_INTERNAL_THROW_ON_FATAL;

    void (*std_writer)(sage::Severity, const std::string_view&) = &defaults::write_std;
    void (*file_writer)(const std::string_view&)                = &defaults::write_file;

    std::string_view (*format_prefix)(sage::Severity) = &defaults::format_prefix;
    std::string_view (*format_color)(sage::Severity)  = &defaults::format_color;
};

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


export template<Config cfg = Config{}>
class Logger
{
  private:
    // msg has to accept Rvalue and Lvalue, hence copy is needed
    template<sage::Severity level>
    static void log_impl(const std::source_location& whence, std::string msg)
    {
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) == 0)
            return;

        if constexpr (cfg.formatting)
        {
            const std::string_view fpath     = whence.file_name();
            const std::string_view func_name = whence.function_name();
            msg                              = std::format(
                "==[{}:{}:{}]==\n[{}]\t{}\n",
                fpath.substr(fpath.find_last_of("/\\") + 1),
                func_name.substr(func_name.find_last_of(' ') + 1),
                whence.line(),
                cfg.format_prefix(level),
                msg
            );
        }
        if constexpr (cfg.write_to_file)
            cfg.file_writer(msg);

        if constexpr (cfg.colors)
            msg = std::format("{}{}{}", cfg.format_color(level), msg, "\e[0m");

        cfg.std_writer(level, msg);
    }

  public:
    template<typename... Args>
    static void trace(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::TRC>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void trace(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::TRC>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void debug(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::DBG>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void debug(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::DBG>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void info(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::Info>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void info(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::Info>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void warn(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::WRN>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );
    }
    template<typename... Args>
    static void warn(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::WRN>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );
    }


    template<typename... Args>
    static void error(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::ERR>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );

        if constexpr (cfg.throw_on_error)
            throw std::runtime_error(fmt.format.get().data());
    }
    template<typename... Args>
    static void error(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::ERR>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );

        if constexpr (cfg.throw_on_error)
            throw std::runtime_error(fmt.format.get().data());
    }


    template<typename... Args>
    static void fatal(logger_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::FTL>(
            fmt.location,
            std::format(fmt.format, std::forward<Args>(args)...)
        );

        if constexpr (cfg.throw_on_fatal)
            throw std::runtime_error(fmt.format.get().data());
        else
            abort();
    }
    template<typename... Args>
    static void fatal(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        log_impl<sage::Severity::FTL>(
            fmt.location,
            std::vformat(fmt.format, std::make_format_args(args...))
        );

        if constexpr (cfg.throw_on_fatal)
            throw std::runtime_error(fmt.format.get().data());
        else
            abort();
    }
};
}  // namespace sage::_backend::log


#ifndef SAGE_LOG_NO_DEFAULT_LOGGER
namespace sage
{
export using log = sage::_backend::log::Logger<>;
}
#endif
