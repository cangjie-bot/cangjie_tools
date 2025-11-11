// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CrashReporter.cpp"
#include <gtest/gtest.h>

using namespace ark;

TEST(CrashReporterTest, MessageInfoHandlerTest001)
{
    std::string message = "Test message 1";
    ark::Logger::messageQueue.push(message);

    MessageInfoHandler();

    std::string baseDir = ark::Logger::GetLogPath();
    std::string logDir = Cangjie::FileUtil::JoinPath(baseDir, ".log");
    std::string logFile = logDir + ark::FILE_SEPARATOR + "messageInfo.txt";
    std::ifstream ifs(logFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    EXPECT_NE(content.find(message), std::string::npos);
}

TEST(CrashReporterTest, KernelLogHandlerTest001)
{
    std::thread::id testThreadId = std::this_thread::get_id();

    std::vector<ark::KernelLog> msgs = {
        {"2023-10-01", "testFunction1", "success"}, {"2023-10-02", "testFunction2", "failed"}};
    for (const auto &msg : msgs) {
        ark::Logger::kernelLog[testThreadId].push_back(msg);
    }

    KernelLogHandler(testThreadId);

    std::string baseDir = ark::Logger::GetLogPath();
    std::string logDir = Cangjie::FileUtil::JoinPath(baseDir, ".log");
    std::string logFile = logDir + ark::FILE_SEPARATOR + "kernelLog.txt";
    std::ifstream ifs(logFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    for (const auto &msg : msgs) {
        std::string expectedLine = msg.date + ": function: " + msg.func + " state: " + msg.state;
        EXPECT_NE(content.find(expectedLine), std::string::npos);
    }
}

#ifdef __linux__
TEST(CrashReporterTest, PrintStackTraceOnSignalTest001)
{
    std::stringstream oss;
    std::string expected = "stack[0]:0x";

    PrintStackTraceOnSignal(oss);

    std::string output = oss.str();
    EXPECT_NE(output.find(expected), std::string::npos);
}

#elif defined(_WIN32)
TEST(CrashReporterTest, ReportExceptionTest001)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_ACCESS_VIOLATION;

    EXPECT_EQ("Access violation", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest002)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_BREAKPOINT;

    EXPECT_EQ("Exception: Breakpoint", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest003)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_ARRAY_BOUNDS_EXCEEDED;

    EXPECT_EQ("Exception: Array index out of bounds", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest004)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_STACK_OVERFLOW;

    EXPECT_EQ("Exception: Stack overflow", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest005)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_PRIV_INSTRUCTION;

    EXPECT_EQ("Exception: General Protection Fault", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest006)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_ILLEGAL_INSTRUCTION;

    EXPECT_EQ("Exception: Illegal instruction in program", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest007)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_INT_OVERFLOW;

    EXPECT_EQ("Exception: Integer overflow", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest008)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_INT_DIVIDE_BY_ZERO;

    EXPECT_EQ("Exception: Integer division by zero", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest009)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_FLT_UNDERFLOW;

    EXPECT_EQ("Exception: Floating point value underflow", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest010)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = 0x12345678;

    EXPECT_EQ("Unknown exception", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, InitStackTest001)
{
#ifdef _M_IX86

#elif _M_X64
    DWORD dwImageType = IMAGE_FILE_MACHINE_AMD64;
    STACKFRAME64 frame = {0};
    CONTEXT context = {0};
    context.Rip = 0x12345678;
    context.Rsp = 0x9ABCDEF0;

    InitStack(dwImageType, frame, context);

    EXPECT_EQ(dwImageType, IMAGE_FILE_MACHINE_AMD64);
    EXPECT_EQ(frame.AddrPC.Offset, 0x12345678);
    EXPECT_EQ(frame.AddrPC.Mode, AddrModeFlat);
    EXPECT_EQ(frame.AddrFrame.Offset, 0x9ABCDEF0);
    EXPECT_EQ(frame.AddrFrame.Mode, AddrModeFlat);
    EXPECT_EQ(frame.AddrStack.Offset, 0x9ABCDEF0);
    EXPECT_EQ(frame.AddrStack.Mode, AddrModeFlat);
#elif _M_IA64

#else
#error "Platform not supported!"
#endif
}

#endif

#ifdef __linux__
TEST(CrashReporterTest, RegisterHandlersTest001)
{
    CrashReporter::RegisterHandlers();
    EXPECT_EQ(1, 1);
}

#elif defined(_WIN32)
TEST(CrashReporterTest, VectoredExceptionHandlerTest001)
{
    EXCEPTION_RECORD exceptionRecord = {0};
    EXCEPTION_POINTERS exceptionPointers = {0};
    exceptionPointers.ExceptionRecord = &exceptionRecord;
    exceptionPointers.ContextRecord = new CONTEXT();
    exceptionPointers.ContextRecord->Rip = 0x12345678;
    exceptionPointers.ContextRecord->Rsp = 0x87654321;

    LONG result = VectoredExceptionHandler(&exceptionPointers);

    EXPECT_EQ(result, EXCEPTION_EXECUTE_HANDLER);
    delete exceptionPointers.ContextRecord;
}

TEST(CrashReporterTest, RegisterHandlersTest002)
{
    CrashReporter::RegisterHandlers();
    EXPECT_EQ(1, 1);
}


#elif defined(__APPLE__)

#endif