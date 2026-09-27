#include "events/combatEvent.h"
#include "hooks/hooks.h"
#include "settings/INISettings.h"
#include "settings/JSONSettings.h"

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

SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		return false;
	}
	return true;
}

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Events::CombatEvent::GetSingleton()->RegisterListener();
		JSONSettings::Read();
		INISettings::Read();
		break;
	default:
		break;
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::InitInfo info;
	info.log = true;
	info.hook = true;
	info.trampoline = true;
	info.trampolineSize = 84u;
	
	SKSE::Init(a_skse, info);
	REX::INFO("Author: SeaSparrow"sv);
	SECTION_SEPARATOR;

	const auto ver = a_skse->RuntimeVersion();

#ifdef SKYRIM_GOG
	static constexpr std::array<REL::Version, 4> supported = 
	{
		SKSE::RUNTIME_SSE_1_6_1130,
		SKSE::RUNTIME_SSE_1_6_1170,
		SKSE::RUNTIME_SSE_1_6_1179,
		REL::Version(1, 6, 1179, 1) // no idea what this is still
	};
#else
	static constexpr std::array<REL::Version, 2> supported = 
	{
		SKSE::RUNTIME_SSE_1_7_104,
		SKSE::RUNTIME_SSE_1_7_99
	};	
#endif

	if ((ver < SKSE::RUNTIME_SSE_LATEST) && (!std::ranges::contains(supported, ver))) {
		REX::CRITICAL("Game Version: {}"sv, ver.string());
		REX::CRITICAL("Supported Versions:"sv);
		for (const auto& allowed : supported) {
			REX::CRITICAL("  - {}"sv, allowed.string());
		}
		REX::FAIL(
			fmt::format("You are using a version not supported by this plugin. Check the log at (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME)
		);
	}

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	Hooks::Install();
	return true;
}