#pragma once

#include "LeveledListUtils/LeveledListUtils.h"
#include "Settings/INI/INISettings.h"

#undef AddForm

namespace Hooks {
    template <typename T>
    struct AddFormHook {
        static inline void AddForm(RE::TESLeveledList* leveledList,
            RE::TESBoundObject* listAsBound,
            unsigned short level,
            unsigned long long count,
            RE::TESForm* toAdd)
        {
            constexpr const char* warning = "Leveled List Crash Prevented:"\
                "\nCheck Documents / My Games / Skyrim Special Edition / SKSE / LeveledListCrashPrevention.log"\
                " for more information.\nDo not ignore this error.";

            static bool warn = Settings::INI::GetSetting<bool>(
                Settings::INI::GENERAL_IN_GAME_WARNINGS.data()
            ).value_or(false);

            static bool dynamicGuardOn = Settings::INI::GetSetting<bool>(
                Settings::INI::GENERAL_DYNAMIC_SANITIZATION.data()
            ).value_or(false);

            if (leveledList->numEntries >= 255) {
                const auto targetEDID = LeveledListUtils::GetListEDID(listAsBound->GetFormID());
                const auto addEDID = LeveledListUtils::GetListEDID(toAdd->GetFormID());
                logger::WARN("Prevent insertion of {} to {} because it would overflow."sv, addEDID, targetEDID);

                if (warn) {
                    RE::DebugMessageBox(warning);
                }
                return;
            }

            auto* bound = toAdd ? skyrim_cast<RE::TESBoundObject*>(toAdd) : nullptr;
            if (dynamicGuardOn && bound && LeveledListUtils::IsAddIllegal(listAsBound, bound)) {
                const auto targetEDID = LeveledListUtils::GetListEDID(listAsBound->GetFormID());
                const auto addEDID = LeveledListUtils::GetListEDID(toAdd->GetFormID());
                logger::WARN("Prevent insertion of {} to {} because it would create a circular leveled list."sv, addEDID, targetEDID);

                if (warn) {
                    RE::DebugMessageBox(warning);
                }
                return;
            }

            _addForm(leveledList, listAsBound, level, count, toAdd);
        }

        static inline REL::Relocation<decltype(&AddForm)> _addForm;

        static inline bool Install(REL::ID id, std::ptrdiff_t offset)
        {
            REL::Relocation<std::uintptr_t> target{ id, offset };
            if (!(REL::Pattern<"E8">().match(target.address()))) {
                logger::CRITICAL("    > Failed to match pattern at {}."sv, id.id());
                return false;
            }

            auto& trampoline = REL::GetTrampoline();
            _addForm = trampoline.write_call<5>(target.address(), &AddForm);
            return true;
        }
    };

    bool Install();
}