#include "hooks.h"

#include "RE/Offset.h"

namespace Hooks {
    bool Install() {
        constexpr std::size_t allocSize = 14u * 3u;
        SKSE::AllocTrampoline(allocSize);

        bool success = true;
        success &= AddFormHook<RE::TESLevItem>::Install(RE::Offset::LevItems_AddForm, 0x56);
        success &= AddFormHook<RE::TESLevCharacter>::Install(RE::Offset::LevActors_AddForm, 0x56);
        success &= AddFormHook<RE::TESLevSpell>::Install(RE::Offset::LevSpells_AddForm, 0x56);
        return success;
    }
}