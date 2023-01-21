#pragma once
namespace Croak
{
	__interface IComEventImplementation
	{
        /**
         * @brief Register the com event implementation for events.
         * @return True if the implementation registered successfully
        */
        virtual bool Register();

        /**
         * @brief Unregister the com event implementation from events.
         * @return True if the implementation unregistered successfully
        */
        virtual bool Unregister();
	};
}