#include "settings/JSONSettings.h"

#include "hooks/hooks.h"
#include "utilities/utilities.h"

#include <json/json.h>

namespace JSONSettings
{
	static std::vector<std::string> findJsonFiles()
	{
		static constexpr std::string_view directory = R"(Data/SKSE/Plugins/CombatMusic)";
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
		REX::INFO("Reading configuration files...");
		std::vector<std::string> paths{};
		try {
			paths = findJsonFiles();
		}
		catch (const std::exception& e) {
			REX::WARN("Caught {} while reading files.", e.what());
			return;
		}
		if (paths.empty()) {
			REX::INFO("No settings found");
			return;
		}

		REX::INFO("Found {} files.", paths.size());
		for (const auto& path : paths) {
			REX::INFO("Reading <{}>:", path);
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				JSONReader.parse(rawJSON, JSONFile);
			}
			catch (const Json::Exception& e) {
				REX::WARN("Caught {} while reading files.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				REX::ERROR("Caught unhandled exception {} while reading files.", e.what());
				continue;
			}

			if (!JSONFile.isObject()) {
				REX::WARN("Warning: <{}> is not an object. File will be ignored.", path);
				continue;
			}

			const auto& combatMusic = JSONFile["combatMusic"];
			if (!combatMusic || !combatMusic.isArray()) {
				REX::WARN("<{}> is missing conditional music, or conditional music is not an array.", path);
				continue;
			}

			for (const auto& entry : combatMusic) {
				if (!entry.isObject()) {
					REX::WARN("<{}> has a non-object entry in conditionalMusic. Said entry will be ignored.", path);
					continue;
				}

				bool errorOccured = false;

				auto entryWorldspaceCondition = Hooks::CombatMusicCalls::WorldspaceCondition();
				const auto& entryWorldspaces = entry["worldspaces"];
				if (entryWorldspaces && entryWorldspaces.isObject()) {
					const auto& conditionArray = entryWorldspaces["forms"];
					const auto& conditionAND = entryWorldspaces["AND"];
					if (!conditionArray || !conditionArray.isArray() || !conditionAND || !conditionAND.isBool()) {
						REX::WARN("<{}> contains a worldspace condition that is missing or has incorrect setup.", path);
						continue;
					}
					for (const auto& entryWorldspace : conditionArray) {
						if (!entryWorldspace.isString()) {
							REX::WARN("<{}> contains a worldspace condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundWorldspace = Utilities::Forms::GetFormFromString<RE::TESWorldSpace>(entryWorldspace.asString());
						if (!foundWorldspace) {
							REX::WARN("<{}> -> <{}> could not resolve form.", path, entryWorldspace.asString());
							errorOccured = true;
							continue;
						}
						entryWorldspaceCondition.worldspaces.push_back(foundWorldspace);
						entryWorldspaceCondition.AND = conditionAND.asBool();
					}
				}
				else if (entryWorldspaces) {
					REX::WARN("<{}> contains worldspaces, but it is not an object.", path);
					continue;
				}
				if (errorOccured) {
					continue;
				}

				auto entryCombatTargetCondition = Hooks::CombatMusicCalls::CombatTargetCondition();
				const auto& entryCombatTarget = entry["combatTarget"];
				if (entryCombatTarget && entryCombatTarget.isObject()) {
					const auto& conditionArray = entryCombatTarget["forms"];
					const auto& conditionAND = entryCombatTarget["AND"];
					if (!conditionArray || !conditionArray.isArray() || !conditionAND || !conditionAND.isBool()) {
						REX::WARN("<{}> contains a combatTarget condition that is missing or has incorrect setup.", path);
						continue;
					}
					for (const auto& combatTarget : conditionArray) {
						if (!combatTarget.isString()) {
							REX::WARN("<{}> contains a combatTarget condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundActor = Utilities::Forms::GetFormFromString<RE::TESNPC>(combatTarget.asString());
						if (!foundActor) {
							REX::WARN("<{}> -> <{}> could not resolve form.", path, combatTarget.asString());
							errorOccured = true;
							continue;
						}
						entryCombatTargetCondition.AND = conditionAND.asBool();
						entryCombatTargetCondition.targets.push_back(foundActor);
					}
				}
				else if (entryCombatTarget) {
					REX::WARN("<{}> contains combatTarget, but it is not an object.", path);
					continue;
				}
				if (errorOccured) {
					continue;
				}

				auto entryCombatTargetKeywordsCondition = Hooks::CombatMusicCalls::CombatTargetKeywordCondition();
				const auto& entryTargetKeywords = entry["combatTargetKeywords"];
				if (entryTargetKeywords && entryTargetKeywords.isObject()) {
					const auto& conditionArray = entryTargetKeywords["forms"];
					const auto& conditionAND = entryTargetKeywords["AND"];
					if (!conditionArray || !conditionArray.isArray() || !conditionAND || !conditionAND.isBool()) {
						REX::WARN("<{}> contains a combatTarget condition that is missing or has incorrect setup.", path);
						continue;
					}

					for (const auto& targetKeyword : conditionArray) {
						if (!targetKeyword.isString()) {
							REX::WARN("<{}> contains a combatTargetKeywords condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundKeyword = Utilities::Forms::GetFormFromString<RE::BGSKeyword>(targetKeyword.asString());
						if (!foundKeyword) {
							REX::WARN("<{}> -> <{}> could not resolve form.", path, targetKeyword.asString());
							errorOccured = true;
							continue;
						}
						entryCombatTargetKeywordsCondition.keywords.push_back(foundKeyword);
						entryCombatTargetKeywordsCondition.AND = conditionAND.asBool();
					}
				}
				else if (entryTargetKeywords) {
					REX::WARN("<{}> contains combatTargetKeywords, but it is not an object.", path);
					continue;
				}
				if (errorOccured) {
					continue;
				}

				auto entryCellCondition = Hooks::CombatMusicCalls::CellCondition();
				const auto& entryCells = entry["cells"]; 
				if (entryCells && entryCells.isObject()) {
					const auto& conditionArray = entryCells["forms"];
					const auto& conditionAND = entryCells["AND"];
					if (!conditionArray || !conditionArray.isArray() || !conditionAND || !conditionAND.isBool()) {
						REX::WARN("<{}> contains a combatTarget condition that is missing or has incorrect setup.", path);
						continue;
					}

					for (const auto& entryCell : conditionArray) {
						if (!entryCell.isString()) {
							REX::WARN("<{}> contains a cell condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundCell = Utilities::Forms::GetFormFromString<RE::TESObjectCELL>(entryCell.asString());
						if (!foundCell) {
							REX::WARN("<{}> -> <{}> could not resolve form.", path, entryCell.asString());
							errorOccured = true;
							continue;
						}
						entryCellCondition.cells.push_back(foundCell);
						entryCellCondition.AND = conditionAND.asBool();
					}
				}
				else if (entryCells) {
					REX::WARN("<{}> contains cells, but it is not an object.", path);
					continue;
				}
				if (errorOccured) {
					continue;
				}

				auto entryLocationCondition = Hooks::CombatMusicCalls::LocationCondition();
				const auto& entryLocations = entry["locations"];
				if (entryLocations && entryLocations.isObject()) {
					const auto& conditionArray = entryLocations["forms"];
					const auto& conditionAND = entryLocations["AND"];
					if (!conditionArray || !conditionArray.isArray() || !conditionAND || !conditionAND.isBool()) {
						REX::WARN("<{}> contains a combatTarget condition that is missing or has incorrect setup.", path);
						continue;
					}

					for (const auto& entryLocation : conditionArray) {
						if (!entryLocation.isString()) {
							REX::WARN("<{}> contains a cell condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundLocation = Utilities::Forms::GetFormFromString<RE::BGSLocation>(entryLocation.asString());
						if (!foundLocation) {
							REX::WARN("<{}> -> <{}> could not resolve form.", path, entryLocation.asString());
							errorOccured = true;
							continue;
						}
						entryLocationCondition.locations.push_back(foundLocation);
						entryLocationCondition.AND = conditionAND.asBool();
					}
				}
				else if (entryLocations) {
					REX::WARN("<{}> contains locations, but it is not an object.", path);
					continue;
				}
				if (errorOccured) {
					continue;
				}

				auto entryLocationKeywordCondition = Hooks::CombatMusicCalls::LocationKeywordCondition();
				const auto& entryLocationKeywords = entry["locationKeywords"];
				if (entryLocationKeywords && entryLocationKeywords.isObject()) {
					const auto& conditionArray = entryLocationKeywords["forms"];
					const auto& conditionAND = entryLocationKeywords["AND"];
					if (!conditionArray || !conditionArray.isArray() || !conditionAND || !conditionAND.isBool()) {
						REX::WARN("<{}> contains a entryLocationKeywords condition that is missing or has incorrect setup.", path);
						continue;
					}

					for (const auto& entryKeyword : conditionArray) {
						if (!entryKeyword.isString()) {
							REX::WARN("<{}> contains a cell condition that is not a string.", path);
							errorOccured = true;
							continue;
						}

						const auto foundKeyword = Utilities::Forms::GetFormFromString<RE::BGSKeyword>(entryKeyword.asString());
						if (!foundKeyword) {
							REX::WARN("<{}> -> <{}> could not resolve form.", path, entryKeyword.asString());
							errorOccured = true;
							continue;
						}
						entryLocationKeywordCondition.keywords.push_back(foundKeyword);
						entryLocationKeywordCondition.AND = conditionAND.asBool();
					}
				}
				else if (entryLocationKeywords) {
					REX::WARN("<{}> contains locationKeywords, but it is not an object", path);
					continue;
				}
				if (errorOccured) {
					continue;
				}

				const auto& entryIsCombatMusic = entry["isCombatMusic"];
				if (!entryIsCombatMusic || !entryIsCombatMusic.isBool()) {
					REX::WARN("<{}> is either missing musicType or it is not a bool.", path);
					continue;
				}
				bool isCombatMusic = entryIsCombatMusic.asBool();

				const auto& entryNewMusic = entry["newMusic"];
				if (!entryNewMusic || !entryNewMusic.isString()) {
					REX::WARN("<{}> is either missing newMusic or it is not a string.", path);
					continue;
				}
				const auto entryMusicForm = Utilities::Forms::GetFormFromString<RE::BGSMusicType>(entryNewMusic.asString());
				if (!entryMusicForm) {
					REX::WARN("<{}> -> <{}> could not resolve form.", path, entryNewMusic.asString());
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
				if (!entryLocationKeywordCondition.keywords.empty()) {
					newCombatMusic.conditions.push_back(std::make_unique<Hooks::CombatMusicCalls::LocationKeywordCondition>(entryLocationKeywordCondition));
				}
				if (!entryCombatTargetCondition.targets.empty()) {
					newCombatMusic.conditions.push_back(std::make_unique<Hooks::CombatMusicCalls::CombatTargetCondition>(entryCombatTargetCondition));
				}
				if (!entryCombatTargetKeywordsCondition.keywords.empty()) {
					newCombatMusic.conditions.push_back(std::make_unique<Hooks::CombatMusicCalls::CombatTargetKeywordCondition>(entryCombatTargetKeywordsCondition));
				}

				if (isCombatMusic) {
					Hooks::CombatMusicCalls::GetSingleton()->PushNewCombatMusic(std::move(newCombatMusic));
				}
				else {
					Hooks::CombatMusicCalls::GetSingleton()->PushNewClearedMusic(std::move(newCombatMusic));
				}
				
				REX::INFO("Created new {} music: ", isCombatMusic ? "combat" : "dungeon cleared");
				if (!entryWorldspaceCondition.worldspaces.empty()) {
					REX::INFO("  >Music will apply to these worldspaces ({}):", entryWorldspaceCondition.AND ? "AND" : "OR");
					for (const auto& string : entryWorldspaceCondition.worldspaces) {
						REX::INFO("    [{}]", string->GetFormEditorID());
					}
				}
				if (!entryCellCondition.cells.empty()) {
					REX::INFO("  >Music will apply to these cells: ({}):", entryCellCondition.AND ? "AND" : "OR");
					for (const auto& string : entryCellCondition.cells) {
						REX::INFO("    [{}]", string->GetFormEditorID());
					}
				}
				if (!entryLocationCondition.locations.empty()) {
					REX::INFO("  >Music will apply to these locations: (PO3's Tweaks must be enabled to view) ({}):", entryLocationCondition.AND ? "AND" : "OR");
					for (const auto& string : entryLocationCondition.locations) {
						auto message = Utilities::EDID::GetEditorID(string);
						if (!message.empty()) {
							REX::INFO("    [{}]", message);
						}
					}
				}
				if (!entryLocationKeywordCondition.keywords.empty()) {
					REX::INFO("  >Music will apply to locations with this keywords: ({}):", entryLocationCondition.AND ? "AND" : "OR");
					for (const auto& string : entryLocationKeywordCondition.keywords) {
						REX::INFO("    [{}]", string->GetFormEditorID());
					}
				}
				if (!entryCombatTargetKeywordsCondition.keywords.empty()) {
					REX::INFO("  >Music will apply to these locations: ({}):", entryCombatTargetKeywordsCondition.AND ? "AND" : "OR");
					for (const auto& string : entryCombatTargetKeywordsCondition.keywords) {
						REX::INFO("    [{}]", string->GetFormEditorID());
					}
				}
				if (!entryCombatTargetCondition.targets.empty()) {
					REX::INFO("  >Music will apply to these combat targets: ({}):", entryCombatTargetCondition.AND ? "AND" : "OR");
					for (const auto& string : entryCombatTargetCondition.targets) {
						REX::INFO("    [{}]", string->GetName());
					}
				}
				REX::INFO("---------------------------------------------------");
			}
			REX::INFO("Finished!");
			REX::INFO("___________________________________________________");;
		}
	}
}