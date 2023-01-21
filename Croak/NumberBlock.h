#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "NumberBlock.g.h"

namespace winrt::Croak::implementation
{
    struct NumberBlock : NumberBlockT<NumberBlock>
    {
        NumberBlock();

        static winrt::Microsoft::UI::Xaml::DependencyProperty TextProperty()
        {
            return _contentProperty;
        }

        inline winrt::hstring Text() const
        {
            return unbox_value<winrt::hstring>(GetValue(_contentProperty));
        }

        inline void Text(const winrt::hstring& value)
        {
            SetValue(_contentProperty, box_value(value));
        }

        inline double Double()
        {
            return number;
        }
        inline void Double(const double& value)
        {
            number = value;
            UpdateText();
        }

        inline float Single() const
        {
            return static_cast<float>(number);
        }

        inline void Single(const float& value)
        {
            number = value;
            UpdateText();
        }

        inline int32_t Int() const
        {
            return static_cast<int32_t>(number);
        }

        inline void Int(const int32_t& value)
        {
            number = static_cast<double>(value);
            UpdateText();
        }

        inline uint32_t UInt() const
        {
            return static_cast<uint32_t>(number);
        }

        inline void UInt(const uint32_t& value)
        {
            number = static_cast<double>(value);
            UpdateText();
        }

    private:
        static winrt::Microsoft::UI::Xaml::DependencyProperty _contentProperty;

        double number = 0.0;
        uint32_t decimals = 0u;

        void UpdateText();
    };
}

namespace winrt::Croak::factory_implementation
{
    struct NumberBlock : NumberBlockT<NumberBlock, implementation::NumberBlock>
    {
    };
}
