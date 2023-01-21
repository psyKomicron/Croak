#pragma once

#include "NavigationBreadcrumbBarItem.g.h"

namespace winrt::Croak::implementation
{
    struct NavigationBreadcrumbBarItem : NavigationBreadcrumbBarItemT<NavigationBreadcrumbBarItem>
    {
        NavigationBreadcrumbBarItem() = default;
        NavigationBreadcrumbBarItem(const hstring& displayName, const Windows::UI::Xaml::Interop::TypeName& itemTypeName) :
            _displayName(displayName),
            _itemTypeName(itemTypeName)
        {
        };

        hstring DisplayName() const
        {
            return _displayName;
        };
        void DisplayName(const hstring& value)
        {
            _displayName = value;
        };

        Windows::UI::Xaml::Interop::TypeName ItemTypeName() const
        {
            return _itemTypeName;
        };
        void ItemTypeName(const Windows::UI::Xaml::Interop::TypeName& value)
        {
            _itemTypeName = value;
        };

    private:
        hstring _displayName{};
        Windows::UI::Xaml::Interop::TypeName _itemTypeName{};
    };
}

namespace winrt::Croak::factory_implementation
{
    struct NavigationBreadcrumbBarItem : NavigationBreadcrumbBarItemT<NavigationBreadcrumbBarItem, implementation::NavigationBreadcrumbBarItem>
    {
    };
}
