#include "serialization/serde.h"

#include "hooks/hooks.h"

namespace Serialization {
	void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		if (!a_intfc->OpenRecord(StoredCombatMusic, Version)) {
			logger::error("Failed to open Stored Combat Music record.");
			return;
		}

		const auto* currentCombatMusic = Hooks::CombatMusicCalls::GetSingleton()->GetCurrentCombatMusic();
		if (currentCombatMusic) {
			if (!a_intfc->WriteRecordData(currentCombatMusic->formID)) {
				logger::error("Failed to write combat music ID to the save record.");
				return;
			}
		}
		else {
			RE::FormID nullID = 0x0;
			if (!a_intfc->WriteRecordData(nullID)) {
				logger::error("Failed to write Null ID to the save record.");
				return;
			}
		}
	}

	void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		std::uint32_t type;
		std::uint32_t version;
		std::uint32_t length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version != Version) {
				logger::error("Loaded data is incompatible with plugin version!");
				return;
			}

			if (type == StoredCombatMusic) {
				Hooks::CombatMusicCalls::GetSingleton()->SetCurrentCombatMusic(nullptr);

				RE::FormID musicTypeOld;
				if (!a_intfc->ReadRecordData(musicTypeOld)) {
					logger::error("Failed to read FormID");
					return;
				}

				if (musicTypeOld == 0x0) {
					return;
				}

				RE::FormID musicTypeID;
				if (!a_intfc->ResolveFormID(musicTypeOld, musicTypeID)) {
					logger::error("Failed to resolve FormID ({:08X})", musicTypeOld);
					return;
				}

				auto* musicTypeForm = RE::TESForm::LookupByID<RE::BGSMusicType>(musicTypeID);
				if (!musicTypeForm) {
					logger::error("Failed to find appropriate form ({:08X})", musicTypeID);
					return;
				}

				Hooks::CombatMusicCalls::GetSingleton()->SetCurrentCombatMusic(musicTypeForm);
			}
		}
	}

	void RevertCallback(SKSE::SerializationInterface* a_intfc)
	{
		Hooks::CombatMusicCalls::GetSingleton()->SetCurrentCombatMusic(nullptr);
	}

	std::string DecodeTypeCode(std::uint32_t a_typeCode)
	{
		constexpr std::size_t SIZE = sizeof(std::uint32_t);

		std::string sig;
		sig.resize(SIZE);
		char* iter = reinterpret_cast<char*>(&a_typeCode);
		for (std::size_t i = 0, j = SIZE - 2; i < SIZE - 1; ++i, --j) {
			sig[j] = iter[i];
		}
		return sig;
	}
}