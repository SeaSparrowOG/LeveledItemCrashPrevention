#pragma once

namespace LeveledListUtils
{
	inline std::map<RE::FormID, std::unordered_set<RE::FormID>> g_listParents;

	void DebugCache();
	bool IsFormLeveledList(const RE::TESBoundObject* form);
	void RecordParents(const RE::TESBoundObject* boundList, std::unordered_set<RE::FormID>& parents);
	bool FindMalformedLeveledLists();
	bool IsAddIllegal(RE::TESBoundObject* target, RE::TESBoundObject* toAdd);

	template <typename T>
	void RefreshCache() {
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return;
		}
		const auto& forms = dh->GetFormArray<T>();
		if (forms.empty()) {
			return;
		}

		for (const auto* form : forms) {
			const auto* bound = form ? skyrim_cast<const RE::TESBoundObject*>(form) : nullptr;
			const auto* list = bound ? skyrim_cast<const RE::TESLeveledList*>(bound) : nullptr;
			if (!IsFormLeveledList(bound) || !list) {
				continue;
			}

			auto boundID = bound->GetFormID();
			std::unordered_set<RE::FormID> parents = { boundID };
			std::unordered_set<RE::FormID> seenInLevel;

			const auto& entries = list->entries;
			if (entries.empty()) {
				continue;
			}
			for (const auto& entry : entries) {
				const auto* containedForm = entry.form;
				const auto* subBound = containedForm ? skyrim_cast<const RE::TESBoundObject*>(containedForm) : nullptr;
				if (!subBound || !IsFormLeveledList(subBound)) {
					continue;
				}
				if (seenInLevel.contains(subBound->GetFormID())) {
					continue;
				}
				seenInLevel.insert(subBound->GetFormID());
				RecordParents(subBound, parents);
			}
		}
	}
}