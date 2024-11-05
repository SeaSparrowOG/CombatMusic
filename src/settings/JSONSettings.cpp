#include "settings/JSONSettings.h"

#include "hooks/hooks.h"
#include "utilities/utilities.h"

namespace JSONSettings
{
	static std::vector<std::string> findJsonFiles()
	{
		static constexpr std::string_view directory = R"(Data/SKSE/Plugins/StaffEnchanting)";
		std::vector<std::string> jsonFilePaths;
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			if (entry.is_regular_file() && entry.path().extension() == ".json") {
				jsonFilePaths.push_back(entry.path().string());
			}
		}

		std::sort(jsonFilePaths.begin(), jsonFilePaths.end());
		return jsonFilePaths;
	}

	void Read() {
		std::vector<std::string> paths{};
		try {
			paths = findJsonFiles();
		}
		catch (const std::exception& e) {
			logger::warn("Caught {} while reading files.", e.what());
			return;
		}
		if (paths.empty()) {
			logger::info("No settings found");
			return;
		}

		for (const auto& path : paths) {
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				JSONReader.parse(rawJSON, JSONFile);
			}
			catch (const Json::Exception& e) {
				logger::warn("Caught {} while reading files.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				logger::error("Caught unhandled exception {} while reading files.", e.what());
				continue;
			}

			if (!JSONFile.isObject()) {
				logger::warn("Warning: <{}> is not an object. File will be ignored.", path);
				continue;
			}

			const auto& combatMusic = JSONFile["combatMusic"];
			if (!combatMusic || !combatMusic.isArray()) {
				logger::warn("<{}> is missing conditional music, or conditional music is not an array.", path);
				continue;
			}

			for (const auto& entry : combatMusic) {
				if (!entry.isObject()) {
					logger::warn("<{}> has a non-object entry in conditionalMusic. Said entry will be ignored.", path);
					continue;
				}

				bool errorOccured = false;

				auto entryWorldspaceCondition = Hooks::CombatMusicCalls::WorldspaceCondition();
				const auto& entryWorldspaces = entry["worldspaces"];
				if (entryWorldspaces && entryWorldspaces.isArray()) {
					for (const auto& entryWorldspace : entryWorldspaces) {
						if (!entryWorldspace.isString()) {
							logger::warn("<{}> contains a worldspace condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundWorldspace = Utilities::Forms::GetFormFromString<RE::TESWorldSpace>(entryWorldspace.asString());
						if (!foundWorldspace) {
							logger::warn("<{}> -> <{}> could not resolve form.", path, entryWorldspace.asString());
							errorOccured = true;
							continue;
						}
						entryWorldspaceCondition.worldspaces.push_back(foundWorldspace);
					}
				}
				if (errorOccured) {
					continue;
				}

				auto entryCellCondition = Hooks::CombatMusicCalls::CellCondition();
				const auto& entryCells = entry["cells"]; 
				if (entryCells && entryCells.isArray()) {
					for (const auto& entryCell : entryCells) {
						if (!entryCell.isString()) {
							logger::warn("<{}> contains a cell condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundCell = Utilities::Forms::GetFormFromString<RE::TESObjectCELL>(entryCell.asString());
						if (!foundCell) {
							logger::warn("<{}> -> <{}> could not resolve form.", path, entryCell.asString());
							errorOccured = true;
							continue;
						}
						entryCellCondition.cells.push_back(foundCell);
					}
				}
				if (errorOccured) {
					continue;
				}

				auto entryLocationCondition = Hooks::CombatMusicCalls::LocationCondition();
				const auto& entryLocations = entry["locations"];
				if (entryLocations && entryLocations.isArray()) {
					for (const auto& entryLocation : entryLocations) {
						if (!entryLocation.isString()) {
							logger::warn("<{}> contains a cell condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundLocation = Utilities::Forms::GetFormFromString<RE::BGSLocation>(entryLocation.asString());
						if (!foundLocation) {
							logger::warn("<{}> -> <{}> could not resolve form.", path, entryLocation.asString());
							errorOccured = true;
							continue;
						}
						entryLocationCondition.locations.push_back(foundLocation);
					}
				}
				if (errorOccured) {
					continue;
				}

				const auto& entryMusicType = entry["musicType"];
				if (!entryMusicType || !entryMusicType.isString()) {
					logger::warn("<{}> is either missing musicType or it is not a string.", path);
					continue;
				}
				const auto entryMusicForm = Utilities::Forms::GetFormFromString<RE::BGSMusicType>(entryMusicType.asString());
				if (!entryMusicForm) {
					logger::warn("<{}> -> <{}> could not resolve form.", path, entryMusicType.asString());
					continue;
				}
				
				auto newCombatMusic = Hooks::CombatMusicCalls::ConditionalBattleMusic(entryMusicForm);
				if (!entryWorldspaceCondition.worldspaces.empty()) {
					newCombatMusic.conditions.push_back(std::make_unique<Hooks::CombatMusicCalls::WorldspaceCondition>(entryWorldspaceCondition));
				}
				if (!entryCellCondition.cells.empty()) {
					newCombatMusic.conditions.push_back(std::make_unique<Hooks::CombatMusicCalls::CellCondition>(entryCellCondition));
				}
				if (!entryLocationCondition.locations.empty()) {
					newCombatMusic.conditions.push_back(std::make_unique<Hooks::CombatMusicCalls::LocationCondition>(entryLocationCondition));
				}

				Hooks::CombatMusicCalls::GetSingleton()->PushNewMusic(std::move(newCombatMusic));
				logger::info("Created new combat music: ");
				if (!entryWorldspaceCondition.worldspaces.empty()) {
					logger::info("  >Music will apply to these worldspaces:");
					for (const auto& string : entryWorldspaceCondition.worldspaces) {
						logger::info("    [{}]", string->GetFormEditorID());
					}
				}
				if (!entryCellCondition.cells.empty()) {
					logger::info("  >Music will apply to these cells:");
					for (const auto& string : entryCellCondition.cells) {
						logger::info("    [{}]", string->GetFormEditorID());
					}
				}
				if (!entryLocationCondition.locations.empty()) {
					logger::info("  >Music will apply to these locations:");
					for (const auto& string : entryLocationCondition.locations) {
						logger::info("    [{}]", string->GetFormEditorID());
					}
				}
			}
		}
	}
}