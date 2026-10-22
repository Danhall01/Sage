module;

#include <concepts>
#include <cstdio>
#include <format>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
export module sage:io.logger;
export import :core.severity;
export import :core.types;

#ifdef SAGE_LOG_ENABLED_SEVERITY
    #define SAGE_INTERNAL_ENABLED_SEVERITY SAGE_LOG_ENABLED_SEVERITY
#else
    #define SAGE_INTERNAL_ENABLED_SEVERITY (\
            static_cast<sage::types::byte>(sage::Severity::Trace) \
            | static_cast<sage::types::byte>(sage::Severity::Debug) \
            | static_cast<sage::types::byte>(sage::Severity::Info) \
            | static_cast<sage::types::byte>(sage::Severity::Warn) \
            | static_cast<sage::types::byte>(sage::Severity::Error) \
            | static_cast<sage::types::byte>(sage::Severity::Fatal))
#endif

#if defined(SAGE_LOG_NO_FORMATTING) && SAGE_LOG_NO_FORMATTING
    #define SAGE_INTERNAL_FORMATTING false
#else
    #define SAGE_INTERNAL_FORMATTING true
#endif

#ifdef SAGE_LOG_FORMATTING_OPT_WHENCE_SEVERITY
    #define SAGE_INTERNAL_FORMATTING_WHENCE_SEVERITY SAGE_LOG_FORMATTING_OPT_WHENCE_SEVERITY
#else
    #define SAGE_INTERNAL_FORMATTING_WHENCE_SEVERITY (\
            static_cast<sage::types::byte>(sage::Severity::Warn) \
            | static_cast<sage::types::byte>(sage::Severity::Error) \
            | static_cast<sage::types::byte>(sage::Severity::Fatal))
#endif

#if defined(SAGE_LOG_NO_COLORS) && SAGE_LOG_NO_COLORS
    #define SAGE_INTERNAL_COLORS false
#else
    #define SAGE_INTERNAL_COLORS true
#endif

#if defined(SAGE_LOG_WRITE_TO_FILE) && SAGE_LOG_WRITE_TO_FILE
    #define SAGE_INTERNAL_WRITE_TO_FILE true
#else
    #define SAGE_INTERNAL_WRITE_TO_FILE false
#endif

#if defined(SAGE_LOG_THROW_ON_ERROR) && SAGE_LOG_THROW_ON_ERROR
    #define SAGE_INTERNAL_THROW_ON_ERROR true
#else
    #define SAGE_INTERNAL_THROW_ON_ERROR false
#endif

#if defined(SAGE_LOG_THROW_ON_FATAL) && SAGE_LOG_THROW_ON_FATAL
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
        auto* write_file = level >= sage::Severity::Warn ? stderr : stdout;
        std::fwrite(msg.data(), 1, msg.size(), write_file);
        if (level >= sage::Severity::Error)
            std::fflush(write_file);
    }
    inline void write_file(const std::string_view& msg)
    {
        static auto crashfile = std::unique_ptr<FILE, decltype(&std::fclose)>(
            std::fopen("last_run.log", "w"),
            [](FILE* fp) -> int  // C++ does not sanction taking adressess of standard functions
            {
                if (fp)
                    return std::fclose(fp);
                return EOF;
            }
        );
        if (crashfile.get() == nullptr)
            return;

        std::fwrite(msg.data(), sizeof(sage::types::char8), msg.size(), crashfile.get());
        fflush(crashfile.get());
    }
    constexpr std::string_view format_prefix(sage::Severity severity)
    {
        switch (severity)
        {
            case sage::Severity::Trace:
                return "TRC";
            case sage::Severity::Debug:
                return "DBG";
            case sage::Severity::Info:
                return "INF";
            case sage::Severity::Warn:
                return "WRN";
            case sage::Severity::Error:
                return "ERR";
            case sage::Severity::Fatal:
                return "FTL";

            default:
                return "N/A";
        }
    }
    constexpr std::string_view format_color(sage::Severity severity)
    {
        switch (severity)
        {
            case sage::Severity::Trace:
                return "\033[0;37m";  // Gray
            case sage::Severity::Debug:
                return "\033[0;35m";  // Purple
            case sage::Severity::Info:
                return "\033[0;32m";  // Green
            case sage::Severity::Warn:
                return "\033[0;33m";  // Yellow
            case sage::Severity::Error:
                return "\033[0;31m";  // Red
            case sage::Severity::Fatal:
                return "\033[1;91m";  // DarkRed

            default:
                return "";
        }
    }
}  // namespace defaults

export struct Config
{
    sage::types::byte enabled_severity = SAGE_INTERNAL_ENABLED_SEVERITY;
    bool              formatting       = SAGE_INTERNAL_FORMATTING;
    bool              colors           = SAGE_INTERNAL_COLORS;
    bool              write_to_file    = SAGE_INTERNAL_WRITE_TO_FILE;

    sage::types::byte formatting_whence = SAGE_INTERNAL_FORMATTING_WHENCE_SEVERITY;

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
    template<sage::Severity level, sage::types::bool8 capture = false>
    static auto log_impl(const std::source_location& whence, std::string_view msg)
        -> std::conditional_t<capture, std::string, void>
    {
        constexpr sage::types::bool8 use_buffer = cfg.formatting || cfg.colors || capture;
        constexpr sage::types::bool8 keep_plain = (cfg.write_to_file && cfg.file_writer != nullptr)
                                                  || capture;

        constexpr sage::types::bool8 color_merge = cfg.colors && !keep_plain;
        constexpr std::string_view   color_reset = "\033[0m";
        constexpr std::string_view   color       = cfg.colors ? cfg.format_color(level)
                                                              : std::string_view{};

        std::string buffer;
        if constexpr (use_buffer)
        {
            buffer.reserve(msg.size() + 512);

            if constexpr (color_merge)
                buffer += color;

            if constexpr (cfg.formatting)
            {
                const std::string_view fpath     = whence.file_name();
                const std::string_view func_name = whence.function_name();

                std::string_view format;

                std::format_to(
                    std::back_inserter(buffer),
                    "{}:{} [{}]  {}\n",
                    fpath.substr(fpath.find_last_of("/\\") + 1),
                    whence.line(),
                    cfg.format_prefix(level),
                    msg
                );
                if constexpr ((static_cast<sage::types::byte>(level) & cfg.formatting_whence) != 0)
                {
                    buffer += "\t\t↳ ";
                    buffer += whence.function_name();
                    buffer += '\n';
                }
            }
            else
            {
                buffer += msg;
            }

            if constexpr (color_merge)
                buffer += color_reset;
        }

        std::string_view out = use_buffer ? std::string_view{buffer} : msg;

        if constexpr (cfg.write_to_file && cfg.file_writer != nullptr)
            cfg.file_writer(out);

        if constexpr (keep_plain && cfg.colors)
        {
            buffer.insert(0, color);
            buffer += color_reset;
            out     = buffer;  // Insert might realloc
        }

        if constexpr (cfg.std_writer != nullptr)
            cfg.std_writer(level, out);

        if constexpr (capture)
        {
            if constexpr (cfg.colors)
            {
                buffer.erase(0, color.size());
                buffer.resize(buffer.size() - color_reset.size());
            }
            return buffer;
        }
    }

  public:
    template<typename... Args>
    static void trace(logger_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Trace;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void trace(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Trace;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::vformat(fmt.format, std::make_format_args(args...)));
    }


    template<typename... Args>
    static void debug(logger_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Debug;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void debug(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Debug;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::vformat(fmt.format, std::make_format_args(args...)));
    }


    template<typename... Args>
    static void info(logger_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Info;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void info(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Info;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::vformat(fmt.format, std::make_format_args(args...)));
    }


    template<typename... Args>
    static void warn(logger_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Warn;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void warn(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Warn;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
            log_impl<level>(fmt.location, std::vformat(fmt.format, std::make_format_args(args...)));
    }


    template<typename... Args>
    static void error(logger_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Error;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
        {
            if constexpr (cfg.throw_on_error)
            {
                throw std::runtime_error(
                    log_impl<level, true>(
                        fmt.location,
                        std::format(fmt.format, std::forward<Args>(args)...)
                    )
                );
            }
            else
            {
                log_impl<level>(fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
            }
        }
    }
    template<typename... Args>
    static void error(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Error;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
        {
            if constexpr (cfg.throw_on_error)
            {
                throw std::runtime_error(
                    log_impl<level, true>(
                        fmt.location,
                        std::vformat(fmt.format, std::make_format_args(args...))
                    )
                );
            }
            else
            {
                log_impl<level>(
                    fmt.location,
                    std::vformat(fmt.format, std::make_format_args(args...))
                );
            }
        }
    }


    template<typename... Args>
    static void fatal(logger_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Fatal;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
        {
            if constexpr (cfg.throw_on_fatal)
            {
                throw std::runtime_error(
                    log_impl<level, true>(
                        fmt.location,
                        std::format(fmt.format, std::forward<Args>(args)...)
                    )
                );
            }
            else
            {
                log_impl<level>(fmt.location, std::format(fmt.format, std::forward<Args>(args)...));
                std::abort();
            }
        }
    }
    template<typename... Args>
    static void fatal(logger_runtime_str_t<Args...> fmt, Args&&... args)
    {
        constexpr auto level = sage::Severity::Fatal;
        if constexpr ((static_cast<sage::types::byte>(level) & cfg.enabled_severity) != 0)
        {
            if constexpr (cfg.throw_on_fatal)
            {
                throw std::runtime_error(
                    log_impl<level, true>(
                        fmt.location,
                        std::vformat(fmt.format, std::make_format_args(args...))
                    )
                );
            }
            else
            {
                log_impl<level>(
                    fmt.location,
                    std::vformat(fmt.format, std::make_format_args(args...))
                );
                std::abort();
            }
        }
    }
};
}  // namespace sage::_backend::log


#ifndef SAGE_LOG_NO_DEFAULT_LOGGER
namespace sage
{
export using log = sage::_backend::log::Logger<>;
}
#endif
