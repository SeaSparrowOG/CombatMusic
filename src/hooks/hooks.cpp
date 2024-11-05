#include "Hooks/hooks.h"

namespace Hooks {
	void Install()
	{
		CombatMusicCalls::GetSingleton()->Install();
	}


	bool CombatMusicCalls::Install()
	{
		SKSE::AllocTrampoline(28);
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> combatMusicTarget1{ REL::ID(46870), 0x1C1 };
		_startCombatMusic = trampoline.write_call<5>(combatMusicTarget1.address(), &StartCombatMusic);

		REL::Relocation<std::uintptr_t> combatMusicTarget2{ REL::ID(46870), 0x22D };
		_endCombatMusic = trampoline.write_call<5>(combatMusicTarget2.address(), &EndCombatMusic);

		// Not needed
		// REL::Relocation<std::uintptr_t> combatMusicTarget3{ REL::ID(18369), 0x173 };
		// _clearLocation = trampoline.write_call<5>(combatMusicTarget3.address(), &ClearLocation);
		return true;
	}

	RE::BGSMusicType* CombatMusicCalls::GetCurrentCombatMusic()
	{
		return storedMusic;
	}

	void CombatMusicCalls::SetCurrentCombatMusic(RE::BGSMusicType* a_combatMusic)
	{
		storedMusic = a_combatMusic;
	}

	RE::BGSMusicType* CombatMusicCalls::GetAppropriateMusic(RE::BGSMusicType* a_music)
	{
		if (!a_music) {
			return a_music;
		}

		const auto defaultObjects = RE::BGSDefaultObjectManager::GetSingleton();
		assert(defaultObjects);
		const auto MUSCombat = defaultObjects->GetObject<RE::BGSMusicType>(
			RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic);
		if (!MUSCombat || a_music != MUSCombat) {
			return a_music;
		}

		return a_music;
	}

	RE::BGSMusicType* CombatMusicCalls::ClearMusic()
	{
		const auto musicToStop = CombatMusicCalls::GetSingleton()->storedMusic;
		if (!musicToStop) {
			const auto defaultObjects = RE::BGSDefaultObjectManager::GetSingleton();
			assert(defaultObjects);
			const auto MUSCombat = defaultObjects->GetObject<RE::BGSMusicType>(
				RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic);
			assert(MUSCombat);

			return MUSCombat;
		}
		return musicToStop;
	}

	RE::BGSMusicType* CombatMusicCalls::StartCombatMusic(int a1)
	{
		const auto callsSingleton = CombatMusicCalls::GetSingleton();
		const auto response = _startCombatMusic(a1);
		callsSingleton->storedMusic = callsSingleton->GetAppropriateMusic(response);
		return callsSingleton->storedMusic;
	}

	RE::BGSMusicType* CombatMusicCalls::EndCombatMusic(int a1)
	{
		return CombatMusicCalls::GetSingleton()->ClearMusic();
	}

	RE::BGSMusicType* CombatMusicCalls::ClearLocation(int a1)
	{
		return CombatMusicCalls::GetSingleton()->ClearMusic();
	}
}