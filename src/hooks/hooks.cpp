#include "Hooks/hooks.h"

namespace Hooks {
	void Install()
	{
		CombatMusicCalls::GetSingleton()->Install();
		ActorUpdate::Install();
	}

	bool CombatMusicCalls::Install()
	{
		logger::info("Preparing to install hooks...");
		SKSE::AllocTrampoline(84);
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> combatMusicTarget0{ REL::ID(46897), 0x20 };
		_revertCombatMusic = trampoline.write_call<5>(combatMusicTarget0.address(), &RevertCombatMusic);
		logger::info("  >Installed revert combat music hook...");

		REL::Relocation<std::uintptr_t> combatMusicTarget1{ REL::ID(46870), 0x1C1 };
		_startCombatMusic = trampoline.write_call<5>(combatMusicTarget1.address(), &StartCombatMusic);
		logger::info("  >Installed start combat music hook...");

		REL::Relocation<std::uintptr_t> combatMusicTarget2{ REL::ID(46870), 0x22D };
		_endCombatMusic = trampoline.write_call<5>(combatMusicTarget2.address(), &EndCombatMusic);
		logger::info("  >Installed end combat music hook...");

		REL::Relocation<std::uintptr_t> combatMusicTarget3{ REL::ID(18369), 0x173 };
		_clearLocation = trampoline.write_call<5>(combatMusicTarget3.address(), &ClearLocation);
		logger::info("  >Installed clear location combat music hook...");

		REL::Relocation<std::uintptr_t> combatMusicTarget4{ REL::ID(51653), 0x29C };
		_discoveryMusic = trampoline.write_call<5>(combatMusicTarget4.address(), &DiscoveryMusic);
		logger::info("  >Installed discover location combat music hook...");

		REL::Relocation<std::uintptr_t> combatMusicTarget5{ REL::ID(46895), 0x3B4 };
		_loadCombatMusic = trampoline.write_call<5>(combatMusicTarget5.address(), &LoadCombatMusic);
		logger::info("  >Installed load combat music hook...");

		logger::info("Finished installing hooks.");
		logger::info("___________________________________________________");
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

	void CombatMusicCalls::PushNewCombatMusic(ConditionalBattleMusic&& newMusic)
	{
		if (newMusic.conditions.empty()) {
			return;
		}
		conditionalMusic.push_back(std::move(newMusic));
	}

	void CombatMusicCalls::PushNewClearedMusic(ConditionalBattleMusic&& newMusic)
	{
		if (newMusic.conditions.empty()) {
			return;
		}
		conditionalClearedMusic.push_back(std::move(newMusic));
	}

	RE::BGSMusicType* CombatMusicCalls::GetAppropriateCombatMusic(RE::BGSMusicType* a_music)
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

		RE::BGSMusicType* newMusic = nullptr;
		int bestMatch = 0;
		PriorityLevel bestPriorityLevel = PriorityLevel::LOW;
		for (const auto& candidate : conditionalMusic) {
			const auto candidateMatch = candidate.MatchDegree();
			if (candidateMatch.first == PriorityLevel::HIGH && bestPriorityLevel == PriorityLevel::LOW) {
				bestPriorityLevel = PriorityLevel::HIGH;
				bestMatch = candidateMatch.second;
				newMusic = candidate.music;
			}
			else if (candidateMatch.first == PriorityLevel::LOW && bestPriorityLevel == PriorityLevel::HIGH) {
				continue;
			}
			else if (bestMatch < candidateMatch.second) {
				bestMatch = candidateMatch.second;
				newMusic = candidate.music;
			}
		}

		if (newMusic) {
			logger::debug("  Starting {}", Utilities::EDID::GetEditorID(newMusic));
			storedMusic = newMusic;
			return newMusic;
		}
		return a_music;
	}

	RE::BGSMusicType* CombatMusicCalls::GetAppropriateClearedMusic(RE::BGSMusicType* a_music)
	{
		RE::BGSMusicType* newMusic = nullptr;
		int bestMatch = 0;
		PriorityLevel bestPriorityLevel = PriorityLevel::LOW;
		for (const auto& candidate : conditionalClearedMusic) {
			const auto candidateMatch = candidate.MatchDegree();
			if (candidateMatch.first == PriorityLevel::HIGH && bestPriorityLevel == PriorityLevel::LOW) {
				bestPriorityLevel = PriorityLevel::HIGH;
				bestMatch = candidateMatch.second;
				newMusic = candidate.music;
			}
			else if (candidateMatch.first == PriorityLevel::LOW && bestPriorityLevel == PriorityLevel::HIGH) {
				continue;
			}
			else if (bestMatch < candidateMatch.second) {
				bestMatch = candidateMatch.second;
				newMusic = candidate.music;
			}
		}

		if (newMusic) {
			logger::debug("  Starting {}", Utilities::EDID::GetEditorID(newMusic));
			storedMusic = newMusic;
			return newMusic;
		}
		return a_music;
	}

	RE::BGSMusicType* CombatMusicCalls::ClearMusic()
	{
		const auto callsSingleton = CombatMusicCalls::GetSingleton();
		const auto musicToStop = callsSingleton->storedMusic;
		callsSingleton->storedMusic = nullptr;

		if (!musicToStop) {
			const auto defaultObjects = RE::BGSDefaultObjectManager::GetSingleton();
			assert(defaultObjects);
			const auto MUSCombat = defaultObjects->GetObject<RE::BGSMusicType>(
				RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic);
			assert(MUSCombat);

			return MUSCombat;
		}
		logger::debug("  Stopping {}", Utilities::EDID::GetEditorID(musicToStop));
		return musicToStop;
	}

	RE::BGSMusicType* CombatMusicCalls::RevertCombatMusic(RE::DEFAULT_OBJECT a1)
	{
#ifdef DEBUG
		const auto obj = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(a1);
		logger::debug("Revert combat music hook ({})", obj ? Utilities::EDID::GetEditorID(obj) : "NULL");
#endif
		const auto response = _revertCombatMusic(a1);
		if (a1 != RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic) {
			return response;
		}

		return CombatMusicCalls::GetSingleton()->ClearMusic();
	}

	RE::BGSMusicType* CombatMusicCalls::StartCombatMusic(RE::DEFAULT_OBJECT a1)
	{
#ifdef DEBUG
		const auto obj = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(a1);
		logger::debug("Start combat music hook ({})", obj ? Utilities::EDID::GetEditorID(obj) : "NULL");
#endif
		const auto response = _startCombatMusic(a1);
		if (a1 != RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic) {
			return response;
		}

		return CombatMusicCalls::GetSingleton()->GetAppropriateCombatMusic(response);
	}

	RE::BGSMusicType* CombatMusicCalls::LoadCombatMusic(RE::DEFAULT_OBJECT a1)
	{
#ifdef DEBUG
		const auto obj = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(a1);
		logger::debug("Load combat music hook ({})", obj ? Utilities::EDID::GetEditorID(obj) : "NULL");
#endif
		const auto response = _loadCombatMusic(a1);
		if (a1 != RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic) {
			return response;
		}

		return CombatMusicCalls::GetSingleton()->GetAppropriateCombatMusic(response);
	}

	RE::BGSMusicType* CombatMusicCalls::EndCombatMusic(RE::DEFAULT_OBJECT a1)
	{
#ifdef DEBUG
		const auto obj = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(a1);
		logger::debug("End combat music hook ({})", obj ? Utilities::EDID::GetEditorID(obj) : "NULL");
#endif
		const auto response = _endCombatMusic(a1);
		if (a1 != RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic) {
			return response;
		}

		return CombatMusicCalls::GetSingleton()->ClearMusic();
	}

	RE::BGSMusicType* CombatMusicCalls::DiscoveryMusic(RE::DEFAULT_OBJECT a1)
	{
#ifdef DEBUG
		const auto obj = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(a1);
		logger::debug("Discovery music hook ({})", obj ? Utilities::EDID::GetEditorID(obj) : "NULL");
#endif
		if (a1 == RE::BGSDefaultObjectManager::DefaultObject::kBattleMusic) {
			const auto storedMusic = GetSingleton()->storedMusic;
			if (storedMusic) {
				return storedMusic;
			}
		}
		return _discoveryMusic(a1);
	}

	RE::BGSMusicType* CombatMusicCalls::ClearLocation(RE::DEFAULT_OBJECT a1)
	{
#ifdef DEBUG
		const auto obj = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(a1);
		logger::debug("Clear location music hook ({})", obj ? Utilities::EDID::GetEditorID(obj) : "NULL");
#endif
		const auto response = _clearLocation(a1);
		if (a1 != RE::BGSDefaultObjectManager::DefaultObject::kDungeonClearedMusic) {
			return response;
		}

		return CombatMusicCalls::GetSingleton()->GetAppropriateClearedMusic(response);
	}
}