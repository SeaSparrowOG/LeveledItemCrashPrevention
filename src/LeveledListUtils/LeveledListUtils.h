#pragma once

namespace LeveledListUtils
{
	bool IsObjectList(const RE::TESBoundObject* form);
	std::string GetListEDID(RE::FormID id);

	bool CanReachList(const RE::TESLeveledList* from, const RE::TESLeveledList* to);
	bool IsAddIllegal(const RE::TESBoundObject* target, const RE::TESBoundObject* toAdd);

	void AuditLeveledLists();

	template <typename T>
	inline void Audit() {
		SECTION_SEPARATOR;
		logger::info("Auditing: {}"sv, RE::FormTypeToString(T::FORMTYPE));
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			logger::error("  - Failed to fetch the game's internal data handler. You likely will crash later."sv);
			return;
		}
		const auto& lists = dh->GetFormArray<T>();
		if (lists.empty()) {
			logger::error("  - Lists resolved as empty. This is definitely not intentional."sv);
			return;
		}

		std::vector<const RE::TESLeveledList*> circularLists;
		std::unordered_set<const RE::TESLeveledList*> audited;
		for (const auto* list : lists) {
			if (!list || list->entries.empty()) {
				continue;
			}

			audited.clear();
			bool circular = false;
			const auto& entries = list->entries;
			for (auto it = entries.begin(); !circular && it != entries.end(); ++it) {
				const auto* entryList = it->form ? skyrim_cast<const RE::TESLeveledList*>(it->form) : nullptr;
				if (!entryList || audited.contains(entryList)) {
					continue;
				}
				circular = CanReachList(entryList, list);
			}
			if (circular) {
				circularLists.emplace_back(list);
			}
		}
		if (circularLists.empty()) {
			logger::info("  - No circular lists found."sv);
			return;
		}

		logger::error("  - {} circular leveled lists found."sv, circularLists.size());
		for (const auto* list : circularLists) {
			const auto* bound = skyrim_cast<const RE::TESBoundObject*>(list);
			if (!bound) {
				continue;
			}
			const auto listEDID = GetListEDID(bound->GetFormID());
			logger::error("      > {}"sv, listEDID);
		}
	}
}