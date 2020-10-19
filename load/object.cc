#include "object.hpp"

namespace mydoc {
Object::~Object() {
    if (Type::Object == type) {
        for (auto p = value.objects->rbegin(); p != value.objects->rend(); ++p)
            delete *p;
        delete value.objects;
    }

    if (Type::Array == type) {
        delete value.array;
    }
}
}