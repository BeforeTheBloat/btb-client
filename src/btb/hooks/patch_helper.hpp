#pragma once

#include "../../pch.hpp"

namespace btb {
    struct SignatureInfo {
        std::string signature;
        int minor;
        int build;
        int revision;
    };

    struct OffsetInfo {
        int32_t offset;
        int minor;
        int build;
        int revision;
    };

    extern std::map<std::string, SignatureInfo> signatures;
    extern std::map<std::string, OffsetInfo> offsets;

    uintptr_t get_signature(std::string patch_name);
    int32_t get_offset(std::string patch_name);
    uintptr_t get_raw_signature(std::string_view signature);
    void register_signature(std::string patch_name, std::string_view signature, int minor, int build, int revision);
    void register_offset(std::string patch_name, int32_t offset, int minor, int build, int revision);
};