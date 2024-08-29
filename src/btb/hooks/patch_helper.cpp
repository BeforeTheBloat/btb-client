#include "patch_helper.hpp"

std::map<std::string, btb::SignatureInfo> btb::signatures;
std::map<std::string, btb::OffsetInfo> btb::offsets;

uintptr_t btb::get_raw_signature(std::string_view signature) {
    const auto parsed = hat::parse_signature(signature);
    const auto result = hat::find_pattern(parsed.value());

    if (!parsed.has_value() || !result.has_result()) {
        return 0u;
    }

    return reinterpret_cast<uintptr_t>(result.get());
}

uintptr_t btb::get_signature(std::string patch_name) {
    const auto it = signatures.find(patch_name);

    const winrt::Windows::ApplicationModel::Package package = winrt::Windows::ApplicationModel::Package::Current();
    auto [major, minor, build, revision] = package.Id().Version();

    if (it != signatures.end()) {
        const auto& info = it->second;
        if (info.minor == minor && info.build == build && info.revision == revision) {
            return get_raw_signature(info.signature);
        }
    }

    return 0u;
}

int32_t btb::get_offset(std::string patch_name) {
    const auto it = offsets.find(patch_name);

    const winrt::Windows::ApplicationModel::Package package = winrt::Windows::ApplicationModel::Package::Current();
    auto [major, minor, build, revision] = package.Id().Version();

    if (it != offsets.end()) {
        const auto& info = it->second;
        if (info.minor == minor && info.build == build && info.revision == revision) {
            return info.offset;
        }
    }

    return 0;
}


void btb::register_signature(std::string patch_name, std::string_view signature, int minor, int build, int revision) {
    signatures[patch_name] = { std::string(signature), minor, build, revision };
}

void btb::register_offset(std::string patch_name, int32_t offset, int minor, int build, int revision) {
    offsets[patch_name] = { offset, minor, build, revision };
}