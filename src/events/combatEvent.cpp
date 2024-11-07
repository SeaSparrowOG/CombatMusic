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

	void CombatEvent::SetShouldWait(bool a_ShouldWait)
	{
		shouldWait = a_ShouldWait;
	}

	RE::BSEventNotifyControl CombatEvent::ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		using control = RE::BSEventNotifyControl;
		if (!event || !shouldWait) {
			return control::kContinue;
		}
		if (event->newState != RE::ACTOR_COMBAT_STATE::kNone) {
			return control::kContinue;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return control::kContinue;
		}

		Hooks::ActorUpdate::StartCountdown(static_cast<float>(combatMusicFixWait));
		return control::kContinue;
	}
}
