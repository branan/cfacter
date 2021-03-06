#include <gmock/gmock.h>
#include <facter/facts/scalar_value.hpp>
#include <rapidjson/document.h>
#include <yaml-cpp/yaml.h>
#include <sstream>

using namespace std;
using namespace facter::facts;
using namespace rapidjson;
using namespace YAML;

TEST(facter_facts_string_value, move_constructor) {
    string s = "hello world";
    string_value value(move(s));
    ASSERT_EQ("", s);
    ASSERT_EQ("hello world", value.value());
}

TEST(facter_facts_string_value, copy_constructor) {
    string s = "hello world";
    string_value value(s);
    ASSERT_EQ("hello world", s);
    ASSERT_EQ("hello world", value.value());
}

TEST(facter_facts_string_value, to_json) {
    string_value value("hello world");

    rapidjson::Value json_value;
    MemoryPoolAllocator<> allocator;
    value.to_json(allocator, json_value);
    ASSERT_TRUE(json_value.IsString());
    ASSERT_EQ("hello world", string(json_value.GetString()));
}

TEST(facter_facts_string_value, insertion_operator) {
    string_value value("hello world");

    ostringstream stream;
    stream << value;
    ASSERT_EQ("hello world", stream.str());
}

TEST(facter_facts_string_value, yaml_insertion_operator) {
    string_value value("hello world");

    Emitter emitter;
    emitter << value;
    ASSERT_EQ("\"hello world\"", string(emitter.c_str()));
}
