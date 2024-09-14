#include "settings.h"

namespace Settings {
	bool ShouldRebuildINI(CSimpleIniA* a_ini) {
		const char* section = "General";
		const char* keys[] = {
			"bWarn",
			"bSanitize" };
		int sectionLength = sizeof(keys) / sizeof(keys[0]);
		std::list<CSimpleIniA::Entry> keyHolder;

		a_ini->GetAllKeys(section, keyHolder);
		if (std::size(keyHolder) != sectionLength) return true;
		for (auto* key : keys) {
			if (!a_ini->KeyExists(section, key)) return true;
		}
		return false;
	}

	bool Holder::ReadSettings() {
		std::filesystem::path f{ "./Data/SKSE/Plugins/LeveledListCrashPrevention.ini" };
		bool createEntries = false;
		if (!std::filesystem::exists(f)) {
			std::fstream createdINI;
			createdINI.open(f, std::ios::out);
			createdINI.close();
			createEntries = true;
		}

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(f.c_str());
		if (!createEntries) { createEntries = ShouldRebuildINI(&ini); }

		if (createEntries) {
			ini.Delete("General", NULL);
			ini.SetBoolValue("General", "bWarn", false,     ";If set to true, prints a warning in game when a leveled list would be overfilled. Default: False.");
			ini.SetBoolValue("General", "bSanitize", false, ";If set to true, checks leveled lists in all plugins to verify that they are under 255 entries. Default: False.");
			ini.SaveFile(f.c_str());
		}

		this->warn = ini.GetBoolValue("General", "bWarn", false);
		this->sanitize = ini.GetBoolValue("General", "bSanitize", false);
		return true;
	}

	bool Holder::ShouldWarn()
	{
		return this->warn;
	}

	bool Holder::ShouldSanitize()
	{
		return this->sanitize;
	}
}