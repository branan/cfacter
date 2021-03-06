/**
 * @file
 * Declares the OSX networking fact resolver.
 */
#ifndef FACTER_FACTS_OSX_NETWORKING_RESOLVER_HPP_
#define FACTER_FACTS_OSX_NETWORKING_RESOLVER_HPP_

#include "../bsd/networking_resolver.hpp"

namespace facter { namespace facts { namespace osx {

    /**
     * Responsible for resolving networking facts.
     */
    struct networking_resolver : bsd::networking_resolver
    {
     protected:
        /**
         * Determines if the given sock address is a link layer address.
         * @param addr The socket address to check.
         * @returns Returns true if the socket address is a link layer address or false if it is not.
         */
        virtual bool is_link_address(sockaddr const* addr) const;

        /**
         * Gets the bytes of the link address.
         * @param addr The socket address representing the link address.
         * @return Returns a pointer to the address bytes or nullptr if not a link address.
         */
        virtual uint8_t const* get_link_address_bytes(sockaddr const* addr) const;

        /**
         * Gets the MTU of the link layer data.
         * @param interface The name of the link layer interface.
         * @param data The data pointer from the link layer interface.
         * @return Returns The MTU of the interface or -1 if there's no MTU.
         */
        virtual int get_link_mtu(std::string const& interface, void* data) const;

        /**
         * Called to resolve the hostname fact.
         * @param facts The fact map that is resolving facts.
         */
        virtual void resolve_hostname(fact_map& facts);

        /**
         * Gets the primary interface.
         * This is typically the interface of the default route.
         * @return Returns the primary interface or empty string if one could not be determined.
         */
        virtual std::string get_primary_interface();

        /**
         * Finds known DHCP servers for all interfaces.
         * @return Returns a map between interface name and DHCP server.
         */
        virtual std::map<std::string, std::string> find_dhcp_servers();

        /**
         * Finds the DHCP server for the given interface.
         * @param interface The interface to find the DHCP server for.
         * @returns Returns the DHCP server for the interface or empty string if one isn't found.
         */
        virtual std::string find_dhcp_server(std::string const& interface);
    };

}}}  // namespace facter::facts::osx

#endif  // FACTER_FACTS_OSX_NETWORKING_RESOLVER_HPP_
