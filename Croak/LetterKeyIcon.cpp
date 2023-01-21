#include "pch.h"
#include "LetterKeyIcon.h"
#if __has_include("LetterKeyIcon.g.cpp")
#include "LetterKeyIcon.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::Croak::implementation
{
    DependencyProperty LetterKeyIcon::_keyProperty = DependencyProperty::Register(
        L"Key",
        xaml_typename<hstring>(),
        xaml_typename<Croak::LetterKeyIcon>(),
        PropertyMetadata(box_value(L""))
    );

    LetterKeyIcon::LetterKeyIcon()
    {
        DefaultStyleKey(winrt::box_value(L"Croak.LetterKeyIcon"));
    }

    LetterKeyIcon::LetterKeyIcon(const hstring& key) : LetterKeyIcon()
    {
        SetValue(_keyProperty, box_value(key));
    }


    hstring LetterKeyIcon::Key() const
    {
        return unbox_value<hstring>(GetValue(_keyProperty));
    }

    void LetterKeyIcon::Key(const hstring& value)
    {
        SetValue(_keyProperty, box_value(value));
    }
}
