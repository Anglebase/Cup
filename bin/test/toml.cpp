#include "toml/trait.h"
#include "log/log.h"

const std::string_view toml_str = R"(
[data]
name = "Tom"
age = 25
children = [ "Alice", "Bob", "Charlie" ]

[person]
name = "Jane"
age = 30
married = true
children = [ "Eve", "Frank" ]
)";

struct Data
{
    data::String name;
    data::Integer age;
    std::optional<data::Boolean> married;
    data::Array<data::String> children;
};

template <>
struct data::Deserializer<Data>
{
    static inline Data deserialize(const toml::node &v, const std::string &key = "")
    {
        if (!v.is_table())
            throw std::runtime_error("toml node is not a table");
        auto table = *v.as_table();
        Data result;
        require(table, "name", result.name, key);
        require(table, "age", result.age, key);
        options(table, "married", result.married, key);
        require(table, "children", result.children, key);
        return result;
    }
};

std::ostream &operator<<(std::ostream &os, const Data &data)
{
    os << "name: " << data.name << std::endl;
    os << "age: " << data.age << std::endl;
    if (data.married)
        os << "married: " << *data.married << std::endl;
    os << "children: " << std::endl;
    for (const auto &child : data.children)
        os << "  - " << child << std::endl;
    return os;
}

struct Person
{
    data::String name;
    data::Integer age;
    std::optional<data::Boolean> married;
    data::Array<data::String> children;
};

template <>
struct data::Deserializer<Person>
{
    static inline Person deserialize(const toml::node &v, const std::string &key = "")
    {
        if (!v.is_table())
            throw std::runtime_error("toml node is not a table");
        auto table = *v.as_table();
        Person result;
        require(table, "name", result.name, key);
        require(table, "age", result.age, key);
        options(table, "married", result.married, key);
        require(table, "children", result.children, key);
        return result;
    }
};

std::ostream &operator<<(std::ostream &os, const Person &person)
{
    os << "name: " << person.name << std::endl;
    os << "age: " << person.age << std::endl;
    if (person.married)
        os << "married: " << *person.married << std::endl;
    os << "children: " << std::endl;
    for (const auto &child : person.children)
        os << "  - " << child << std::endl;
    return os;
}

struct Config
{
    Data data;
    Person person;
};

template <>
struct data::Deserializer<Config>
{
    static inline Config deserialize(const toml::node &v, const std::string &key = "")
    {
        if (!v.is_table())
            throw std::runtime_error("toml node is not a table");
        auto table = *v.as_table();
        Config result;
        require(table, "data", result.data, key);
        require(table, "person", result.person, key);
        return result;
    }
};

std::ostream &operator<<(std::ostream &os, const Config &config)
{
    os << "data:" << std::endl;
    os << config.data << std::endl;
    os << "person:" << std::endl;
    os << config.person << std::endl;
    return os;
}

int main()
{
    try
    {
        auto data = toml::parse(toml_str);
        auto d = data::Deserializer<Config>::deserialize(data);
        LOG_INFO(d);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
    }
    return 0;
}