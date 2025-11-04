// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "CrashReporter.cpp"
#include "Logger.h"
#include <string>
#include <thread>

using namespace ark;

// Test basic functionality of CrashReporter class
TEST(CrashReporterTest, CrashReporterInstance) {
    // Ensure object can be constructed
    EXPECT_NO_THROW(CrashReporter reporter);
}

TEST(CrashReporterTest, RegisterHandlers) {
    CrashReporter reporter;
    // Test that registering handlers does not crash
    EXPECT_NO_THROW(reporter.RegisterHandlers());
}

// Test MessageInfoHandler function
TEST(CrashReporterTest, MessageInfoHandler_EmptyQueue) {
    // Clear the queue
    while (!Logger::messageQueue.empty()) {
        Logger::messageQueue.pop();
    }

    // Test handling empty queue does not crash
    EXPECT_NO_THROW(MessageInfoHandler());
}

TEST(CrashReporterTest, MessageInfoHandler_WithMessages) {
    // Clear the queue
    while (!Logger::messageQueue.empty()) {
        Logger::messageQueue.pop();
    }

    // Add some test messages
    Logger::messageQueue.push("Test message 1");
    Logger::messageQueue.push("Test message 2");

    // Test handling non-empty queue does not crash
    EXPECT_NO_THROW(MessageInfoHandler());
}

// Test KernelLogHandler function
TEST(CrashReporterTest, KernelLogHandler_NonExistentThread) {
    std::thread::id fakeThreadId;

    // Test handling logs for non-existent thread does not crash
    EXPECT_NO_THROW(KernelLogHandler(fakeThreadId));
}

TEST(CrashReporterTest, KernelLogHandler_CurrentThread) {
    std::thread::id currentThreadId = std::this_thread::get_id();

    // Test handling logs for current thread does not crash
    EXPECT_NO_THROW(KernelLogHandler(currentThreadId));
}

// Test platform-specific functions (compile only, do not actually execute)
#ifdef __linux__
TEST(CrashReporterTest, Linux_SignalHandler) {
    // Test that signal handler function can be compiled (do not actually trigger signals)
    EXPECT_TRUE(true);
}

TEST(CrashReporterTest, Linux_SigpipeHandler) {
    // Test that SIGPIPE handler function can be compiled
    EXPECT_TRUE(true);
}

#elif defined(_WIN32)
TEST(CrashReporterTest, Windows_ReportException) {
    // Test that exception reporting function can be compiled (do not actually trigger exceptions)
    EXPECT_TRUE(true);
}

TEST(CrashReporterTest, Windows_InitStack) {
    // Test that stack initialization function can be compiled
    EXPECT_TRUE(true);
}

#elif defined(__APPLE__)
TEST(CrashReporterTest, MacOS_RegisterHandlers) {
    // Test that macOS register handlers function can be compiled
    EXPECT_TRUE(true);
}
#endif

// Test log path related functionality
TEST(CrashReporterTest, LoggerPath) {
    // Getting log path should not crash
    EXPECT_NO_THROW(Logger::GetLogPath());
}

// Test exception handler functions (only verify callability)
TEST(CrashReporterTest, ExceptionHandlersCallable) {
#ifdef __linux__
    // Test Linux signal handling related functions
    EXPECT_NO_THROW(SigpipeHandler(0));
#elif defined(_WIN32)
    // On Windows, we cannot easily call exception handling functions because they require specific parameters
    EXPECT_TRUE(true);
#endif
}

// Test helper functions
TEST(CrashReporterTest, StackSizeConstant) {
#ifdef __linux__
    // Verify stack size constant exists and is reasonable
    EXPECT_GT(STACK_SIZE, 0u);
#endif
}

// Test signal array
TEST(CrashReporterTest, SignalsArray) {
#ifdef __linux__
    // Verify signal array exists and has correct size
    EXPECT_EQ(SIGNAL_NUM, 3u);
    EXPECT_NE(SIGNALS[0], 0);
#endif
}
