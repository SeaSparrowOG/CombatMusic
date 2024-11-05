#pragma once

#include "utilities/utilities.h"

namespace Hooks {
	void Install();

	class CombatMusicCalls : public Utilities::Singleton::ISingleton<CombatMusicCalls>
	{
	public:
		bool Install();
		RE::BGSMusicType* GetCurrentCombatMusic();
		void SetCurrentCombatMusic(RE::BGSMusicType* a_combatMusic);

	private:
		RE::BGSMusicType* GetAppropriateMusic(RE::BGSMusicType* a_music);
		RE::BGSMusicType* ClearMusic();

		static RE::BGSMusicType* StartCombatMusic(int a1);
		static RE::BGSMusicType* EndCombatMusic(int a1);
		static RE::BGSMusicType* ClearLocation(int a1);

		RE::BGSMusicType* storedMusic;

		inline static REL::Relocation<decltype(&StartCombatMusic)> _startCombatMusic;
		inline static REL::Relocation<decltype(&EndCombatMusic)>   _endCombatMusic;
		inline static REL::Relocation<decltype(&ClearLocation)>    _clearLocation;
	};
}