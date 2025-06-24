#pragma once

#include <json/json.h>

namespace Settings
{
	namespace JSON
	{
		bool Read();

		class Reader : public REX::Singleton<Reader>
		{
		public:
			bool Read();

		private:
			bool ReadConfig(const Json::Value& a_entry);

			RE::TESDataHandler* dataHandler{ nullptr };
			// Helper functions
			using _GetFormEditorID = const char* (*)(std::uint32_t);
			std::string GetEditorID(const RE::TESForm* a_form);

			inline std::vector<std::string> split(const std::string& a_str, const std::string& a_delimiter);
			inline bool is_only_hex(std::string_view a_str, bool a_requirePrefix = true);
			inline std::string tolower(std::string_view a_str);

			template <class T>
			T to_num(const std::string& a_str, bool a_hex = false)
			{
				const int base = a_hex ? 16 : 10;

				if constexpr (std::is_same_v<T, double>) {
					return static_cast<T>(std::stod(a_str, nullptr));
				}
				else if constexpr (std::is_same_v<T, float>) {
					return static_cast<T>(std::stof(a_str, nullptr));
				}
				else if constexpr (std::is_same_v<T, std::int64_t>) {
					return static_cast<T>(std::stol(a_str, nullptr, base));
				}
				else if constexpr (std::is_same_v<T, std::uint64_t>) {
					return static_cast<T>(std::stoull(a_str, nullptr, base));
				}
				else if constexpr (std::is_signed_v<T>) {
					return static_cast<T>(std::stoi(a_str, nullptr, base));
				}
				else {
					return static_cast<T>(std::stoul(a_str, nullptr, base));
				}
			}

			template <typename T>
			T* GetFormFromString(const std::string& a_str)
			{
				T* response = nullptr;
				if (const auto splitID = split(a_str, "|"); splitID.size() == 2) {
					const auto& modName = splitID[0];
					if (!dataHandler->LookupModByName(modName)) {
						return response;
					}
					if (!is_only_hex(splitID[1])) {
						return response;
					}

					try {
						const auto  formID = to_num<RE::FormID>(splitID[1], true);
						auto* intermediate = dataHandler->LookupForm(formID, modName);
						if (intermediate) {
							return skyrim_cast<T*>(intermediate);
						}
					}
					catch (std::exception& e) {
						logger::error("Caught exception: {}", e.what());
						return response;
					}
				}
				auto* intermediate = RE::TESForm::LookupByEditorID(a_str);
				return intermediate ? skyrim_cast<T*>(intermediate) : nullptr;
			}
		};

		inline static constexpr std::uint8_t PARSER_VERSION = 1;
		inline static constexpr const char* MINIMUM_VERSION_FIELD = "MinimumVersion";
	}
}