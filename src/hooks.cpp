#include "hooks.h"

#include "settings.h"

namespace Hooks {
void DebugLeveledList(RE::TESLeveledList *a_list, RE::TESForm *a_problem) {
  auto *form = skyrim_cast<RE::TESBoundObject *>(a_list);
  _loggerInfo("Caught problematic insertion of form {} to leveled list {}.",
              _debugEDID(a_problem), _debugEDID(form));
  _loggerInfo("The form has not been inserted. For ease of review,\nhere are "
              "the current contents of the list:\n");
  int i = 1;

  for (auto &entry : a_list->entries) {
    if (!entry.form) {
      _loggerInfo("{}. Null form. This is a problem, do not ignore it.", i);
    } else {
      _loggerInfo("{}. {}", i, _debugEDID(entry.form));
    }
    ++i;
  }
  _loggerInfo("___________________________________________________");
  if (Settings::Holder::GetSingleton()->ShouldWarn()) {
    auto game = OFFSET_3("Skyrim Special Edition", "Skyrim Special Edition",
                         "SkyrimVR");
    auto message = std::format("Warning: Caught bad AddForm. Check <My Games/{}/SKSE/LeveledListCrashPrevention.log> for more information. This message is safe to ignore.", game);
    RE::DebugMessageBox(message.c_str());
  }
}

// Leveled Items
void ProtectLevItems::AddForm(RE::TESLeveledList *a_this,
                              RE::TESBoundObject *a_list,
                              unsigned short a_level,
                              unsigned long long a_count, RE::TESForm *a_form) {
  if (a_this->numEntries > 254) {
    DebugLeveledList(a_this, a_form);
  } else {
    _originalCall(a_this, a_list, a_level, a_count, a_form);
  }
}

bool ProtectLevItems::Install() {
  SKSE::AllocTrampoline(14);
  auto &trampoline = SKSE::GetTrampoline();

  REL::Relocation<std::uintptr_t> target{REL::ID(OFFSET(55401, 55965)), 0x56};
  if (!REL::make_pattern<OFFSET3("E8 55 ED 80 FF", "E8 75 1C 7C FF",
                                 "E8 05 39 7E FF")>()
           .match(target.address())) {
    _loggerInfo(
        "Failed to validate hook address for ProtectLevItems. Aborting load.");
    return false;
  }
  _loggerInfo("Installed leveled item patch.");
  _originalCall = trampoline.write_call<5>(target.address(), &AddForm);
  return true;
}

// Leveled Actors

void ProtectLeveledActors::AddForm(RE::TESLeveledList *a_this,
                                   RE::TESBoundObject *a_list,
                                   unsigned short a_level,
                                   unsigned long long a_count,
                                   RE::TESForm *a_form) {
  if (a_this->numEntries > 254) {
    DebugLeveledList(a_this, a_form);
  } else {
    _originalCall(a_this, a_list, a_level, a_count, a_form);
  }
}

bool ProtectLeveledActors::Install() {
  SKSE::AllocTrampoline(14);
  auto &trampoline = SKSE::GetTrampoline();

  REL::Relocation<std::uintptr_t> target{REL::ID(OFFSET(55387, 55954)), 0x56};
  if (!REL::make_pattern<OFFSET_3("E8 05 F5 80 FF", "E8 F5 24 7C FF",
                                  "E8 B5 40 7E FF")>()
           .match(target.address())) {
    _loggerInfo("Failed to validate hook address for ProtectLeveledActors. "
                "Aborting load.");
    return false;
  }

  _loggerInfo("Installed leveled actor patch.");
  _originalCall = trampoline.write_call<5>(target.address(), &AddForm);
  return true;
}

// Leveled Spells

void ProtectLeveledSpells::AddForm(RE::TESLeveledList *a_this,
                                   RE::TESBoundObject *a_list,
                                   unsigned short a_level,
                                   unsigned long long a_count,
                                   RE::TESForm *a_form) {
  if (a_this->numEntries > 254) {
    DebugLeveledList(a_this, a_form);
  } else {
    _originalCall(a_this, a_list, a_level, a_count, a_form);
  }
}

bool ProtectLeveledSpells::Install() {
  SKSE::AllocTrampoline(14);
  auto &trampoline = SKSE::GetTrampoline();

  REL::Relocation<std::uintptr_t> target{REL::ID(OFFSET(55415, 55976)), 0x56};
  if (!REL::make_pattern<OFFSET3("E8 25 E5 80 FF", "E8 65 13 7C FF",
                                 "E8 D5 30 7E FF")>()
           .match(target.address())) {
    _loggerInfo("Failed to validate hook address for ProtectLeveledSpells. "
                "Aborting load.");
    return false;
  }

  _loggerInfo("Installed leveled spell patch.");
  _originalCall = trampoline.write_call<5>(target.address(), &AddForm);
  return true;
}

// Install

bool Install() {
  if (!ProtectLevItems::Install() || !ProtectLeveledActors::Install() ||
      !ProtectLeveledSpells::Install()) {
    return false;
  }
  return true;
}
} // namespace Hooks