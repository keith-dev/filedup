#pragma once

//#include <stdexcept>
//#include <string>
#include <vector>

namespace mydoc {
class Object;
class sArray;

class Object {
    // When I grow up, I want to be a variant.
    enum class Type { Object, Array, Boolean, Real, Integer };
    union Value {
		Value() : object(nullptr) {}
        Value(Object* object) : object(object) {}
        Value(sArray* array) : array(array) {}
        Value(bool boolean) : boolean(boolean) {}
        Value(double real) : real(real) {}
        Value(int integer) : integer(integer) {}

        Object* object;
        sArray* array;
        bool boolean;
        double real;
        int integer;
    };

    Type type;
    Value value;

public:
    Object() : type(Type::Object) {}
    Object(Object* object) : type(Type::Object), value(object) {}
    Object(sArray* array) : type(Type::Array), value(array) {}
    Object(bool boolean) : type(Type::Boolean), value(boolean) {}
    Object(double real) : type(Type::Real), value(real) {}
    Object(int integer) : type(Type::Integer), value(integer) {}

    ~Object();

/*
    std::string getTypeAsString() const {
        switch (type) {
        case Type::Object: return "Object";
        case Type::Array: return "Array";
        case Type::Boolean: return "Boolean";
        case Type::Real: return "Real";
        case Type::Integer: return "Integer";
        }
    }

    Object*& object() {
        if (Type::Object != type) throw std::runtime_error("getObject has type" + getTypeAsString());
        return value.object;
    }

    sArray*& array() {
        if (Type::Array != type) throw std::runtime_error("getArray has type" + getTypeAsString());
        return value.array;
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

    Object* object() const {
        if (Type::Object != type) throw std::runtime_error("getObject has type" + getTypeAsString());
        return value.object;
    }

    sArray* array() const {
        if (Type::Array != type) throw std::runtime_error("getArray has type" + getTypeAsString());
        return value.array;
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
 */
};

class sArray : public std::vector<Object*> {};

}
