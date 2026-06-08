#pragma once

#include "LeveledListUtils/LeveledListUtils.h"
#include "Settings/INI/INISettings.h"

#undef AddForm

namespace Hooks {
    template <typename T>
    struct AddFormHook {
        static inline void AddForm(T* leveledList,
            RE::TESBoundObject* toAdd,
            unsigned short level,
            unsigned long long count,
            RE::TESForm* a5)
        {
            constexpr const char* warning = "Leveled List Crash Prevented:"\
                "\nCheck Documents / My Games / Skyrim Special Edition / SKSE / LeveledListCrashPrevention.log"\
                "for more information.\nDo not ignore this error.";

            static bool warn = Settings::INI::GetSetting<bool>(
                Settings::INI::GENERAL_IN_GAME_WARNINGS.data()
            ).value_or(false);

            static bool dynamicGuardOn = Settings::INI::GetSetting<bool>(
                Settings::INI::GENERAL_DYNAMIC_SANITIZATION.data()
            ).value_or(false);

            if (leveledList->numEntries >= 255) {
                const auto* bound = skyrim_cast<const RE::TESBoundObject*>(leveledList);
                const auto targetEDID = bound ? LeveledListUtils::GetListEDID(bound->GetFormID()) : "NULL";
                const auto addEDID = LeveledListUtils::GetListEDID(toAdd->GetFormID());
                logger::warn("Prevent insertion of {} to {} because it would overflow."sv, addEDID, targetEDID);

                if (warn) {
                    RE::DebugMessageBox(warning);
                }
                return;
            }

            const auto* bound = skyrim_cast<const RE::TESBoundObject*>(leveledList);
            if (dynamicGuardOn && bound && LeveledListUtils::IsAddIllegal(bound, toAdd)) {
                const auto targetEDID = bound ? LeveledListUtils::GetListEDID(bound->GetFormID()) : "NULL";
                const auto addEDID = LeveledListUtils::GetListEDID(toAdd->GetFormID());
                logger::warn("Prevent insertion of {} to {} because it would create a circular leveled list."sv, addEDID, targetEDID);

                if (warn) {
                    RE::DebugMessageBox(warning);
                }
                return;
            }

            _addForm(leveledList, toAdd, level, count, a5);
        }

        static inline REL::Relocation<decltype(&AddForm)> _addForm;

        static inline bool Install(REL::ID id, std::ptrdiff_t offset)
        {
            REL::Relocation<std::uintptr_t> target{ id, offset };
            if (!(REL::make_pattern<"E8">().match(target.address()))) {
                logger::critical("    > Failed to match pattern at {}."sv, id.id());
                return false;
            }

            auto& trampoline = SKSE::GetTrampoline();
            _addForm = trampoline.write_call<5>(target.address(), &AddForm);
            return true;
        }
    };

    bool Install();
}