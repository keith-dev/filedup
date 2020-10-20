#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace mydoc {
class Object;
class Objects;
class Array;

class Objects : public std::vector<Object*> {};
class Array : public std::vector<Object*> {};

class Object {
    // When I grow up, I want to be a variant.
    enum class Type { None, Object, Array, Boolean, String, Real, Integer };
    union Value {
        Value(Objects* objects) : objects(objects) {}
        Value(Array* array) : array(array) {}
        Value(bool boolean) : boolean(boolean) {}
        Value(std::string str) : str(new std::string(std::move(str))) {}
        Value(double real) : real(real) {}
        Value(int integer) : integer(integer) {}

        Value(const Value &) = delete;
        Value& operator=(const Value &) = delete;

        Objects* objects;
        Array* array;
        bool boolean;
        std::string* str;
        double real;
        int integer;
    };

    Type type;
    Value value;

protected:
    Object(Objects* objects): type(Type::Object),   value(objects) {}
    Object(Array* array)    : type(Type::Array),    value(array) {}
    Object(bool boolean)    : type(Type::Boolean),  value(boolean) {}
    Object(std::string str) : type(Type::String),   value(std::move(str)) {}
    Object(double real)     : type(Type::Real),     value(real) {}
    Object(int integer)     : type(Type::Integer),  value(integer) {}

public:
    Object() : type(Type::None), value(0) {}
    ~Object();

    Object(const Object &) = delete;
    Object& operator=(const Object &) = delete;

    static Object* createObject()               { return new Object(new Objects{}); }
    static Object* createArray()                { return new Object(new Array); }
    static Object* createBool(bool boolean)     { return new Object(boolean); }
    static Object* createString(std::string str){ return new Object(std::move(str)); }
    static Object* createReal(double real)      { return new Object(real); }
    static Object* createInteger(int integer)   { return new Object(integer); }

    std::string getTypeAsString() const {
        switch (type) {
        case Type::None:    return "None";
        case Type::Object:  return "Object";
        case Type::Array:   return "Array";
        case Type::Boolean: return "Boolean";
        case Type::String:  return "String";
        case Type::Real:    return "Real";
        case Type::Integer: return "Integer";
        }
        return "Unknown";
    }

    Objects& object() {
        if (Type::Object != type) throw std::runtime_error("getObject has type" + getTypeAsString());
        return *value.objects;
    }

    Array& array() {
        if (Type::Array != type) throw std::runtime_error("getArray has type" + getTypeAsString());
        return *value.array;
    }

    bool& boolean() {
        if (Type::Boolean != type) throw std::runtime_error("getBoolean has type" + getTypeAsString());
        return value.boolean;
    }

    double& real() {
        if (Type::Real != type) throw std::runtime_error("getReal has type" + getTypeAsString());
        return value.real;
    }

    int& integer() {
        if (Type::Integer != type) throw std::runtime_error("getInteger has type" + getTypeAsString());
        return value.integer;
    }

    const Objects& object() const {
        if (Type::Object != type) throw std::runtime_error("getObject has type" + getTypeAsString());
        return *value.objects;
    }

    const Array& array() const {
        if (Type::Array != type) throw std::runtime_error("getArray has type" + getTypeAsString());
        return *value.array;
    }

    bool boolean() const {
        if (Type::Boolean != type) throw std::runtime_error("getBoolean has type" + getTypeAsString());
        return value.boolean;
    }

    double real() const {
        if (Type::Real != type) throw std::runtime_error("getReal has type" + getTypeAsString());
        return value.real;
    }

    int integer() const {
        if (Type::Integer != type) throw std::runtime_error("getInteger has type" + getTypeAsString());
        return value.integer;
    }
};

}