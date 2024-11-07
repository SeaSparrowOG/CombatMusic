#include "events/combatEvent.h"

#include "hooks/hooks.h"

namespace Events
{
	void CombatEvent::RegisterListener()
	{
		auto sink = RE::ScriptEventSourceHolder::GetSingleton();
		if (!sink) {
			return;
		}

		sink->AddEventSink(this);
	}

	void CombatEvent::SetWaitTime(long a_timeSpanSeconds)
	{
		if (a_timeSpanSeconds < 5) {
			a_timeSpanSeconds = 5;
		}
		else if (a_timeSpanSeconds > 30) {
			a_timeSpanSeconds = 30;
		}
		combatMusicFixWait = a_timeSpanSeconds;
	}

	RE::BSEventNotifyControl CombatEvent::ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		using control = RE::BSEventNotifyControl;
		if (!event || !event->actor.get()) {
			return control::kContinue;
		}
		if (event->newState != RE::ACTOR_COMBAT_STATE::kNone) {
			return control::kContinue;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player || player != event->actor.get()) {
			return control::kContinue;
		}

		// Adapted from ThirdEyeSqueegee
		auto func = [&]() {
			std::this_thread::sleep_for(std::chrono::seconds(combatMusicFixWait));
			if (!player->IsInCombat()) {
				const auto combatMusic = Hooks::CombatMusicCalls::GetSingleton()->GetCurrentCombatMusic();
				if (combatMusic) {
					combatMusic->DoFinish(true);
				}
			}
			};
		std::jthread t(func);
		t.detach();

		return control::kContinue;
	}
}
