#pragma once

#include "utilities/utilities.h"

namespace Hooks {
	void Install();

	class CombatMusicCalls : public Utilities::Singleton::ISingleton<CombatMusicCalls>
	{
	public:
		struct Condition {
			virtual bool IsTrue() const = 0;
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
			std::vector<RE::BGSLocation*> locations;
		};

		struct ConditionalBattleMusic {
			RE::BGSMusicType* music;
			std::vector<std::unique_ptr<Condition>> conditions;

			int MatchDegree() const {
				int response = 0;
				for (const auto& condition : conditions) {
					if (condition->IsTrue()) {
						response++;
					}
				}
				return response;
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

		static RE::BGSMusicType* StartCombatMusic(int a1);
		static RE::BGSMusicType* EndCombatMusic(int a1);
		static RE::BGSMusicType* ClearLocation(int a1);

		RE::BGSMusicType* storedMusic;
		std::vector<ConditionalBattleMusic> conditionalMusic;

		inline static REL::Relocation<decltype(&StartCombatMusic)> _startCombatMusic;
		inline static REL::Relocation<decltype(&EndCombatMusic)>   _endCombatMusic;
		inline static REL::Relocation<decltype(&ClearLocation)>    _clearLocation;
	};
}