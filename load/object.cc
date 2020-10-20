#include "object.hpp"

namespace mydoc {
Object::~Object() {
    switch (type) {
    case Type::Object:
        for (auto p = value.objects->rbegin(); p != value.objects->rend(); ++p)
            delete *p;
        delete value.objects;
        break;
    case Type::Array:
        for (auto p = value.array->rbegin(); p != value.array->rend(); ++p)
            delete *p;
        delete value.array;
        break;
    case Type::String:
        delete value.str;
        break;
    case Type::None:    // fallthru
    case Type::Boolean: // fallthru
    case Type::Real:    // fallthru
    case Type::Integer: // fallthru
        ;
    }
}
}