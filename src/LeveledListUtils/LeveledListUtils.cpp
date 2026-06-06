#include "LeveledListUtils.h"

#include "Settings/INI/INISettings.h"

namespace LeveledListUtils
{
	std::string GetLeveledListEDID(RE::FormID id) {
		const auto* form = RE::TESForm::LookupByID(id);
		const auto* ll = form ? skyrim_cast<const RE::TESLeveledList*>(form) : nullptr;
		if (!ll) {
			return "";
		}

		static auto tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks");
		std::string listEdid = "";
		if (tweaks) {
			listEdid = clib_util::editorID::get_editorID(form);
		}
		else {
			const auto* sourceFiles = form->sourceFiles.array;
			listEdid = fmt::format("{:08X}", form->GetLocalFormID());
			if (auto back = sourceFiles ? sourceFiles->back() : nullptr; back) {
				listEdid = std::string(back->GetFilename()) + "|" + listEdid;
			}
		}
		return listEdid;
	}

	void DebugCache() {
		SECTION_SEPARATOR;
		LOG_DEBUG("List of lists and their parents:"sv);
		for (const auto& [listID, parentIDs] : g_listParents) {
			const auto* listForm = RE::TESForm::LookupByID<RE::TESBoundObject>(listID);
			if (!listForm) {
				LOG_DEBUG("  - Failed to resolve: {:8X}."sv, listID);
				continue;
			}
			const auto* list = skyrim_cast<const RE::TESLeveledList*>(listForm);
			if (!list) {
				LOG_DEBUG("  - Not a leveled list: {}"sv, clib_util::editorID::get_editorID(listForm));
				continue;
			}
			LOG_DEBUG("  - {}"sv, clib_util::editorID::get_editorID(listForm));

			for (const auto& parentID : parentIDs) {
				const auto* parentForm = RE::TESForm::LookupByID<RE::TESBoundObject>(parentID);
				if (!parentForm) {
					LOG_DEBUG("    >Failed to resolve: {:8X}."sv, parentID);
					continue;
				}
				const auto* parentList = skyrim_cast<const RE::TESLeveledList*>(parentForm);
				if (!parentList) {
					LOG_DEBUG("    >Not a leveled list: {}"sv, clib_util::editorID::get_editorID(parentForm));
					continue;
				}
				LOG_DEBUG("    >{}"sv, clib_util::editorID::get_editorID(parentForm));
			}
		}
		LOG_DEBUG("End."sv);
	}

	bool IsFormLeveledList(const RE::TESBoundObject* form) {
		switch (form->GetFormType()) {
		case RE::FormType::LeveledItem:
		case RE::FormType::LeveledNPC:
		case RE::FormType::LeveledSpell:
			return true;
		default:
			return false;
		}
	}

	void RecordParents(const RE::TESBoundObject* boundList, std::unordered_set<RE::FormID>& parents) {
		auto boundID = boundList->GetFormID();
		auto& currentParents = g_listParents[boundID];
		auto* asList = boundList->As<RE::TESLeveledList>();
		if (!asList) {
			return;
		}
		for (const auto& parent : parents) {
			currentParents.insert(parent);
		}

		parents.insert(boundID);
		std::unordered_set<RE::FormID> seenInLevel;
		const auto& entries = asList->entries;
		for (const auto& entry : entries) {
			const auto* form = entry.form;
			const auto* asBound = form ? skyrim_cast<const RE::TESBoundObject*>(form) : nullptr;
			if (!asBound || !IsFormLeveledList(asBound)) {
				continue;
			}

			auto subBoundID = asBound->GetFormID();
			if (seenInLevel.contains(subBoundID)) {
				continue;
			}

			seenInLevel.insert(subBoundID);
			RecordParents(asBound, parents);
			parents.erase(subBoundID);
		}
		parents.erase(boundID);
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

	bool ContainsCircularList(const RE::TESBoundObject* obj,
		std::unordered_set<RE::FormID>& parents)
	{
		if (!obj || !IsFormLeveledList(obj)) {
			return false;
		}

		auto listID = obj->GetFormID();
		if (parents.contains(listID)) {
			logger::critical("Found circular leveled list. Path:"sv);
			logger::critical("{}"sv, GetLeveledListEDID(listID));
			return true;
		}
		const auto* list = obj->As<RE::TESLeveledList>();
		if (!list) {
			return false;
		}

		parents.insert(listID);
		const auto& entries = list->entries;

		for (const auto& entry : entries) {
			const auto* form = entry.form;
			const auto* bound = form ? skyrim_cast<const RE::TESBoundObject*>(form) : nullptr;
			if (!bound || !IsFormLeveledList(bound)) {
				continue;
			}

			const auto boundID = bound->GetFormID();
			if (parents.contains(boundID)) {
				logger::critical("Found circular leveled list. Path:"sv);
				logger::critical("{}"sv, GetLeveledListEDID(boundID));
				return true;
			}
			if (ContainsCircularList(bound, parents)) {
				logger::critical("  \\_{}"sv, GetLeveledListEDID(boundID));
				return true;
			}
		}
		parents.erase(listID);
		return false;
	}
}