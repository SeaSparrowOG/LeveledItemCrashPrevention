#include "LeveledListUtils.h"

#include "Settings/INI/INISettings.h"

namespace LeveledListUtils
{
	static bool IsFormLeveledList(const RE::TESBoundObject* form) {
		switch (form->GetFormType()) {
		case RE::FormType::LeveledItem:
		case RE::FormType::LeveledNPC:
		case RE::FormType::LeveledSpell:
			return true;
		default:
			return false;
		}
	}

	static void RecordParents(const RE::TESBoundObject* boundList, std::unordered_set<RE::FormID>& parents) {

	}

	void RefreshCache() {
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return;
		}
		const auto& leveledLists = dh->GetFormArray<RE::TESLeveledList>();
		if (leveledLists.empty()) {
			logger::warn("Leveled lists resolved empty within the data handler - potentially catastrophical."sv);
			return;
		}

		g_listParents.clear();

		for (const auto* list : leveledLists) {
			if (!list) {
				continue;
			}
			const auto* boundList = skyrim_cast<const RE::TESBoundObject*>(list);
			if (!boundList) {
				continue;
			}

			std::unordered_set<RE::FormID> parents = { boundList->GetFormID() };
			auto listForms = list->GetContainedForms();
			for (const auto* containedForm : listForms) {
				const auto* bound = skyrim_cast<const RE::TESBoundObject*>(containedForm);
				if (!bound || !IsFormLeveledList(bound)) {
					continue;
				}
				RecordParents(bound, parents);
			}
		}
	}

	bool FindMalformedLeveledLists() {
		return false;
	}

	bool IsAddIllegal(RE::TESBoundObject* target, RE::TESBoundObject* toAdd) {
		if (!target ||
			!Settings::INI::GetSetting<bool>(Settings::INI::GENERAL_DYNAMIC_SANITIZATION.data()).
				value_or(false))
		{
			return false;
		}
		if (!IsFormLeveledList(toAdd) || !IsFormLeveledList(target)) {
			return false;
		}

		auto targetID = target->GetFormID();
		auto listToAdd = toAdd->GetFormID();
		auto it = g_listParents.find(targetID);
		if (it == g_listParents.end()) {
			return false;
		}

		const auto& parents = it->second;
		if (parents.contains(listToAdd)) {
			return true;
		}

		auto toAddParents = g_listParents.find(listToAdd);
		if (toAddParents != g_listParents.end()) {
			toAddParents->second.insert(targetID);
		}
		return false;
	}
}