#include "multiversion.hpp"

btb::AutoRegisterVersion::AutoRegisterVersion() {
    btb::register_signature("Keyboard::feed", "48 89 5C 24 ?? 57 48 83 EC ?? 8B 05 ?? ?? ?? ?? 8B DA", 500, 0, 0);
}

static btb::AutoRegisterVersion autoRegisterInstance;