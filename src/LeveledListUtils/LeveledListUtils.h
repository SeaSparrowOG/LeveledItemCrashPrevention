#pragma once

namespace LeveledListUtils
{
	bool IsObjectList(const RE::TESBoundObject* form);
	std::string GetListEDID(RE::FormID id);

	class LeveledListData
	{
	public:
		LeveledListData(const RE::FormID id) : _id{ id } {}

		void               AddParent(const RE::FormID id);
		void               AddChild(const RE::FormID id);
		void               RecordHierarchy();

		[[nodiscard]] bool             IsCircular() const;
		[[nodiscard]] bool             IsInComplete() const;
		[[nodiscard]] bool             HasParent(const RE::FormID id) const;
		[[nodiscard]] const RE::FormID GetID() const;

		std::vector<RE::FormID>& GetChildren();
		const std::vector<RE::FormID>& GetChildren() const;

	private:
		bool                           _incomplete = false;
		bool                           _isCircular = false;
		RE::FormID                     _id;
		std::vector<RE::FormID>        _children;
		std::unordered_set<RE::FormID> _parents;
	};

	class ListCache : public REX::Singleton<ListCache>
	{
	public:
		[[nodiscard]] bool             IsAddLegal(const RE::FormID targetID, const RE::FormID addID);
		[[nodiscard]] LeveledListData* GetData(const RE::FormID id);

		void RecordListData(const RE::FormID id, LeveledListData& data);

		template <typename T>
		[[nodiscard]] bool             Initialize()
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				logger::critical("  - Failed to retrieve the game's Data Handler!"sv);
				return false;
			}
			const auto& leveledItems = dh->GetFormArray<T>();
			const auto originalSize = _data.size();

			if (leveledItems.empty()) {
				logger::critical("  - Failed to find any leveled items in the game's files."sv);
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
			logger::info("  - Processed: {} lists, added data for {} of them."sv, leveledItems.size(), _data.size() - originalSize);

			return std::ranges::any_of(_data.begin(), _data.end(), [](const auto& element) {
				return element.second.IsCircular();
				});
		}
	private:
		bool _dynamicGuardOn = true;
		std::unordered_map<RE::FormID, LeveledListData> _data;
	};

	inline bool IsAddIllegal(RE::TESBoundObject* target, RE::TESBoundObject* toAdd) {
		static auto* cache = ListCache::GetSingleton();
		if (!cache) {
			logger::critical("Failed to get internal Leveled List Cache. State potentially corrupted."sv);
			SKSE::stl::report_and_fail("Failed to get internal Leveled List Cache. State potentially corrupted, aborting game to prevent data corruption."sv);
		}
		if (!target || !toAdd) {
			return true;
		}
		if (!IsObjectList(target) || !IsObjectList(toAdd)) {
			return true;
		}

		const auto targetID = target->GetFormID();
		const auto toAddID = toAdd->GetFormID();
		return cache->IsAddLegal(targetID, toAddID);
	}
}