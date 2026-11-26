#pragma once
#include "common.hpp"
#include "game/function_types.hpp"
#include "game/game.hpp"

namespace dvars {
	struct dvar_info {
		std::string name;
		std::string description;
		std::uint64_t hash;
	};

	inline std::unordered_map<std::uint64_t, dvar_info> dvar_map;

	inline s4::dvar_t* con_inputBoxColor = nullptr;
	inline s4::dvar_t* con_inputCmdMatchColor = nullptr;
	inline s4::dvar_t* con_inputDvarInactiveValueColor = nullptr;
	inline s4::dvar_t* con_inputDvarMatchColor = nullptr;
	inline s4::dvar_t* con_inputDvarValueColor = nullptr;
	inline s4::dvar_t* con_inputHintBoxColor = nullptr;
	inline s4::dvar_t* con_inputHintDescColor = nullptr;
	inline s4::dvar_t* con_inputHintDescUnavailableColor = nullptr;
	inline s4::dvar_t* con_inputTextColor = nullptr;
	inline s4::dvar_t* con_inputTitleColor = nullptr;
	inline s4::dvar_t* con_outputBarColor = nullptr;
	inline s4::dvar_t* con_outputSliderColor = nullptr;
	inline s4::dvar_t* con_outputTextColor = nullptr;
	inline s4::dvar_t* con_outputWindowColor = nullptr;
	inline s4::dvar_t* lfx_masterServerIP = nullptr;
	inline s4::dvar_t* lfx_masterServerPort = nullptr;
	inline s4::dvar_t* lui_checkIfLevelInFileSystem = nullptr;
	inline s4::dvar_t* lui_enableDebugPrinting = nullptr;
	inline s4::dvar_t* sv_cheats = nullptr;

	inline bool get_enabled(s4::dvar_t* dvar, bool default_value = false) {
		if (dvar != nullptr) {
			return dvar->get_current().enabled_;
		}
		return default_value;
	}

	inline int get_integer(s4::dvar_t* dvar, int default_value = 0) {
		if (dvar != nullptr) {
			return dvar->get_current().integer_;
		}
		return default_value;
	}

	inline std::uint32_t get_unsigned_int(s4::dvar_t* dvar, std::uint32_t default_value = 0) {
		if (dvar != nullptr) {
			return dvar->get_current().unsigned_int_;
		}
		return default_value;
	}

	inline std::int64_t get_integer64(s4::dvar_t* dvar, std::int64_t default_value = 0) {
		if (dvar != nullptr) {
			return dvar->get_current().integer64_;
		}
		return default_value;
	}

	inline std::uint64_t get_unsigned_int64(s4::dvar_t* dvar, std::uint64_t default_value = 0) {
		if (dvar != nullptr) {
			return dvar->get_current().unsigned_int64_;
		}
		return default_value;
	}

	inline float get_value(s4::dvar_t* dvar, float default_value = 0.f) {
		if (dvar != nullptr) {
			return dvar->get_current().value_;
		}
		return default_value;
	}

	inline s4::vec4_t get_vector(s4::dvar_t* dvar, s4::vec4_t default_value = { 0.f, 0.f, 0.f, 0.f }) {
		if (dvar != nullptr) {
			return dvar->get_current().vector_;
		}
		return default_value;
	}

	inline const char* get_string(s4::dvar_t* dvar, const char* default_value = "") {
		if (dvar != nullptr && dvar->get_current().string_ != nullptr) {
			return dvar->get_current().string_;
		}
		return default_value;
	}

	s4::dvar_t* get_dvar_by_hash(std::uint64_t hash);

	inline s4::dvar_t* get_dvar_by_name(const std::string& name) {
		return get_dvar_by_hash(game::hash_dvar(name));
	}

	inline void insert_dvar_info(const std::uint64_t hash, const std::string& name, const std::string& description) {
		dvar_info info{};
		info.hash = hash;
		info.name = name;
		info.description = description;
		dvar_map.insert(std::make_pair(hash, info));
	}

	inline void insert_dvar_info(const std::string& name, const std::string& description) {
		insert_dvar_info(game::hash_dvar(name), name, description);
	}

	inline std::optional<dvar_info> get_dvar_info_from_hash(const std::uint64_t hash) {
		const auto iter = dvar_map.find(hash);
		if (iter != dvar_map.end()) {
			return iter->second;
		}

		return {};
	}

	inline std::optional<dvar_info> get_dvar_info(const std::string& name) {
		const auto hash = game::hash_dvar(name);
		return get_dvar_info_from_hash(hash);
	}

	inline std::string dvar_get_description(const std::string& name) {
		const auto info = get_dvar_info(name);
		if (info.has_value()) {
			return info->description;
		}

		return {};
	}

	std::string dvar_get_domain(const s4::DvarType type, const s4::DvarLimits& domain);

	s4::dvar_t* register_bool(const std::string& name, bool value, std::uint32_t flags = 0, const std::string& description = "");
	s4::dvar_t* register_string(const std::string& name, const std::string& value, std::uint32_t flags = 0, const std::string& description = "");
	s4::dvar_t* register_vec4(const std::string& name, float x, float y, float z, float w, float min, float max, std::uint32_t flags = 0,
		const std::string& description = "");
}
