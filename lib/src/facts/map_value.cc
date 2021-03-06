#include <facter/facts/map_value.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/logging/logging.hpp>
#include <facter/facterlib.h>
#include <rapidjson/document.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace rapidjson;
using namespace YAML;

LOG_DECLARE_NAMESPACE("facts.value.map");

namespace facter { namespace facts {

    void map_value::add(string&& name, unique_ptr<value>&& value)
    {
        if (!value) {
            LOG_DEBUG("null value cannot be added to map.");
            return;
        }

        _elements.emplace(move(name), move(value));
    }

    bool map_value::empty() const
    {
        return _elements.empty();
    }

    size_t map_value::size() const
    {
        return _elements.size();
    }

    void map_value::each(function<bool(string const&, value const*)> func) const
    {
        for (auto const& kvp : _elements) {
            if (!func(kvp.first, kvp.second.get())) {
                break;
            }
        }
    }

    value const* map_value::operator[](string const& name) const
    {
        auto it = _elements.find(name);
        if (it == _elements.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    void map_value::to_json(Allocator& allocator, rapidjson::Value& value) const
    {
        value.SetObject();

        for (auto const& kvp : _elements) {
            rapidjson::Value child;
            kvp.second->to_json(allocator, child);
            value.AddMember(kvp.first.c_str(), child, allocator);
        }
    }

    void map_value::notify(string const& name, enumeration_callbacks const* callbacks) const
    {
        if (!callbacks) {
            return;
        }

        if (callbacks->map_start) {
            callbacks->map_start(name.c_str());
        }

        // Call notify on each element in the array
        for (auto const& element : _elements) {
            element.second->notify(element.first, callbacks);
        }

        if (callbacks->map_end) {
            callbacks->map_end();
        }
    }

    ostream& map_value::write(ostream& os) const
    {
        // Write out the elements in the map
        os << "{";
        bool first = true;
        for (auto const& kvp : _elements) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << '"' << kvp.first << "\"=>";
            bool quote = dynamic_cast<string_value const*>(kvp.second.get());
            if (quote) {
                os << '"';
            }
            os << *kvp.second;
            if (quote) {
                os << '"';
            }
        }
        os << "}";
        return os;
    }

    Emitter& map_value::write(Emitter& emitter) const
    {
        emitter << BeginMap;
        for (auto const& kvp : _elements) {
            emitter << Key << kvp.first;
            emitter << YAML::Value << *kvp.second;
        }
        emitter << EndMap;
        return emitter;
    }

}}  // namespace facter::facts
