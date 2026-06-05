#include "hooks.h"

namespace Hooks {
    void ProtectLevItems::AddForm(RE::TESLeveledList* a_this, RE::TESBoundObject* a_list, unsigned short a_level, unsigned long long a_count, RE::TESForm* a_form) {
        (void)a_this;
        (void)a_list;
        (void)a_level;
        (void)a_count;
        (void)a_form;
    }

    bool ProtectLevItems::Install() {
        return true;
    }

    void ProtectLeveledActors::AddForm(RE::TESLeveledList* a_this, RE::TESBoundObject* a_list, unsigned short a_level, unsigned long long a_count, RE::TESForm* a_form) {
        (void)a_this;
        (void)a_list;
        (void)a_level;
        (void)a_count;
        (void)a_form;
    }

    bool ProtectLeveledActors::Install() {
        return true;
    }

    void ProtectLeveledSpells::AddForm(RE::TESLeveledList* a_this, RE::TESBoundObject* a_list, unsigned short a_level, unsigned long long a_count, RE::TESForm* a_form) {
        (void)a_this;
        (void)a_list;
        (void)a_level;
        (void)a_count;
        (void)a_form;
    }

    bool ProtectLeveledSpells::Install() {
        return true;
    }

    bool Install() {
        return true;
    }
}