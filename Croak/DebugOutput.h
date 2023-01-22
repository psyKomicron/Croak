#pragma once
#include <Windows.h>
#include <hstring.h>
#include <string>

#ifdef DEBUG
/**
 Debug break shortcut macro.
 If DEBUG is not defined, the macro is empty.
*/
#define DEBUG_BREAK() if (IsDebuggerPresent()) __debugbreak()
#else
#define DEBUG_BREAK() 
#endif // DEBUG

/**
 * @brief Outputs a hstring (wide string) to the debug ouptput, followed by a break line.
 * @param text
*/
inline void OutputDebugHString(winrt::hstring const& text)
{
    OutputDebugString(text.c_str());
    OutputDebugString(L"\n");
}

/**
 * @brief Outputs a wide string to the debug output, followed by a break line.
 * @param text
*/
inline void OutputDebugWString(const std::wstring& text)
{
    OutputDebugString(text.c_str());
    OutputDebugString(L"\n");
}

/**
 * @brief Outputs a wide string to the debug output, followed by a break line.
 * @param text
*/
inline void OutputDebugWString(const std::wstring_view& text)
{
    OutputDebugString(text.data());
    OutputDebugString(L"\n");
}

inline void DebugLog(const std::string& text, const std::source_location& sourceLocation = std::source_location::current())
{
    std::string fileName = std::filesystem::path(sourceLocation.file_name()).filename().string();
    std::string functionName = sourceLocation.function_name();

    fileName = fileName.substr(0, fileName.find_last_of('.'));

    std::string out = std::format(
        "DEBUG: {0}::{4}({1},{2}): {3}", 
        fileName, 
        std::to_string(sourceLocation.line()), std::to_string(sourceLocation.column()), 
        text,
        functionName);

    OutputDebugString(winrt::to_hstring(out).c_str());
}