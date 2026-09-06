#include "gtest/gtest.h"
#include <cstdio>

import sage;

static void caller()
{
    testing::internal::CaptureStdout();
    constexpr int line_nr = __LINE__ + 1;
    sage::log::trace("{}:{}", "Hello world!", 5);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(
        output,
        std::format(
            "\x1B[0;37m==[output_default_test.cpp:caller():{}]==\n[TRC]\tHello world!:5\x1B[0m",
            line_nr
        )
    );
}

TEST(Logger, CorrectOutput)
{
    testing::internal::CaptureStdout();
    sage::log::trace("Hello world!");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Hello world!"), std::string::npos);

    testing::internal::CaptureStdout();
    sage::log::debug("Hello world!");
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Hello world!"), std::string::npos);

    testing::internal::CaptureStdout();
    sage::log::info("Hello world!");
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Hello world!"), std::string::npos);

    testing::internal::CaptureStderr();
    sage::log::warn("Hello world!");
    output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Hello world!"), std::string::npos);

    testing::internal::CaptureStderr();
    sage::log::warn("Hello world!");
    output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Hello world!"), std::string::npos);

    testing::internal::CaptureStderr();
    sage::log::error("Hello world!");
    output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Hello world!"), std::string::npos);
}
TEST(Logger, FatalAbort)
{
    // Check if fatal is death + correct print
    EXPECT_DEATH({ sage::log::fatal("Goodbye world!"); }, ".*Goodbye world!.*");

    // Check if dumpfile was created
}

TEST(Logger, VARGS)
{
    // Vargs
    testing::internal::CaptureStdout();
    sage::log::trace("{}:{}", "Hello world!", 5);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Hello world!:5"), std::string::npos);

    // Variable usage
    std::string fmt = "{}:{}";
    testing::internal::CaptureStdout();
    sage::log::trace(fmt, "Hello world!", 1);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Hello world!:1"), std::string::npos);
}

TEST(Logger, CorrectFormat) { caller(); }

#ifdef SAGE_LOG_WRITE_TO_FILE
TEST(Logger, CrashlogCreated)
{
    sage::log::trace("Create crashlog");
    FILE* file = fopen("last_run.log", "r");
    EXPECT_FALSE(file == nullptr);
    fclose(file);
}
#endif
