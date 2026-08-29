#include "Hooks/Hooks.h"
#include "LeveledListUtils/LeveledListUtils.h"
#include "Settings/INI/INISettings.h"

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
		logger::INFO("Finished startup! Enjoy your game!"sv);
		[[fallthrough]];
	case SKSE::MessagingInterface::kNewGame:
	case SKSE::MessagingInterface::kPostLoadGame:
		LeveledListUtils::AuditLeveledLists();
		break;
	default:
		break;
	}
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
	{
		SKSE::PluginVersionData v{};

		v.PluginVersion(Plugin::VERSION);
		v.PluginName(Plugin::NAME);
		v.AuthorName("SeaSparrow"sv);
		v.UsesAddressLibrary();
		v.UsesUpdatedStructs();

		return v;
	}();
#endif

SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		logger::CRITICAL("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
#ifdef SKYRIM_AE
	if (ver < SKSE::RUNTIME_SSE_LATEST) {
#else
	if (ver < SKSE::RUNTIME_1_5_39) {
#endif
		logger::CRITICAL(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
	}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface * a_skse)
{
	constexpr std::size_t allocSize = 14u * 3u;

	SKSE::InitInfo info;
	info.hook = true;
	info.log = true;
	info.logLevel = REX::ELogLevel::Trace;
	info.logName = Plugin::NAME.data();
	info.trampoline = true;
	info.trampolineSize = allocSize;

	SKSE::Init(a_skse, info);
	logger::INFO("Author: SeaSparrow"sv);
	SECTION_SEPARATOR;

#ifdef SKYRIM_AE
	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_LATEST) {
		return false;
	}
#endif

	logger::INFO("Performing startup tasks..."sv);

	if (!Settings::INI::Read()) {
		REX::FAIL("Failed to load INI settings. Check the log (Documents/My Games/Skyrim Special Edition/ContainerDistributionFramework.log) for more information."sv);
	}
	SECTION_SEPARATOR;
	if (!Hooks::Install()) {
		REX::FAIL("Failed to install hooks. Check the log (Documents/My Games/Skyrim Special Edition/ContainerDistributionFramework.log) for more information."sv);
	}
	SECTION_SEPARATOR;

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	return true;
}