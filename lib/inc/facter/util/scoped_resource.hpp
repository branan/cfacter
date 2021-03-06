/**
 * @file
 * Declares the base class for scoped resources.
 */
#ifndef FACTER_UTIL_SCOPED_RESOURCE_HPP_
#define FACTER_UTIL_SCOPED_RESOURCE_HPP_

#include <functional>

namespace facter { namespace util {
    /**
     * Simple class that is used for the RAII pattern.
     * Used to scope a resource.  When it goes out of scope, a deleter
     * function is called to delete the resource.
     * This type can be moved but cannot be copied.
     * @tparam T The type of resource being scoped.
    */
    template<typename T> struct scoped_resource
    {
        /**
         * Constructs a scoped_resource.
         * Takes ownership of the given resource.
         * @param resource The resource to scope.
         * @param deleter The function to call when the resource goes out of scope.
         */
        scoped_resource(T&& resource, std::function<void(T&)> deleter) :
            _resource(resource),
            _deleter(deleter)
        {
        }

        /**
         * Prevents the scoped_resource from being copied.
         */
        explicit scoped_resource(scoped_resource<T> const&) = delete;
        /**
         * Prevents the scoped_resource from being copied.
         * @returns Returns this scoped_resource.
         */
        scoped_resource& operator=(scoped_resource<T> const&) = delete;
        /**
         * Moves the given scoped_resource into this scoped_resource.
         * @param other The scoped_resource to move into this scoped_resource.
         */
        scoped_resource(scoped_resource<T>&& other) = default;
        /**
         * Moves the given scoped_resource into this scoped_resource.
         * @param other The scoped_resource to move into this scoped_resource.
         * @return Returns this scoped_resource.
         */
        scoped_resource& operator=(scoped_resource<T>&& other) = default;

        /**
         * Destructs a scoped_resource.
         */
        virtual ~scoped_resource()
        {
            release();
        }

        /**
         * Implicitly casts to T&.
         * @return Returns reference-to-T.
         */
        operator T&()
        {
            return _resource;
        }

        /**
         * Implicitly casts to T const&.
         * @return Returns const-reference-to-T.
         */
        operator T const&() const
        {
            return _resource;
        }

        /**
         * Releases the resource before destruction.
         */
        void release()
        {
            if (_deleter) {
                _deleter(_resource);
                _deleter = std::function<void(T&)>();
            }
        }

     protected:
        /**
         * Stores the resource being scoped.
         */
        T _resource;
        /**
         * Stores the function to call when the resource goes out of scope.
         */
        std::function<void(T&)> _deleter;
    };

}}  // namespace facter::util

#endif  // FACTER_UTIL_SCOPED_RESOURCE_HPP_
