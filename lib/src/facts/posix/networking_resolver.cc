#include <facter/facts/posix/networking_resolver.hpp>
#include <facter/facts/fact.hpp>
#include <facter/facts/fact_map.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/logging/logging.hpp>
#include <facter/util/posix/scoped_addrinfo.hpp>
#include <facter/util/string.hpp>
#include <unistd.h>
#include <limits.h>
#include <limits>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <boost/format.hpp>

using namespace std;
using namespace facter::util;
using namespace facter::util::posix;
using boost::format;

LOG_DECLARE_NAMESPACE("facts.posix.networking");

namespace facter { namespace facts { namespace posix {

    networking_resolver::networking_resolver() :
        fact_resolver(
            "networking",
            {
                fact::hostname,
                fact::ipaddress,
                fact::ipaddress6,
                fact::netmask,
                fact::netmask6,
                fact::network,
                fact::network6,
                fact::macaddress,
                fact::interfaces,
                fact::domain,
                fact::fqdn,
                fact::dhcp_servers,
            },
            {
                string("^") + fact::ipaddress + "_",
                string("^") + fact::ipaddress6 + "_",
                string("^") + fact::mtu + "_",
                string("^") + fact::netmask + "_",
                string("^") + fact::netmask6 + "_",
                string("^") + fact::network + "_",
                string("^") + fact::network6 + "_",
                string("^") + fact::macaddress + "_",
            })
    {
    }

    void networking_resolver::resolve_facts(fact_map& facts)
    {
        resolve_hostname(facts);
        resolve_domain(facts);
        resolve_interface_facts(facts);
    }

    void networking_resolver::resolve_hostname(fact_map& facts)
    {
        int max = sysconf(_SC_HOST_NAME_MAX);
        vector<char> name(max);
        if (gethostname(name.data(), max) != 0) {
            LOG_WARNING("gethostname failed: %1% (%2%): %3% fact is unavailable.", strerror(errno), errno, fact::hostname);
            return;
        }

        // Use everything up to the first period
        string value = name.data();
        size_t pos = value.find('.');
        if (pos != string::npos) {
           value = value.substr(0, pos);
        }
        if (value.empty()) {
            return;
        }

        facts.add(fact::hostname, make_value<string_value>(move(value)));
    }

    void networking_resolver::resolve_domain(fact_map& facts)
    {
        auto hostname = facts.get<string_value>(fact::hostname, false);
        if (!hostname) {
            LOG_WARNING("%1% and %2% facts cannot be resolved without the %3% fact.", fact::fqdn, fact::domain, fact::hostname);
            return;
        }

        // Retrieve the FQDN by resolving the hostname
        string fqdn;
        string domain;
        scoped_addrinfo info(hostname->value());
        if (info.result() != 0 && info.result() != EAI_NONAME) {
            LOG_WARNING("getaddrinfo failed: %1% (%2%): defaulting to %3% for the %4% fact.", gai_strerror(info.result()), info.result(), fact::hostname, fact::fqdn);
        } else if (!info || info.result() == EAI_NONAME) {
            LOG_WARNING("hostname \"%1%\" could not be resolved: %2% fact may not be externally resolvable.", hostname->value(), fact::fqdn);
        } else {
            fqdn = static_cast<addrinfo*>(info)->ai_canonname;
        }

        // Default to the hostname for an empty FQDN
        if (fqdn.empty()) {
            fqdn = hostname->value();
        }

        // Set the domain name if the FQDN is prefixed with the hostname
        if (starts_with(fqdn, hostname->value() + ".")) {
            domain = fqdn.substr(hostname->value().length() + 1);
        }

        if (!domain.empty()) {
            facts.add(fact::domain, make_value<string_value>(move(domain)));
        }

        if (!fqdn.empty()) {
            facts.add(fact::fqdn, make_value<string_value>(move(fqdn)));
        }
    }

    string networking_resolver::address_to_string(sockaddr const* addr, sockaddr const* mask) const
    {
        if (!addr) {
            return {};
        }

        // Check for IPv4 and IPv6
        if (addr->sa_family == AF_INET) {
            in_addr ip = reinterpret_cast<sockaddr_in const*>(addr)->sin_addr;

            // Apply an IPv4 mask
            if (mask && mask->sa_family == addr->sa_family) {
                ip.s_addr &= reinterpret_cast<sockaddr_in const*>(mask)->sin_addr.s_addr;
            }

            char buffer[INET_ADDRSTRLEN] = {};
            inet_ntop(AF_INET, &ip, buffer, sizeof(buffer));
            return buffer;
        } else if (addr->sa_family == AF_INET6) {
            in6_addr ip = reinterpret_cast<sockaddr_in6 const*>(addr)->sin6_addr;

            // Apply an IPv6 mask
            if (mask && mask->sa_family == addr->sa_family) {
                auto mask_ptr = reinterpret_cast<sockaddr_in6 const*>(mask);
                for (size_t i = 0; i < 16; ++i) {
                    ip.s6_addr[i] &= mask_ptr->sin6_addr.s6_addr[i];
                }
            }

            char buffer[INET6_ADDRSTRLEN] = {};
            inet_ntop(AF_INET6, &ip, buffer, sizeof(buffer));
            return buffer;
        } else {
            auto link_addr = get_link_address_bytes(addr);
            if (link_addr) {
                return macaddress_to_string(reinterpret_cast<uint8_t const*>(link_addr));
            }
        }

        return {};
    }

    string networking_resolver::macaddress_to_string(uint8_t const* bytes)
    {
        if (!bytes) {
            return {};
        }

        // Ignore MAC address "0"
        bool nonzero = false;
        for (size_t i = 0; i < 6; ++i) {
            if (bytes[i] != 0) {
                nonzero = true;
                break;
            }
        }
        if (!nonzero) {
            return {};
        }

        return (format("%02x:%02x:%02x:%02x:%02x:%02x") %
                static_cast<int>(bytes[0]) % static_cast<int>(bytes[1]) %
                static_cast<int>(bytes[2]) % static_cast<int>(bytes[3]) %
                static_cast<int>(bytes[4]) % static_cast<int>(bytes[5])).str();
    }

}}}  // namespace facter::facts::posix
