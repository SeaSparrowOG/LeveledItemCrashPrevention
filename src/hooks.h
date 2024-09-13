#pragma once

namespace Hooks {
    struct ProtectLevItems {
        static inline void AddForm(RE::TESLeveledList* a_this, RE::TESBoundObject* arg2,
            unsigned short arg3, unsigned long long arg4,
            RE::TESForm* arg5);

        static inline REL::Relocation<decltype(&ProtectLevItems::AddForm)> _originalCall;

        static inline bool Install();
    };

    struct ProtectLeveledActors {
        static inline void AddForm(RE::TESLeveledList* a_this, RE::TESBoundObject* arg2,
            unsigned short arg3, unsigned long long arg4,
            RE::TESForm* arg5);

        static inline REL::Relocation<decltype(&ProtectLevItems::AddForm)> _originalCall;

        static inline bool Install();
    };

    struct ProtectLeveledSpells {
        static inline void AddForm(RE::TESLeveledList* a_this, RE::TESBoundObject* arg2,
            unsigned short arg3, unsigned long long arg4,
            RE::TESForm* arg5);

        static inline REL::Relocation<decltype(&ProtectLevItems::AddForm)> _originalCall;

        static inline bool Install();
    };

    bool Install();
}