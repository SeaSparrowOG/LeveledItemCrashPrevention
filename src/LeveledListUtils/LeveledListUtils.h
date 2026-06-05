#pragma once

namespace LeveledListUtils
{
	std::map<RE::FormID, std::unordered_set<RE::FormID>> g_listParents;

	void RefreshCache();

	bool FindMalformedLeveledLists();

	bool IsAddIllegal(RE::TESBoundObject* target, RE::TESBoundObject* toAdd);
}