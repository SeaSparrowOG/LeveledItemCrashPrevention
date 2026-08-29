#include "hooks.h"

#include "RE/Offset.h"

namespace Hooks {
    bool Install() {
        logger::INFO("Installing hooks..."sv);

        bool success = true;
        logger::INFO("  - Installing Leveled Items hook..."sv);
        success &= AddFormHook<RE::TESLevItem>::Install(RE::Offset::LevItems_AddForm, 0x56);
        logger::INFO("  - Installing Leveled Characters hook..."sv);
        success &= AddFormHook<RE::TESLevCharacter>::Install(RE::Offset::LevActors_AddForm, 0x56);
        logger::INFO("  - Installing Leveled Spells hook..."sv);
        success &= AddFormHook<RE::TESLevSpell>::Install(RE::Offset::LevSpells_AddForm, 0x56);
        logger::INFO("Finished installing hooks."sv);
        return success;
    }
}