#include "multiversion.hpp"

btb::AutoRegisterVersion::AutoRegisterVersion() {
    // 1.21.2101.0
    btb::register_signature("Keyboard::feed", "48 83 EC ? 0F B6 C1 4C 8D 05", 21, 2101, 0);
}

static btb::AutoRegisterVersion autoRegisterInstance;