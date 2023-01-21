#include "pch.h"
#include "NumberBlock.h"
#if __has_include("NumberBlock.g.cpp")
#include "NumberBlock.g.cpp"
#endif

#include <math.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::Croak::implementation
{
    DependencyProperty NumberBlock::_contentProperty = DependencyProperty::Register(
        L"Text",
        xaml_typename<hstring>(),
        xaml_typename<Croak::NumberBlock>(),
        PropertyMetadata(box_value(L""))
    );

    NumberBlock::NumberBlock()
    {
        DefaultStyleKey(winrt::box_value(L"Croak.NumberBlock"));
    }

    void NumberBlock::UpdateText()
    {
        uint32_t multiplicator = (10 * decimals) + (1 * (decimals == 0));
        double value = round(number * multiplicator);
        value /= multiplicator;

        Text(to_hstring(value));
    }
}
