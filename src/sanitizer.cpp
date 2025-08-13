#include "sanitizer.h"

namespace Sanitizer {
void Sanitize() {
  _loggerInfo("Sanitizing  leveled lists if needed.");

  bool foundBadLL = false;
  auto &formArray = RE::TESDataHandler::GetSingleton()->GetFormArray(
      RE::FormType::LeveledItem);
  for (auto *form : formArray) {
    auto *ll = skyrim_cast<RE::TESLeveledList *>(form);
    if (!ll)
      continue;
    if (!(ll->numEntries == 0 || ll->numEntries == 255))
      continue;

    size_t i = 0;
    for (auto &llEntry : ll->entries) {
      i++;
    }
    if (i <= 255)
      continue;

    _loggerInfo("    Leveled list {} has {} entries.", _debugEDID(form), i);
    foundBadLL = true;
  }

  if (foundBadLL) {
    auto game = OFFSET_3("Skyrim Special Edition", "Skyrim Special Edition",
                         "SkyrimVR");
    auto message = std::format("Warning: At least 1 leveled list has over 255 entries in the plugin record. Check the log at <Documents/My Games/{}/SKSE/LeveledListCrashPrevention.log> for more information. This message is safe to ignore.", game);
	SKSE::stl::report_and_fail(message.c_str());
  }
}
} // namespace Sanitizer