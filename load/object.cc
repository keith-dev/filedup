#include "object.hpp"

namespace mydoc {
Object::~Object() {
    if (Type::Object == type) delete value.object;
    if (Type::Array == type) delete value.array;
}
}