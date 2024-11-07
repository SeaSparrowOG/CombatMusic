#pragma once

#include "utilities/utilities.h"

namespace Events
{
	class CombatEvent : public Utilities::Singleton::ISingleton<CombatEvent>,
		public RE::BSTEventSink<RE::TESCombatEvent> {
	public:
		void RegisterListener();
		void SetWaitTime(long a_timeSpanSeconds);
		void SetShouldWait(bool a_ShouldWait);

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*) override;

		bool shouldWait;
		long combatMusicFixWait;
	};
}