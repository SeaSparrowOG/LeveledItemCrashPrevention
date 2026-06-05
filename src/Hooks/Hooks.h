#pragma once

#include "Settings/INI/INISettings.h"

namespace Hooks {
    template <typename T>
    struct AddFormHook {
        static inline void AddForm(T* leveledList,
            RE::TESBoundObject* toAdd,
            unsigned short level,
            unsigned long long count,
            RE::TESForm* a5)
        {
            if (leveledList->numEntries >= 255) {
                static auto tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks");
                std::string edid = "";
                if (tweaks) {
                    edid = clib_util::editorID::get_editorID(leveledList);
                }
                else {
                    const auto id = leveledList->formID;
                    edid = fmt::format("{:08X}", id);
                    auto& sourceFiles = leveledList->sourceFiles.array;
                    if (auto end = sourceFiles ? sourceFiles->back() : nullptr; end) {
                        edid += end->GetFilename();
                    }
                }

                logger::warn("Crash prevented due to bad insertion at {}"sv, edid);
                bool warn = Settings::INI::GetSetting<bool>(
                    Settings::INI::GENERAL_IN_GAME_WARNINGS.data()
                ).value_or(false);

                if (warn) {
                    const char* warning = "Leveled List Crash Prevented:\nCheck Documents/My Games/Skyrim Special Edition/SKSE/LeveledListCrashPrevention.log for more information.\nDo not ignore this error.";
                    RE::DebugMessageBox(warning);
                }
            }
            else {
                _addForm(leveledList, toAdd, level, count, a5);
            }
        }

        static inline REL::Relocation<decltype(&AddForm)> _addForm;

        static inline bool Install(REL::ID id, std::ptrdiff_t offset)
        {
            REL::Relocation<std::uintptr_t> target{ id, offset };
            if (!(REL::make_pattern<"E8">().match(target.address())))
            {
                logger::critical("  Failed to match pattern at {}."sv, id);
                return false;
            }

            auto& trampoline = SKSE::GetTrampoline();
            _addForm = trampoline.write_call<5>(target.address(), &AddForm);
        }
    };

    bool Install();
}