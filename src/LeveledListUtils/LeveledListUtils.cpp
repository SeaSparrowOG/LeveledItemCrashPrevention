#include "LeveledListUtils.h"

#include "Settings/INI/INISettings.h"

namespace LeveledListUtils
{
	bool IsObjectList(const RE::TESBoundObject* form) {
		switch (form->GetFormType()) {
		case RE::FormType::LeveledItem:
		case RE::FormType::LeveledSpell:
		case RE::FormType::LeveledNPC:
			return true;
		default:
			return false;
		}
	}

	std::string GetListEDID(RE::FormID id) {
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

	bool CanReachList(const RE::TESLeveledList* from, 
		const RE::TESLeveledList* to)
	{
		assert(from && to);
		if (from == to) {
			return true;
		}
		const auto& fromEntries = from->entries;
		if (fromEntries.empty()) {
			return false;
		}

		std::stack<const RE::TESLeveledList*> listStack;
		std::unordered_set<const RE::TESLeveledList*> processed;

		for (const auto& entry : fromEntries) {
			const auto* entryForm = entry.form;
			const auto* entryList = entryForm ? skyrim_cast<const RE::TESLeveledList*>(entryForm) : nullptr;
			if (entryList == to) {
				return true;
			}
			if (entryList && !processed.contains(entryList)) {
				processed.insert(entryList);
				listStack.push(entryList);
			}
		}

		while (!listStack.empty()) {
			auto* topList = listStack.top();
			listStack.pop();
			if (topList == to) {
				return true;
			}

			const auto& entries = topList->entries;
			if (entries.empty()) {
				continue;
			}
			for (const auto& entry : entries) {
				const auto* entryForm = entry.form;
				const auto* entryList = entryForm ? skyrim_cast<const RE::TESLeveledList*>(entryForm) : nullptr;
				if (entryList == to) {
					return true;
				}
				if (entryList && !processed.contains(entryList)) {
					processed.insert(entryList);
					listStack.push(entryList);
				}
			}
		}
		return false;
	}

	bool IsAddIllegal(const RE::TESBoundObject* target,
		const RE::TESBoundObject* toAdd)
	{
		assert(target && toAdd && IsObjectList(target));
		if (!IsObjectList(toAdd)) {
			return false;
		}

		const RE::TESLeveledList* addList = skyrim_cast<const RE::TESLeveledList*>(toAdd);
		const RE::TESLeveledList* parentList = skyrim_cast<const RE::TESLeveledList*>(target);
		if (!addList) [[unlikely]] {
			return false;
		}

		return CanReachList(addList, parentList);
	}

	void AuditLeveledLists() {
		static bool doAudit = Settings::INI::GetSetting<bool>(
			Settings::INI::GENERAL_STATIC_CIRCULAR_DETECTION.data()
		).value_or(false);

		if (!doAudit) {
			return;
		}

		auto then = std::chrono::steady_clock::now();
		Audit<RE::TESLevItem>();
		Audit<RE::TESLevSpell>();
		Audit<RE::TESLevCharacter>();
		auto elapsed = std::chrono::steady_clock::now() - then;
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		logger::info("-----------------------------------------------"sv);
		logger::info("Finished in {}ms."sv, milliseconds);
	}
}