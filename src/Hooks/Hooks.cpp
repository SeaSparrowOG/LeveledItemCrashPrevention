#include "hooks.h"

#include "RE/Offset.h"

namespace Hooks {
    bool Install() {
        constexpr std::size_t allocSize = 14u * 3u;

        logger::info("Installing hooks..."sv);
        SKSE::AllocTrampoline(allocSize);

        bool success = true;
        logger::info("  - Installing Leveled Items hook..."sv);
        success &= AddFormHook<RE::TESLevItem>::Install(RE::Offset::LevItems_AddForm, 0x56);
        logger::info("  - Installing Leveled Characters hook..."sv);
        success &= AddFormHook<RE::TESLevCharacter>::Install(RE::Offset::LevActors_AddForm, 0x56);
        logger::info("  - Installing Leveled Spells hook..."sv);
        success &= AddFormHook<RE::TESLevSpell>::Install(RE::Offset::LevSpells_AddForm, 0x56);
        logger::info("Finished installing hooks."sv);
        return success;
    }
}