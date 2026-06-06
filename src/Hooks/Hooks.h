#pragma once

#include "LeveledListUtils/LeveledListUtils.h"
#include "Settings/INI/INISettings.h"

#undef AddForm

namespace Hooks {
    template <typename T>
    struct AddFormHook {

        static void GenerateWarning(T* target, RE::TESBoundObject* toAdd) {
            static auto tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks");
            std::string objEdid = "";
            std::string listEdid = "";
            if (tweaks) {
                listEdid = clib_util::editorID::get_editorID(target);
                objEdid = clib_util::editorID::get_editorID(toAdd);
            }
            else {
                objEdid = fmt::format("{:08X}", toAdd->GetLocalFormID());
                auto& sourceFiles = target->sourceFiles.array;
                if (auto end = sourceFiles ? sourceFiles->back() : nullptr; end) {
                    objEdid = std::string(end->GetFilename()) + "|" + objEdid;
                }
                listEdid = fmt::format("{:08X}", target->GetLocalFormID());
                if (auto end = sourceFiles ? sourceFiles->back() : nullptr; end) {
                    listEdid = std::string(end->GetFilename()) + "|" + listEdid;
                }
            }

            logger::warn("Bad insertion at: {} (tried to add {})"sv, listEdid, objEdid);
        }

        static inline void AddForm(T* leveledList,
            RE::TESBoundObject* toAdd,
            unsigned short level,
            unsigned long long count,
            RE::TESForm* a5)
        {
            if (leveledList->numEntries >= 255) {
                GenerateWarning(leveledList, toAdd);
                bool warn = Settings::INI::GetSetting<bool>(
                    Settings::INI::GENERAL_IN_GAME_WARNINGS.data()
                ).value_or(false);

                if (warn) {
                    const char* warning = "Leveled List Crash Prevented:\nCheck Documents/My Games/Skyrim Special Edition/SKSE/LeveledListCrashPrevention.log for more information.\nDo not ignore this error.";
                    RE::DebugMessageBox(warning);
                }
            }
            else if (LeveledListUtils::IsAddIllegal(skyrim_cast<RE::TESBoundObject*>(leveledList), toAdd)) {
                GenerateWarning(leveledList, toAdd);
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
                logger::critical("  Failed to match pattern at {}."sv, id.id());
                return false;
            }

            auto& trampoline = SKSE::GetTrampoline();
            _addForm = trampoline.write_call<5>(target.address(), &AddForm);
            return true;
        }
    };

    bool Install();
}