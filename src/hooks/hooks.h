#pragma once

#include "utilities/utilities.h"

namespace Hooks {
	void Install();

	class CombatMusicCalls : public Utilities::Singleton::ISingleton<CombatMusicCalls>
	{
	public:
		enum PriorityLevel {
			LOW,
			HIGH
		};

		struct Condition {
			virtual bool IsTrue() const = 0;
			PriorityLevel level;
			bool AND;
		};

		struct CombatTargetCondition : public Condition {
			bool IsTrue() const override {
				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto combatTarget = player->currentCombatTarget.get().get();
				if (!combatTarget) {
					return false;
				}
				const auto targetBase = combatTarget->GetActorBase();
				if (!targetBase) {
					return false;
				}

				for (const auto target : targets) {
					if (target == targetBase) {
						return true;
					}
				}

				return false;
			}

			CombatTargetCondition() {
				level = PriorityLevel::HIGH;
			}
			std::vector<RE::TESNPC*> targets;
		};

		struct CombatTargetKeywordCondition : public  Condition {
			bool IsTrue() const override {
				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto combatTarget = player->currentCombatTarget.get().get();
				if (!combatTarget) {
					return false;
				}
				const auto targetBase = combatTarget->GetActorBase();
				const auto targetRace = combatTarget->GetRace();
				if (!targetBase || !targetRace) {
					return false;
				}

				for (const auto keyword : keywords) {
					if (targetBase->HasKeyword(keyword) || targetRace->HasKeyword(keyword)) {
						return true;
					}
				}

				return false;
			}

			CombatTargetKeywordCondition() {
				level = PriorityLevel::HIGH;
			}
			std::vector<RE::BGSKeyword*> keywords;
		};

		struct WorldspaceCondition : public Condition {
			bool IsTrue() const override {
				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto playerWorldspace = player->GetWorldspace();
				if (!playerWorldspace) {
					return false;
				}

				for (const auto* worldspace : worldspaces) {
					if (worldspace == playerWorldspace) {
						return true;
					}
				}

				return false;
			}

			WorldspaceCondition() {
				level = PriorityLevel::LOW;
			}
			std::vector<RE::TESWorldSpace*> worldspaces;
		};

		struct CellCondition : public Condition {
			bool IsTrue() const override {
				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto playerCell = player->GetParentCell();
				if (!playerCell) {
					return false;
				}

				for (const auto* cell : cells) {
					if (cell == playerCell) {
						return true;
					}
				}

				return false;
			}

			CellCondition() {
				level = PriorityLevel::LOW;
			}
			std::vector<RE::TESObjectCELL*> cells;
		};

		struct LocationCondition : public Condition {
			bool IsTrue() const override {
				const auto player = RE::PlayerCharacter::GetSingleton();
				auto playerLocation = player->GetCurrentLocation();
				if (!playerLocation) {
					return false;
				}

				for (const auto* location : locations) {
					if (location == playerLocation) {
						return true;
					}
				}

				while (playerLocation->parentLoc) {
					playerLocation = playerLocation->parentLoc;
					for (const auto* location : locations) {
						if (location == playerLocation) {
							return true;
						}
					}
				}

				return false;
			}

			LocationCondition() {
				level = PriorityLevel::LOW;
			}
			std::vector<RE::BGSLocation*> locations;
		};

		struct LocationKeywordCondition : public Condition {
			bool IsTrue() const override {
				const auto player = RE::PlayerCharacter::GetSingleton();
				auto playerLocation = player->GetCurrentLocation();
				if (!playerLocation) {
					return false;
				}

				for (const auto* keyword : keywords) {
					if (playerLocation->HasKeyword(keyword)) {
						return true;
					}
				}

				while (playerLocation->parentLoc) {
					playerLocation = playerLocation->parentLoc;
					for (const auto* keyword : keywords) {
						if (playerLocation->HasKeyword(keyword)) {
							return true;
						}
					}
				}

				return false;
			}

			LocationKeywordCondition() {
				level = PriorityLevel::LOW;
			}
			std::vector<RE::BGSKeyword*> keywords;
		};

		struct ConditionalBattleMusic {
			RE::BGSMusicType* music;
			std::vector<std::unique_ptr<Condition>> conditions;

			std::pair<PriorityLevel, int> MatchDegree() const {
				int response = 0;
				bool matchedOR = false;
				bool hasOR = false;
				auto priority = PriorityLevel::LOW;
				for (const auto& condition : conditions) {
					if (!hasOR && !condition->AND) {
						hasOR = true;
					}
					if (condition->IsTrue()) {
						if (!matchedOR && !condition->AND) {
							matchedOR = true;
							response++;
						}
						if (condition->level == PriorityLevel::HIGH && priority == PriorityLevel::LOW) {
							priority = PriorityLevel::HIGH;
						}
						if (condition->AND) {
							response++;
						}
					}
					else if (condition->AND) {
						return std::make_pair(PriorityLevel::LOW, 0);
					}
				}
				if (hasOR && !matchedOR) {
					return std::make_pair(PriorityLevel::LOW, 0);
				}
				return std::make_pair(priority, response);
			}

			ConditionalBattleMusic(RE::BGSMusicType* a_music) {
				this->music = a_music;
				conditions = std::vector<std::unique_ptr<Condition>>();
			}
		};

		bool Install();
		RE::BGSMusicType* GetCurrentCombatMusic();
		void SetCurrentCombatMusic(RE::BGSMusicType* a_combatMusic);
		void PushNewMusic(ConditionalBattleMusic&& newMusic);

	private:
		RE::BGSMusicType* GetAppropriateMusic(RE::BGSMusicType* a_music);
		RE::BGSMusicType* ClearMusic();

		// Reverts the combat music, in cases like exiting back to the main menu.
		static RE::BGSMusicType* RevertCombatMusic(RE::DEFAULT_OBJECT a1);
		// Main hook, called when combat starts.
		static RE::BGSMusicType* StartCombatMusic(RE::DEFAULT_OBJECT a1);
		// The game gets MUSCombat again whenever a save is loaded that had combat music playing.
		static RE::BGSMusicType* LoadCombatMusic(RE::DEFAULT_OBJECT a1);
		// Secondary hook, makes sure to end custom combat music when combat ends.
		static RE::BGSMusicType* EndCombatMusic(RE::DEFAULT_OBJECT a1);
		// Discovery music checks for MUSCombat, so we pass custom music here too.
		static RE::BGSMusicType* DiscoveryMusic(RE::DEFAULT_OBJECT a1);
		// I am not sure, frankly, but it is used in the vanilla system.
		static RE::BGSMusicType* ClearLocation(RE::DEFAULT_OBJECT a1);

		RE::BGSMusicType* storedMusic;
		std::vector<ConditionalBattleMusic> conditionalMusic;

		inline static REL::Relocation<decltype(&RevertCombatMusic)> _revertCombatMusic;
		inline static REL::Relocation<decltype(&StartCombatMusic)>  _startCombatMusic;
		inline static REL::Relocation<decltype(&LoadCombatMusic)>   _loadCombatMusic;
		inline static REL::Relocation<decltype(&EndCombatMusic)>    _endCombatMusic;
		inline static REL::Relocation<decltype(&DiscoveryMusic)>    _discoveryMusic;
		inline static REL::Relocation<decltype(&ClearLocation)>     _clearLocation;
	};
}