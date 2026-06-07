#include "Hooks/Hooks.h"
#include "LeveledListUtils/LeveledListUtils.h"
#include "Settings/INI/INISettings.h"

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
	{
		logger::info("Searching for pre-existing circular leveled lists..."sv);
		auto then = std::chrono::steady_clock::now();
		bool anyCircular = LeveledListUtils::ListCache::GetSingleton()->Initialize();
		auto elapsed = std::chrono::steady_clock::now() - then;
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		if (anyCircular) {
			logger::critical("Circular leveled lists detected within the loaded ESP/ESM/ESL files."sv);
			SKSE::stl::report_and_fail("Found circular leveled lists. These cannot be resolved automatically and must be resolved in xEdit."sv);
		}
		logger::info("Finished sanity check in {}ms."sv, milliseconds);
		SECTION_SEPARATOR;
	}
		break;
	case SKSE::MessagingInterface::kNewGame:
	case SKSE::MessagingInterface::kPostLoadGame:
	{
		logger::info("Updating internal Leveled List cache..."sv);
		auto then = std::chrono::steady_clock::now();

		bool anyCircular = false;
		if (anyCircular) {
			logger::warn("Circular leveled lists found within the save. Dynamic guard will not apply. This cannot be fixed."sv);
			break;
		}

		auto elapsed = std::chrono::steady_clock::now() - then;
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		logger::info("Finished updating entries. Update time: {}ms"sv, milliseconds);
		SECTION_SEPARATOR;
		break;
	}
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

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
#ifdef SKYRIM_AE
	if (ver < SKSE::RUNTIME_SSE_LATEST) {
#else
	if (ver < SKSE::RUNTIME_1_5_39) {
#endif
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
	}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
	SKSE::Init(a_skse);
	logger::info("Author: SeaSparrow"sv);
	SECTION_SEPARATOR;

#ifdef SKYRIM_AE
	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_LATEST) {
		return false;
	}
#endif

	logger::info("Performing startup tasks..."sv);

	if (!Settings::INI::Read()) {
		SKSE::stl::report_and_fail("Failed to load INI settings. Check the log (Documents/My Games/Skyrim Special Edition/ContainerDistributionFramework.log) for more information."sv);
	}
	SECTION_SEPARATOR;
	if (!Hooks::Install()) {
		SKSE::stl::report_and_fail("Failed to install hooks. Check the log (Documents/My Games/Skyrim Special Edition/ContainerDistributionFramework.log) for more information."sv);
	}
	SECTION_SEPARATOR;

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	return true;
}