#include "gtest/gtest.h"
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#if defined(__linux__) || defined(__APPLE__)
    #include <unistd.h>
#endif

import sage;

namespace
{
using Config   = sage::_backend::log::Config;
using Severity = sage::Severity;

template<Config cfg>
using LoggerOf = sage::_backend::log::Logger<cfg>;

struct Written
{
    Severity    level;
    std::string text;
};

std::vector<Written>     g_std;   // everything the std writer received
std::vector<std::string> g_file;  // everything the file writer received
int                      g_format_calls = 0;

void capture_std(Severity level, const std::string_view& msg)
{ g_std.push_back({level, std::string(msg)}); }
void capture_file(const std::string_view& msg) { g_file.emplace_back(msg); }
void discard_std(Severity, const std::string_view&) {}

// Where a test logger sends its output.
enum class Sink
{
    Capture,  // both writers record into g_std / g_file
    Streams,  // the library's default std writer (real stdout / stderr)
    RealFile  // the library's default file writer (last_run.log), console output discarded
};

// Builds a Config starting from the library defaults, so only what a test names is overridden.
struct Opts
{
    unsigned mask        = 0x3F;
    bool     formatting  = false;
    bool     colors      = false;
    bool     file        = false;
    unsigned whence      = 0;     // levels that also print the calling function
    bool     throw_error = false;
    bool     throw_fatal = true;  // true so FATAL can be exercised without aborting
    Sink     sink        = Sink::Capture;
};
constexpr Config make(const Opts& o)
{
    Config c{};
    c.enabled_severity  = static_cast<sage::types::byte>(o.mask);
    c.formatting        = o.formatting;
    c.colors            = o.colors;
    c.write_to_file     = o.file;
    c.formatting_whence = static_cast<sage::types::byte>(o.whence);
    c.throw_on_error    = o.throw_error;
    c.throw_on_fatal    = o.throw_fatal;
    if (o.sink == Sink::Capture)
    {
        c.std_writer  = &capture_std;
        c.file_writer = &capture_file;
    }
    else if (o.sink == Sink::RealFile)
    {
        c.std_writer = &discard_std;
    }
    return c;
}

constexpr Severity all_levels[] = {
    Severity::Trace,
    Severity::Debug,
    Severity::Info,
    Severity::Warn,
    Severity::Error,
    Severity::Fatal
};
constexpr std::string_view color_reset = "\033[0m";

// Distinct, recognisable prefix / color per level, independent of the library defaults.
constexpr std::string_view tagged_prefix(Severity s)
{
    constexpr std::string_view names[] = {"<p0>", "<p1>", "<p2>", "<p3>", "<p4>", "<p5>"};
    return names[std::countr_zero(static_cast<unsigned>(s))];
}
constexpr std::string_view tagged_color(Severity s)
{
    constexpr std::string_view names[] = {"<c0>", "<c1>", "<c2>", "<c3>", "<c4>", "<c5>"};
    return names[std::countr_zero(static_cast<unsigned>(s))];
}
constexpr Config make_tagged_colors()
{
    Config c       = make({.colors = true});
    c.format_color = &tagged_color;
    return c;
}
constexpr Config make_tagged_prefixes()
{
    Config c        = make({.formatting = true});
    c.format_prefix = &tagged_prefix;
    return c;
}

using RawLog             = LoggerOf<make({})>;                    // message passed through
using FormattedLog       = LoggerOf<make({.formatting = true})>;  // record, no function line
using WhenceLog          = LoggerOf<make({.formatting = true, .whence = 0x3F})>;
using ColoredLog         = LoggerOf<make({.colors = true})>;
using TaggedColorLog     = LoggerOf<make_tagged_colors()>;
using TaggedPrefixLog    = LoggerOf<make_tagged_prefixes()>;
using ThrowErrLog        = LoggerOf<make({.throw_error = true})>;
using StreamLog          = LoggerOf<make({.throw_fatal = false, .sink = Sink::Streams})>;  // aborts
using StreamThrowLog     = LoggerOf<make({.sink = Sink::Streams})>;
using ThrowErrNoFatalLog = LoggerOf<make({.throw_error = true, .throw_fatal = false})>;

// Real file output. These run in child processes only.
using FileLog        = LoggerOf<make({.file = true, .throw_fatal = false, .sink = Sink::RealFile})>;
using FileRecordLog  = LoggerOf<make(
    {.formatting = true, .colors = true, .file = true, .throw_fatal = false, .sink = Sink::RealFile}
)>;
using FileNoTraceLog = LoggerOf<
    make({.mask = 0x3E, .file = true, .throw_fatal = false, .sink = Sink::RealFile})>;
using NoFileLog = LoggerOf<make({.file = false, .throw_fatal = false, .sink = Sink::RealFile})>;

struct CountedArg
{ int value; };

std::string this_file()
{
    const std::string_view path = std::source_location::current().file_name();
    return std::string(path.substr(path.find_last_of("/\\") + 1));
}

// Calls the logger function that matches `level` (FATAL is swallowed: capture configs throw).
template<typename L>
void emit_at_level(Severity level, const std::string& text)
{
    switch (level)
    {
        case Severity::Trace:
            L::trace("{}", text);
            break;
        case Severity::Debug:
            L::debug("{}", text);
            break;
        case Severity::Info:
            L::info("{}", text);
            break;
        case Severity::Warn:
            L::warn("{}", text);
            break;
        case Severity::Error:
            try
            {
                L::error("{}", text);
            }
            catch (const std::runtime_error&)
            {
            }
            break;
        case Severity::Fatal:
            try
            {
                L::fatal("{}", text);
            }
            catch (const std::runtime_error&)
            {
            }
            break;
    }
}
template<typename L>
void emit_all_levels(const std::string& text)
{
    for (auto level : all_levels)
        emit_at_level<L>(level, text);
}

std::vector<int> written_levels()
{
    std::vector<int> bits;
    for (const auto& w : g_std)
        bits.push_back(static_cast<int>(w.level));
    return bits;
}
std::vector<int> bits_of(std::initializer_list<Severity> levels)
{
    std::vector<int> bits;
    for (auto level : levels)
        bits.push_back(static_cast<int>(level));
    return bits;
}

bool contains(const std::string& haystack, std::string_view needle)
{ return haystack.find(needle) != std::string::npos; }

// Real streams and files are text mode on Windows: "\n" arrives as "\r\n".
std::string normalized(std::string text)
{
    std::erase(text, '\r');
    return text;
}
std::string read_file(const char* path)
{
    std::ifstream in(path, std::ios::binary);
    return normalized({std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()});
}

// Death tests that use the real last_run.log need a fresh process, not a fork of this one.
class ThreadsafeDeathTests
{
  public:
    ThreadsafeDeathTests() : saved_(GTEST_FLAG_GET(death_test_style))
    { GTEST_FLAG_SET(death_test_style, "threadsafe"); }
    ~ThreadsafeDeathTests() { GTEST_FLAG_SET(death_test_style, saved_); }
    ThreadsafeDeathTests(const ThreadsafeDeathTests&)            = delete;
    ThreadsafeDeathTests& operator=(const ThreadsafeDeathTests&) = delete;

  private:
    std::string saved_;
};

class LoggerCapture : public testing::Test
{
  protected:
    void SetUp() override
    {
        g_std.clear();
        g_file.clear();
        g_format_calls = 0;
    }
};

class LoggerLogFile : public testing::Test
{
  protected:
    void SetUp() override { std::remove("last_run.log"); }
    void TearDown() override { std::remove("last_run.log"); }
};
}  // namespace

template<>
struct std::formatter<CountedArg>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto           format(const CountedArg& arg, std::format_context& ctx) const
    {
        ++g_format_calls;
        return std::format_to(ctx.out(), "{}", arg.value);
    }
};

// Fixtures whose names must show up in the record. Global scope, because compilers decorate
// names inside anonymous namespaces differently.
static void probe_function_with_parameters(int, int) { WhenceLog::warn("probe"); }
struct ProbeClass
{
    void const_member() const { WhenceLog::warn("probe"); }
};

// Defined at the very end of the file (they rename the file with `#line`).
static void log_from_windows_style_path();
static void log_from_unix_style_path();
static void log_from_bare_file_name();

// ================================================================================================
// Compile definitions and defaults
// ================================================================================================

#if defined(SAGE_LOG_NO_FORMATTING) && SAGE_LOG_NO_FORMATTING
constexpr bool expected_formatting = false;
#else
constexpr bool expected_formatting = true;
#endif
#if defined(SAGE_LOG_NO_COLORS) && SAGE_LOG_NO_COLORS
constexpr bool expected_colors = false;
#else
constexpr bool expected_colors = true;
#endif
#if defined(SAGE_LOG_WRITE_TO_FILE) && SAGE_LOG_WRITE_TO_FILE
constexpr bool expected_write_to_file = true;
#else
constexpr bool expected_write_to_file = false;
#endif
#if defined(SAGE_LOG_THROW_ON_ERROR) && SAGE_LOG_THROW_ON_ERROR
constexpr bool expected_throw_on_error = true;
#else
constexpr bool expected_throw_on_error = false;
#endif
#if defined(SAGE_LOG_THROW_ON_FATAL) && SAGE_LOG_THROW_ON_FATAL
constexpr bool expected_throw_on_fatal = true;
#else
constexpr bool expected_throw_on_fatal = false;
#endif
#ifdef SAGE_LOG_ENABLED_SEVERITY
constexpr unsigned expected_mask = SAGE_LOG_ENABLED_SEVERITY;
#else
constexpr unsigned expected_mask = 0x3F;
#endif

TEST(LoggerConfig, DefaultsFollowTheCompileDefinitions)
{
    constexpr Config defaults{};
    EXPECT_EQ(defaults.enabled_severity, expected_mask);
    EXPECT_EQ(defaults.formatting, expected_formatting);
    EXPECT_EQ(defaults.colors, expected_colors);
    EXPECT_EQ(defaults.write_to_file, expected_write_to_file);
    EXPECT_EQ(defaults.throw_on_error, expected_throw_on_error);
    EXPECT_EQ(defaults.throw_on_fatal, expected_throw_on_fatal);
}

#ifndef SAGE_LOG_NO_DEFAULT_LOGGER
TEST(LoggerConfig, DefaultLoggerUsesTheDefaultConfig)
{
    EXPECT_TRUE((std::is_same_v<sage::log, sage::_backend::log::Logger<>>));
    EXPECT_TRUE((std::is_same_v<sage::log, sage::_backend::log::Logger<Config{}>>));
}
#endif

TEST(LoggerConfig, UnsetMembersKeepTheirDefaults)
{
    constexpr Config custom{.enabled_severity = 0b11'1100, .write_to_file = true};
    EXPECT_EQ(custom.enabled_severity, 0b11'1100);
    EXPECT_TRUE(custom.write_to_file);
    EXPECT_EQ(custom.colors, expected_colors);
    EXPECT_EQ(custom.formatting, expected_formatting);
    EXPECT_NE(custom.std_writer, nullptr);
    EXPECT_NE(custom.file_writer, nullptr);
    EXPECT_NE(custom.format_prefix, nullptr);
    EXPECT_NE(custom.format_color, nullptr);
}

// ================================================================================================
// Severity filtering
// ================================================================================================

template<unsigned Mask>
static void expect_only_levels(std::initializer_list<Severity> expected)
{
    using L = LoggerOf<make({.mask = Mask, .file = true})>;
    g_std.clear();
    g_file.clear();
    emit_all_levels<L>("m");
    EXPECT_EQ(written_levels(), bits_of(expected)) << "mask " << Mask;
    EXPECT_EQ(g_file.size(), expected.size()) << "mask " << Mask;  // file follows the same mask
}

TEST_F(LoggerCapture, EmptyMaskSilencesEverything) { expect_only_levels<0>({}); }

TEST_F(LoggerCapture, DisabledLevelsDoNotFormatTheirArguments)
{
    using OnlyError = LoggerOf<make({.mask = 0b01'0000})>;
    OnlyError::trace("{}", CountedArg{1});
    OnlyError::info("{}", CountedArg{2});
    const std::string runtime_fmt = "{}";
    OnlyError::debug(runtime_fmt, CountedArg{3});
    OnlyError::warn(runtime_fmt, CountedArg{4});
    EXPECT_EQ(g_format_calls, 0);
    EXPECT_TRUE(g_std.empty());

    OnlyError::error("{}", CountedArg{5});
    EXPECT_EQ(g_format_calls, 1);
}

TEST_F(LoggerCapture, ADisabledErrorDoesNotThrow)
{
    using NoError = LoggerOf<make({.mask = 0b10'1111, .throw_error = true})>;
    EXPECT_NO_THROW(NoError::error("masked out"));
    EXPECT_TRUE(g_std.empty());
}

TEST_F(LoggerCapture, ADisabledFatalDoesNotThrow)
{
    using NoFatal = LoggerOf<make({.mask = 0b01'1111, .throw_fatal = true})>;
    EXPECT_NO_THROW(NoFatal::fatal("masked out"));
    EXPECT_TRUE(g_std.empty());
}

TEST(LoggerDeathTest, ADisabledFatalDoesNotAbort)
{
    using NoFatal = LoggerOf<make({.mask = 0b01'1111, .throw_fatal = false})>;
    EXPECT_EXIT(
        {
            NoFatal::fatal("masked out");
            std::exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
}

// ================================================================================================
// Formatting
// ================================================================================================

TEST_F(LoggerCapture, WithoutFormattingTheMessageIsPassedThroughUntouched)
{
    emit_all_levels<RawLog>("hello world");
    RawLog::info("one");
    RawLog::info("two");
    ASSERT_EQ(g_std.size(), 8U);
    for (std::size_t i = 0; i < 6; ++i)
        EXPECT_EQ(g_std[i].text, "hello world");
    EXPECT_EQ(g_std[6].text, "one");  // no newline appended, no header
    EXPECT_EQ(g_std[7].text, "two");
}

TEST_F(LoggerCapture, FormattedRecordNamesFileLevelAndMessage)
{
    for (auto level : all_levels)
    {
        g_std.clear();
        emit_at_level<FormattedLog>(level, "the message");
        ASSERT_EQ(g_std.size(), 1U);
        const std::string& record = g_std[0].text;
        EXPECT_TRUE(contains(record, this_file())) << record;
        EXPECT_TRUE(contains(record, Config{}.format_prefix(level))) << record;
        EXPECT_TRUE(contains(record, "the message")) << record;
    }
}

TEST_F(LoggerCapture, FormattedRecordsEndWithANewlineSoTheyDoNotRunTogether)
{
    FormattedLog::info("first");
    FormattedLog::info("second");
    FormattedLog::warn("third");
    ASSERT_EQ(g_std.size(), 3U);
    for (const auto& w : g_std)
        EXPECT_TRUE(w.text.ends_with('\n')) << w.text;
}

TEST_F(LoggerCapture, FormattedRecordNamesTheLineOfTheCall)
{
    const int first = __LINE__ + 1;
    FormattedLog::info("a");
    const int second = __LINE__ + 1;
    FormattedLog::info("b");
    ASSERT_EQ(g_std.size(), 2U);
    EXPECT_TRUE(contains(g_std[0].text, std::to_string(first))) << g_std[0].text;
    EXPECT_FALSE(contains(g_std[0].text, std::to_string(second))) << g_std[0].text;
    EXPECT_TRUE(contains(g_std[1].text, std::to_string(second))) << g_std[1].text;
}

TEST_F(LoggerCapture, TheLineOfTheCallIsCorrectForRuntimeFormatStrings)
{
    const std::string fmt  = "runtime {}";
    const int         line = __LINE__ + 1;
    FormattedLog::info(fmt, 1);
    ASSERT_EQ(g_std.size(), 1U);
    EXPECT_TRUE(contains(g_std[0].text, std::to_string(line))) << g_std[0].text;
    EXPECT_TRUE(contains(g_std[0].text, "runtime 1"));
}

TEST_F(LoggerCapture, OnlyTheFileNameIsShownNotTheDirectory)
{
    // `#line` lets the fixtures at the end of this file pretend to live at other paths. A
    // backslash path is what MSVC reports.
    log_from_windows_style_path();
    log_from_unix_style_path();
    log_from_bare_file_name();
    ASSERT_EQ(g_std.size(), 3U);
    EXPECT_TRUE(contains(g_std[0].text, "windows_style.cpp")) << g_std[0].text;
    EXPECT_FALSE(contains(g_std[0].text, "fakewindowsdir")) << g_std[0].text;
    EXPECT_TRUE(contains(g_std[1].text, "unix_style.cpp")) << g_std[1].text;
    EXPECT_FALSE(contains(g_std[1].text, "fakeunixdir")) << g_std[1].text;
    EXPECT_TRUE(contains(g_std[2].text, "bare_name.cpp")) << g_std[2].text;
}

TEST_F(LoggerCapture, TheCallingFunctionIsShownOnlyForTheLevelsInTheWhenceMask)
{
    using OnlyWarnAndFatal = LoggerOf<make({.formatting = true, .whence = 0b10'1000})>;
    emit_all_levels<OnlyWarnAndFatal>("m");
    ASSERT_EQ(g_std.size(), 6U);
    for (const auto& w : g_std)
    {
        const bool expected = w.level == Severity::Warn || w.level == Severity::Fatal;
        EXPECT_EQ(contains(w.text, "emit_at_level"), expected)
            << "level " << static_cast<int>(w.level) << ": " << w.text;
    }
}

TEST_F(LoggerCapture, NoLevelShowsTheFunctionWhenTheWhenceMaskIsEmpty)
{
    emit_all_levels<FormattedLog>("m");
    for (const auto& w : g_std)
        EXPECT_FALSE(contains(w.text, "emit_at_level")) << w.text;
}

TEST_F(LoggerCapture, TheFunctionNameIsShownIntact)
{
    probe_function_with_parameters(1, 2);
    ProbeClass{}.const_member();
    ASSERT_EQ(g_std.size(), 2U);
    EXPECT_TRUE(contains(g_std[0].text, "probe_function_with_parameters")) << g_std[0].text;
    EXPECT_TRUE(contains(g_std[1].text, "const_member")) << g_std[1].text;
}

TEST_F(LoggerCapture, MessageTextIsNeverReinterpretedAsAFormatString)
{
    FormattedLog::info("{}", "{} {0} {:>5} {{}}");
    RawLog::info("{}", "{} {0} {:>5} {{}}");
    ASSERT_EQ(g_std.size(), 2U);
    EXPECT_TRUE(contains(g_std[0].text, "{} {0} {:>5} {{}}")) << g_std[0].text;
    EXPECT_EQ(g_std[1].text, "{} {0} {:>5} {{}}");
}

TEST_F(LoggerCapture, MultilineEmptyAndHugeMessagesSurviveIntact)
{
    std::string huge(100'000, 'x');
    huge[50'000] = '\0';  // an embedded NULL must not truncate anything

    RawLog::info("a\nb\n\nc");
    RawLog::info("");
    RawLog::info("{}", huge);
    FormattedLog::info("{}", huge);
    ASSERT_EQ(g_std.size(), 4U);
    EXPECT_EQ(g_std[0].text, "a\nb\n\nc");
    EXPECT_EQ(g_std[1].text, "");
    EXPECT_EQ(g_std[2].text, huge);
    EXPECT_TRUE(contains(g_std[3].text, huge));
}

// ================================================================================================
// Colors, file output and the way they combine
// ================================================================================================

TEST_F(LoggerCapture, ColorsWrapTheMessageInTheColorOfItsLevel)
{
    for (auto level : all_levels)
    {
        g_std.clear();
        emit_at_level<TaggedColorLog>(level, "colored");
        ASSERT_EQ(g_std.size(), 1U);
        EXPECT_EQ(
            g_std[0].text,
            std::string(tagged_color(level)) + "colored" + std::string(color_reset)
        );
    }
}

TEST_F(LoggerCapture, TheDefaultColorsAreAnsiEscapeSequencesThatAreResetAfterwards)
{
    for (auto level : all_levels)
    {
        g_std.clear();
        emit_at_level<ColoredLog>(level, "colored");
        ASSERT_EQ(g_std.size(), 1U);
        const std::string& text = g_std[0].text;
        EXPECT_TRUE(text.starts_with(Config{}.format_color(level)));
        EXPECT_TRUE(text.starts_with("\033["));
        EXPECT_TRUE(text.ends_with(color_reset));
    }
}

TEST_F(LoggerCapture, WithoutColorsNoEscapeByteIsEmitted)
{
    emit_all_levels<RawLog>("plain");
    emit_all_levels<FormattedLog>("plain");
    ASSERT_EQ(g_std.size(), 12U);
    for (const auto& w : g_std)
        EXPECT_FALSE(contains(w.text, "\033")) << w.text;
}

// Formatting, colors and file output are independent switches. The file always receives the
// plain record, the console the same record wrapped in color when colors are on.
template<bool Formatting, bool Colors, bool ToFile>
static void expect_combination()
{
    using L = LoggerOf<make({.formatting = Formatting, .colors = Colors, .file = ToFile})>;
    g_std.clear();
    g_file.clear();
    L::info("combo {}", 7);

    ASSERT_EQ(g_std.size(), 1U);
    ASSERT_EQ(g_file.size(), ToFile ? 1U : 0U);

    std::string plain = g_std[0].text;
    if (Colors)
    {
        const std::string color(Config{}.format_color(Severity::Info));
        ASSERT_TRUE(plain.starts_with(color));
        ASSERT_TRUE(plain.ends_with(color_reset));
        plain = plain.substr(color.size(), plain.size() - color.size() - color_reset.size());
    }
    if (Formatting)
    {
        EXPECT_TRUE(contains(plain, "combo 7"));
        EXPECT_TRUE(plain.ends_with('\n'));
        EXPECT_TRUE(contains(plain, this_file()));
    }
    else
    {
        EXPECT_EQ(plain, "combo 7");
    }
    if (ToFile)
        EXPECT_EQ(g_file[0], plain);
}

TEST_F(LoggerCapture, FormattingColorsAndFileOutputCombineIndependently)
{
    expect_combination<false, false, false>();
    expect_combination<false, false, true>();
    expect_combination<false, true, false>();
    expect_combination<false, true, true>();
    expect_combination<true, false, false>();
    expect_combination<true, false, true>();
    expect_combination<true, true, false>();
    expect_combination<true, true, true>();
}

TEST_F(LoggerCapture, FileOutputIsNeverColored)
{
    using Everything = LoggerOf<
        make({.formatting = true, .colors = true, .file = true, .whence = 0x3F})>;
    emit_all_levels<Everything>("m");
    ASSERT_EQ(g_file.size(), 6U);
    for (const auto& text : g_file)
        EXPECT_FALSE(contains(text, "\033")) << text;
}

TEST_F(LoggerCapture, TheFileReceivesTheFunctionLineToo)
{
    using WithFile = LoggerOf<make({.formatting = true, .file = true, .whence = 0x3F})>;
    WithFile::warn("m");
    ASSERT_EQ(g_file.size(), 1U);
    EXPECT_TRUE(contains(g_file[0], "TestBody")) << g_file[0];
}

TEST_F(LoggerCapture, ConfiguredPrefixFunctionIsUsed)
{
    for (auto level : all_levels)
    {
        g_std.clear();
        emit_at_level<TaggedPrefixLog>(level, "m");
        ASSERT_EQ(g_std.size(), 1U);
        EXPECT_TRUE(contains(g_std[0].text, tagged_prefix(level))) << g_std[0].text;
    }
}

TEST_F(LoggerCapture, EachLogCallReachesEachEnabledWriterExactlyOnce)
{
    using Both = LoggerOf<make({.file = true})>;
    Both::info("one");
    ASSERT_EQ(g_std.size(), 1U);
    ASSERT_EQ(g_file.size(), 1U);
    Both::warn("two");
    EXPECT_EQ(g_std.size(), 2U);
    EXPECT_EQ(g_file.size(), 2U);

    RawLog::info("three");  // write_to_file is off
    EXPECT_EQ(g_std.size(), 3U);
    EXPECT_EQ(g_file.size(), 2U);
}

// ================================================================================================
// Throwing and aborting
// ================================================================================================

TEST_F(LoggerCapture, ErrorThrowsARuntimeErrorAfterItHasBeenLogged)
{
    using WithFile = LoggerOf<make({.file = true, .throw_error = true})>;
    EXPECT_THROW(WithFile::error("boom"), std::runtime_error);
    ASSERT_EQ(g_std.size(), 1U);
    EXPECT_EQ(g_std[0].level, Severity::Error);
    EXPECT_TRUE(contains(g_std[0].text, "boom"));
    EXPECT_EQ(g_file.size(), 1U);
}

TEST_F(LoggerCapture, ErrorDoesNotThrowUnlessConfiguredTo)
{
    EXPECT_NO_THROW(RawLog::error("boom"));
    EXPECT_EQ(g_std.size(), 1U);
}

TEST_F(LoggerCapture, OnlyErrorThrowsWhenThrowOnErrorIsSet)
{
    EXPECT_NO_THROW(ThrowErrLog::trace("t"));
    EXPECT_NO_THROW(ThrowErrLog::debug("d"));
    EXPECT_NO_THROW(ThrowErrLog::info("i"));
    EXPECT_NO_THROW(ThrowErrLog::warn("w"));
    EXPECT_EQ(g_std.size(), 4U);
}

TEST_F(LoggerCapture, TheErrorExceptionCarriesTheFormattedMessage)
{
    try
    {
        ThrowErrLog::error("Value {} is bad", 5);
        FAIL() << "error() should have thrown";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_TRUE(contains(e.what(), "Value 5 is bad")) << e.what();
    }
}

TEST_F(LoggerCapture, TheExceptionTextCarriesNoColorCodes)
{
    using ColoredThrow = LoggerOf<make({.colors = true, .throw_error = true, .throw_fatal = true})>;
    try
    {
        ColoredThrow::error("colorless");
        FAIL() << "error() should have thrown";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_TRUE(contains(e.what(), "colorless"));
        EXPECT_FALSE(contains(e.what(), "\033")) << e.what();
    }
    ASSERT_EQ(g_std.size(), 1U);
    EXPECT_TRUE(contains(g_std[0].text, "\033"));  // the console copy is still colored
}

TEST_F(LoggerCapture, RuntimeFormatStringsThrowToo)
{
    const std::string fmt = "runtime {}";
    EXPECT_THROW(ThrowErrLog::error(fmt, 1), std::runtime_error);
    EXPECT_THROW(RawLog::fatal(fmt, 2), std::runtime_error);
    ASSERT_EQ(g_std.size(), 2U);
    EXPECT_TRUE(contains(g_std[0].text, "runtime 1"));
    EXPECT_TRUE(contains(g_std[1].text, "runtime 2"));
}

TEST_F(LoggerCapture, FatalThrowsARuntimeErrorInsteadOfAbortingWhenConfigured)
{
    try
    {
        RawLog::fatal("Code {}", 7);
        FAIL() << "fatal() should have thrown";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_TRUE(contains(e.what(), "Code 7")) << e.what();
    }
    ASSERT_EQ(g_std.size(), 1U);
    EXPECT_EQ(g_std[0].level, Severity::Fatal);
    EXPECT_TRUE(contains(g_std[0].text, "Code 7"));
}

TEST(LoggerDeathTest, FatalWritesItsMessageAndThenAborts)
{ EXPECT_DEATH(StreamLog::fatal("fatal {} of {}", 1, 2), "fatal 1 of 2"); }

TEST(LoggerDeathTest, FatalAbortsWithARuntimeFormatStringToo)
{
    EXPECT_DEATH(
        {
            const std::string fmt = "runtime fatal {}";
            StreamLog::fatal(fmt, 3);
        },
        "runtime fatal 3"
    );
}

// Typo tests
TEST_F(LoggerCapture, ThrowOnFatalDoesNotMakeErrorThrow)
{
    EXPECT_NO_THROW(RawLog::error("still fine"));  // RawLog: throw_on_fatal only
}
TEST(LoggerDeathTest, ThrowOnErrorDoesNotChangeWhatFatalDoes)
{ EXPECT_DEATH(ThrowErrNoFatalLog::fatal("abort me"), ""); }

// ================================================================================================
// Real standard streams
// ================================================================================================

TEST(LoggerStreams, InfoAndBelowGoToStdoutWarnAndAboveToStderr)
{
    const Config defaults{};
    for (auto level : all_levels)
    {
        testing::internal::CaptureStdout();
        testing::internal::CaptureStderr();
        defaults.std_writer(level, "payload");
        const std::string out = testing::internal::GetCapturedStdout();
        const std::string err = testing::internal::GetCapturedStderr();
        if (level >= Severity::Warn)
        {
            EXPECT_EQ(out, "") << static_cast<int>(level);
            EXPECT_EQ(err, "payload") << static_cast<int>(level);
        }
        else
        {
            EXPECT_EQ(out, "payload") << static_cast<int>(level);
            EXPECT_EQ(err, "") << static_cast<int>(level);
        }
    }
}

TEST(LoggerStreams, ALoggerWithTheDefaultWriterSplitsLevelsAcrossTheStreams)
{
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    emit_all_levels<StreamThrowLog>("m");
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "mmm");  // trace, debug, info
    EXPECT_EQ(testing::internal::GetCapturedStderr(), "mmm");  // warn, error, fatal
}

#if !defined(SAGE_LOG_NO_DEFAULT_LOGGER) && !defined(SAGE_LOG_ENABLED_SEVERITY)
TEST(LoggerStreams, TheDefaultLoggerWritesToTheStandardStreams)
{
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    sage::log::info("default info {}", 1);
    sage::log::warn("default warn {}", 2);
    const std::string out = normalized(testing::internal::GetCapturedStdout());
    const std::string err = normalized(testing::internal::GetCapturedStderr());
    EXPECT_TRUE(contains(out, "default info 1")) << out;
    EXPECT_FALSE(contains(out, "default warn"));
    EXPECT_TRUE(contains(err, "default warn 2")) << err;
    EXPECT_FALSE(contains(err, "default info"));
}
#endif

// ================================================================================================
// Arguments and format strings
// ================================================================================================

TEST_F(LoggerCapture, FormatSpecifiersAndPositionalArgumentsAreHonoured)
{
    RawLog::info("[{:>5}|{:<5}|{:^5}]", "a", "b", "c");
    RawLog::info("{:.2f} {:#x} {:+d}", 3.14159, 255, 5);
    RawLog::info("{1}{0}{1}", "a", "b");
    RawLog::info("[{:{}}]", 42, 6);
    RawLog::info("{{}} {}", true);
    ASSERT_EQ(g_std.size(), 5U);
    EXPECT_EQ(g_std[0].text, "[    a|b    |  c  ]");
    EXPECT_EQ(g_std[1].text, "3.14 0xff +5");
    EXPECT_EQ(g_std[2].text, "bab");
    EXPECT_EQ(g_std[3].text, "[    42]");
    EXPECT_EQ(g_std[4].text, "{} true");
}

TEST_F(LoggerCapture, UserDefinedFormattersAreSupportedAndRunOnce)
{
    RawLog::info("<{}>", CountedArg{12});
    ASSERT_EQ(g_std.size(), 1U);
    EXPECT_EQ(g_std[0].text, "<12>");
    EXPECT_EQ(g_format_calls, 1);
}

TEST_F(LoggerCapture, RuntimeFormatStringsOfEveryStringKindAreAccepted)
{
    std::string            mutable_fmt = "{}-{}";
    const std::string      const_fmt   = "{}+{}";
    const char*            c_string    = "{}*{}";
    const std::string_view view        = "{}/{}";
    RawLog::info(mutable_fmt, 1, 2);
    RawLog::info(const_fmt, 3, 4);
    RawLog::info(std::string("{}={}"), 5, 6);
    RawLog::info(view, 7, 8);
    RawLog::info(c_string, 9, 10);
    RawLog::info(std::string("no arguments"));
    ASSERT_EQ(g_std.size(), 6U);
    EXPECT_EQ(g_std[0].text, "1-2");
    EXPECT_EQ(g_std[1].text, "3+4");
    EXPECT_EQ(g_std[2].text, "5=6");
    EXPECT_EQ(g_std[3].text, "7/8");
    EXPECT_EQ(g_std[4].text, "9*10");
    EXPECT_EQ(g_std[5].text, "no arguments");
}

TEST_F(LoggerCapture, ABadRuntimeFormatStringThrowsFormatErrorAndWritesNothing)
{
    using Both = LoggerOf<make({.file = true})>;
    EXPECT_THROW(Both::info(std::string("{} {}"), 1), std::format_error);
    EXPECT_THROW(Both::info(std::string("{")), std::format_error);
    EXPECT_THROW(Both::error(std::string("{:d}"), "text"), std::format_error);
    EXPECT_TRUE(g_std.empty());
    EXPECT_TRUE(g_file.empty());

    Both::info("still works {}", 1);  // the logger is not left in a broken state
    ASSERT_EQ(g_std.size(), 1U);
    EXPECT_EQ(g_std[0].text, "still works 1");
}

// ================================================================================================
// Default log file (last_run.log)
//
// Each scenario runs in a child process; the parent then reads what the child left behind.
// ================================================================================================

TEST_F(LoggerLogFile, EveryLoggedMessageIsWrittenInOrder)
{
    ThreadsafeDeathTests style;
    EXPECT_EXIT(
        {
            FileLog::info("first|");
            FileLog::warn("second|");
            FileLog::error("third|");
            std::exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
    EXPECT_EQ(read_file("last_run.log"), "first|second|third|");
}

TEST_F(LoggerLogFile, TheFileHoldsTheUncoloredRecord)
{
    ThreadsafeDeathTests style;
    EXPECT_EXIT(
        {
            FileRecordLog::warn("record {}", 1);
            std::exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
    const std::string content = read_file("last_run.log");
    EXPECT_TRUE(contains(content, "record 1")) << content;
    EXPECT_TRUE(contains(content, this_file())) << content;
    EXPECT_FALSE(contains(content, "\033")) << content;
}

TEST_F(LoggerLogFile, DisabledLevelsAreNotWritten)
{
    ThreadsafeDeathTests style;
    EXPECT_EXIT(
        {
            FileNoTraceLog::trace("hidden|");
            FileNoTraceLog::info("visible|");
            std::exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
    EXPECT_EQ(read_file("last_run.log"), "visible|");
}

TEST_F(LoggerLogFile, NoFileIsCreatedWhenFileOutputIsOff)
{
    ThreadsafeDeathTests style;
    EXPECT_EXIT(
        {
            NoFileLog::info("console only");
            NoFileLog::error("console only");
            std::exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
    EXPECT_FALSE(std::filesystem::exists("last_run.log"));
}

// "on every log call": a message must be on disk as soon as the call returns, not only after a
// clean shutdown (std::_Exit skips the flush that exit() would do).
TEST_F(LoggerLogFile, EachMessageIsOnDiskWhenTheCallReturns)
{
    ThreadsafeDeathTests style;
    EXPECT_EXIT(
        {
            FileLog::info("must be on disk");
            std::_Exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
    EXPECT_TRUE(contains(read_file("last_run.log"), "must be on disk"));
}

TEST_F(LoggerLogFile, TheFatalMessageSurvivesTheAbort)
{
    ThreadsafeDeathTests style;
    EXPECT_DEATH(
        {
            FileLog::info("context before the crash");
            FileLog::fatal("the fatal message");
        },
        ""
    );
    const std::string content = read_file("last_run.log");
    EXPECT_TRUE(contains(content, "context before the crash")) << content;
    EXPECT_TRUE(contains(content, "the fatal message")) << content;
}

#if defined(__linux__) || defined(__APPLE__)
TEST_F(LoggerLogFile, AnUnwritableLocationDoesNotCrashTheProgram)
{
    if (!std::filesystem::exists("/proc/self"))
        GTEST_SKIP() << "needs a directory that cannot be written to";
    ThreadsafeDeathTests style;
    EXPECT_EXIT(
        {
            if (chdir("/proc") != 0)
                std::_Exit(2);
            FileLog::info("nowhere to write");
            FileLog::error("still nowhere");
            std::exit(0);
        },
        ::testing::ExitedWithCode(0),
        ""
    );
}
#endif

// ------------------------------------------------------------------------------------------------
// Fixtures for the file-name tests. They sit at the very end of the file because `#line`
// renames the file and renumbers every line after it.
// ------------------------------------------------------------------------------------------------

#line 4'242 "C:\\fakewindowsdir\\sub\\windows_style.cpp"
static void log_from_windows_style_path() { FormattedLog::info("windows"); }

#line 5'252 "/fakeunixdir/sub/unix_style.cpp"
static void log_from_unix_style_path() { FormattedLog::info("unix"); }

#line 6'262 "bare_name.cpp"
static void log_from_bare_file_name() { FormattedLog::info("bare"); }
