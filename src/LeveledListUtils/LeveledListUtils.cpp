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

	void ListCache::Reload() {
		_dynamicGuardOn = false;
		_data.clear();

	}

	bool ListCache::Initialize() {
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			logger::critical("  - Failed to retrieve the game's Data Handler!"sv);
			return false;
		}
		const auto& leveledItems = dh->GetFormArray<RE::TESLevItem>();
		const auto& leveledSpells = dh->GetFormArray<RE::TESLevSpell>();
		const auto& leveledCharacters = dh->GetFormArray<RE::TESLevCharacter>();

		if (leveledItems.empty()) {
			logger::critical("  - Failed to find any leveled items in the game's files."sv);
			return false;
		}
		if (leveledSpells.empty()) {
			logger::critical("  - Failed to find any leveled spells in the game's files."sv);
			return false;
		}
		if (leveledCharacters.empty()) {
			logger::critical("  - Failed to find any leveled characters in the game's files."sv);
			return false;
		}

		for (const auto* ll : leveledItems) {
			const auto* obj = ll ? ll->As<RE::TESBoundObject>() : nullptr;
			if (!obj || !IsObjectList(obj) || _data.contains(obj->GetFormID())) {
				continue;
			}

			LeveledListData data(obj->GetFormID());
			data.RecordHierarchy();
			if (data.IsCircular() || data.IsInComplete()) {
				continue;
			}

			_data.emplace(obj->GetFormID(), std::move(data));
		}

		for (const auto* ll : leveledSpells) {
			const auto* obj = ll ? ll->As<RE::TESBoundObject>() : nullptr;
			if (!obj || !IsObjectList(obj) || _data.contains(obj->GetFormID())) {
				continue;
			}

			LeveledListData data(obj->GetFormID());
			data.RecordHierarchy();
			if (data.IsCircular() || data.IsInComplete()) {
				continue;
			}

			_data.emplace(obj->GetFormID(), std::move(data));
		}

		for (const auto* ll : leveledCharacters) {
			const auto* obj = ll ? ll->As<RE::TESBoundObject>() : nullptr;
			if (!obj || !IsObjectList(obj) || _data.contains(obj->GetFormID())) {
				continue;
			}

			LeveledListData data(obj->GetFormID());
			data.RecordHierarchy();
			if (data.IsCircular() || data.IsInComplete()) {
				continue;
			}

			_data.emplace(obj->GetFormID(), std::move(data));
		}
		logger::info("  - Finished processing {} objects, found {} leveled lists."sv, 
			leveledCharacters.size() + leveledSpells.size() + leveledItems.size(), _data.size());

#if 0
		logger::info("    - Created Entries:"sv);
		for (const auto& [id, data] : _data) {
			logger::info("      >{}:"sv, GetListEDID(id));
			const auto& children = data.GetChildren();
			for (const auto& child : children) {
				logger::info("        \\_{}"sv, GetListEDID(child));
			}
		}
#endif
		return false;
	}

	bool ListCache::IsAddLegal(const RE::FormID targetID, 
		const RE::FormID addID)
	{
		if (!_dynamicGuardOn) {
			return true;
		}
		const auto* add = RE::TESForm::LookupByID<RE::TESBoundObject>(addID);
		if (!add || !IsObjectList(add)) {
			return true;
		}

		auto targetIt = _data.find(targetID);
		auto addIt = _data.find(addID);
		if (targetIt == _data.end() || addIt == _data.end()) {
			// If either is a leveled list that is created at runtime...
			// yeah I don't support bad mods. Good luck Charlie.
			return true;
		}

		auto& targetData = targetIt->second;
		auto& addData = addIt->second;
		if (!targetData.HasParent(addID)) {
			return true;
		}
		if (targetData.HasParent(addID)) {
			return false;
		}

		const auto& subChildren = addData.GetChildren();
		bool anyOf = std::ranges::any_of(subChildren.begin(), subChildren.end(), [addData](const RE::FormID& childID) {
			return addData.HasParent(childID);
			});

		if (anyOf) {
			return false;
		}
		addData.AddParent(targetID);
		targetData.AddChild(addID);
		return true;
	}

	LeveledListData* ListCache::GetData(const RE::FormID id) {
		auto it = _data.find(id);
		if (it != _data.end()) {
			return &it->second;
		}
		return nullptr;
	}

	void ListCache::RecordListData(const RE::FormID id, LeveledListData& data) {
#ifndef NDEGBUF
		if (_data.contains(id)) {
			SKSE::stl::report_and_fail("oopsie"sv);
		}
#endif
		_data.emplace(id, std::move(data));
	}

	void LeveledListData::AddChild(const RE::FormID id) {
		for (const auto& child : _children) {
			if (child == id) {
				return;
			}
		}
		_children.emplace_back(id);
	}

	void LeveledListData::RecordHierarchy()
	{
		auto* listCache = ListCache::GetSingleton();
		if (!listCache) {
			return;
		}
		
		auto* form = RE::TESForm::LookupByID(_id);
		auto* bound = form ? skyrim_cast<RE::TESBoundObject*>(form) : nullptr;
		auto* list = bound ? form->As<RE::TESLeveledList>() : nullptr;
		if (!list) {
			_incomplete = true;
			return;
		}

		const auto& entries = list->entries;
		if (entries.empty()) {
			LOG_DEBUG("Empty entries exit"sv);
			return;
		}

		std::stack<LeveledListData> pendingData;
		std::unordered_set<RE::FormID> newParents = _parents;
		std::unordered_set<RE::FormID> seenInLevel;
		std::unordered_set<RE::FormID> children;

		newParents.insert(_id);

		for (const auto& entry : entries) {
			const auto* entryForm = entry.form;
			const auto* entryBound = entryForm ? skyrim_cast<const RE::TESBoundObject*>(entryForm) : nullptr;
			if (!entryBound || !IsObjectList(entryBound)) {
				continue;
			}

			const auto entryID = entryBound->GetFormID();
			if (seenInLevel.contains(entryID)) {
				continue;
			}
			seenInLevel.insert(entryID);

			if (newParents.contains(entryID)) {
				_isCircular = true;
				LOG_DEBUG("Extremely early exit"sv);
				continue;
			}

			auto* existingData = listCache->GetData(entryID);
			if (existingData) {
				for (const auto& parent : newParents) {
					existingData->AddParent(parent);
				}
				for (const auto& newChild : existingData->GetChildren()) {
					children.insert(newChild);
				}
				children.insert(entryID);

				_children.reserve(children.size());
				for (const auto& resultingChild : children) {
					_children.emplace_back(resultingChild);
				}
				continue;
			}

			LeveledListData childData(entryID);
			if (childData.IsInComplete()) {
				LOG_DEBUG("Incomplete exit"sv);
				continue;
			}
			for (const auto& parent : newParents) {
				childData.AddParent(parent);
			}
			childData.RecordHierarchy();
			if (childData.IsCircular()) {
				LOG_DEBUG("Circular exit"sv);
				continue;
			}

			const auto& newChildren = childData.GetChildren();
			for (auto it = newChildren.begin(); !_isCircular && it != newChildren.end(); ++it) {
				_isCircular = newParents.contains(*it);
				children.insert(*it);
			}
			children.insert(entryID);
			listCache->RecordListData(entryID, childData);
		}
		_children.reserve(children.size());
		for (const auto& resultingChild : children) {
			_children.emplace_back(resultingChild);
		}
	}

	std::vector<RE::FormID>& LeveledListData::GetChildren() { return _children; }
	const std::vector<RE::FormID>& LeveledListData::GetChildren() const { return _children; }
	const RE::FormID LeveledListData::GetID() const { return _id; }
	bool LeveledListData::IsCircular() const { return _isCircular; }
	bool LeveledListData::IsInComplete() const { return _incomplete; }
	bool LeveledListData::HasParent(const RE::FormID id) const { return _parents.contains(id); }
	void LeveledListData::AddParent(const RE::FormID id) { _parents.insert(id); }
}