#include "utils/properties.hpp"

#include "utils/safe_release.hpp"
#include "utils/flags.hpp"

#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/encodedstream.h>

#include "utils/io.hpp"
#include "utils/com.hpp"
#include "utils/string.hpp"

#include "identification/utility.hpp"

namespace utils::properties {
	namespace {
		void migrate_if_needed() {
			std::string folder_name = std::format("{}_players", identification::utility::get_client_name());

			std::error_code e;

			// Folder does not exist. Nothing to migrate
			if (!std::filesystem::is_directory("players", e)) {
				return;
			}

			// Folder does exist. Already migrated
			if (std::filesystem::is_directory(folder_name, e)) {
				return;
			}

			utils::io::create_directory(folder_name);

			std::filesystem::copy("players", folder_name, std::filesystem::copy_options::recursive, e);
		}

		typedef rapidjson::EncodedOutputStream<rapidjson::UTF8<>, rapidjson::FileWriteStream> OutputStream;
		typedef rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::FileReadStream> InputStream;

		std::filesystem::path get_properties_folder() {
			static auto props = get_appdata_path() / "user";
			return props;
		}

		std::filesystem::path get_properties_file() {
			static auto props = get_properties_folder() / "properties.json";
			return props;
		}

		rapidjson::Document load_properties() {
			rapidjson::Document default_doc{};
			default_doc.SetObject();

			char read_buffer[256]{0}; // Raw buffer for reading

			const std::wstring& props = get_properties_file();

			FILE* fp;
			auto err = _wfopen_s(&fp, props.data(), L"rb");
			if (err || !fp) {
				return default_doc;
			}

			const auto _ = safe_release([&fp] {
				if (fp) {
					fclose(fp);
				}
			});

			// This will handle the BOM
			rapidjson::FileReadStream bis(fp, read_buffer, sizeof(read_buffer));
			InputStream eis(bis);

			rapidjson::Document doc{};
			const rapidjson::ParseResult result = doc.ParseStream<rapidjson::kParseNoFlags, rapidjson::UTF8<>>(eis);

			if (!result || !doc.IsObject()) {
				return default_doc;
			}

			return doc;
		}

		void store_properties(const rapidjson::Document& doc) {
			char write_buffer[256]{0}; // Raw buffer for writing

			const std::wstring& props = get_properties_file();
			io::create_directory(get_properties_folder());

			FILE* fp;
			auto err = _wfopen_s(&fp, props.data(), L"wb");
			if (err || !fp) {
				return;
			}

			const auto _ = safe_release([&fp] {
				if (fp) {
					fclose(fp);
				}
			});

			rapidjson::FileWriteStream bos(fp, write_buffer, sizeof(write_buffer));
			OutputStream eos(bos);

			rapidjson::Writer writer(eos);
			doc.Accept(writer);
		}
	}

	std::filesystem::path get_game_mod_path() {
		static auto path = []() {
			auto dir = std::filesystem::current_path() / std::format(".{}", identification::utility::get_client_name());
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	std::filesystem::path get_appdata_path() {
		PWSTR path;
		if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
			throw std::runtime_error("Failed to read APPDATA path!");
		}

		auto _ = utils::safe_release([&path] {
			CoTaskMemFree(path);
		});

		static bool handled_dual = false;
		static std::string client_folder = identification::utility::get_client_name();
		if (!handled_dual) {
			if (utils::flags::has_flag("dual")) {
				client_folder += "-2";
			}
			handled_dual = true;
		}

		static auto appdata = std::filesystem::path(path) / client_folder;
		return appdata;
	}

	std::unique_lock<named_mutex> lock() {
		static named_mutex mutex{ std::format("{}-properties-lock", identification::utility::get_client_name()) };
		return std::unique_lock{ mutex };
	}

	std::optional<std::string> load(const std::string& name) {
		const auto _ = lock();
		const auto doc = load_properties();

		if (!doc.HasMember(name.c_str())) {
			return {};
		}

		const auto& value = doc[name.c_str()];
		if (!value.IsString()) {
			return {};
		}

		return {std::string{value.GetString()}};
	}

	void store(const std::string& name, const std::string& value) {
		const auto _ = lock();
		auto doc = load_properties();

		while (doc.HasMember(name.c_str())) {
			doc.RemoveMember(name.c_str());
		}

		rapidjson::Value key{};
		key.SetString(name.c_str(), doc.GetAllocator());

		rapidjson::Value member{};
		member.SetString(value.c_str(), doc.GetAllocator());

		doc.AddMember(key, member, doc.GetAllocator());

		store_properties(doc);
	}
}
