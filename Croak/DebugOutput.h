#pragma once
#include <Windows.h>
#include <hstring.h>
#include <string>

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

#ifdef DEBUG
/**
 Debug break shortcut macro.
 If DEBUG is not defined, the macro is empty.
*/
#define DEBUG_BREAK() if (IsDebuggerPresent()) __debugbreak()
#else
#define DEBUG_BREAK() 
#endif // DEBUG