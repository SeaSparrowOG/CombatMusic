#include "settings/INISettings.h"

#include "events/combatEvent.h"
#include <SimpleIni.h>

namespace INISettings
{
	void Read() {
		//Adapted from Exit-9B (Parapets)
		::CSimpleIniA ini{};
		ini.SetUnicode();
		ini.LoadFile(fmt::format(R"(.\Data\SKSE\Plugins\{}.ini)", Plugin::NAME).c_str());

		const auto combatMusicFixTimeSpanSeconds = ini.GetLongValue("General", "iCombatMusicFixWait", 10);
		Events::CombatEvent::GetSingleton()->SetWaitTime(combatMusicFixTimeSpanSeconds);
	}
}