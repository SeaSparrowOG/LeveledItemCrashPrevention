#pragma once

namespace LeveledListUtils
{
	bool IsObjectList(const RE::TESBoundObject* form);
	std::string GetListEDID(RE::FormID id);

	bool CanReachList(const RE::TESLeveledList* from, const RE::TESLeveledList* to);
	bool IsAddIllegal(RE::TESBoundObject* target, RE::TESBoundObject* toAdd);
}