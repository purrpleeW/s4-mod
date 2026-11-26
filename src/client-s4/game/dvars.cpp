#include "common.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

namespace dvars {
	namespace {
		std::string dvar_get_vector_domain(const int components, const s4::DvarLimits& domain) {
			if (domain.vector_.min_ == -FLT_MAX) {
				if (domain.vector_.max_ == FLT_MAX) {
					return std::format("Domain is any {}D vector", components);
				}
				else {
					return std::format("Domain is any {}D vector with components {} or smaller", components, domain.vector_.max_);
				}
			}
			else if (domain.vector_.max_ == FLT_MAX) {
				return std::format("Domain is any {}D vector with components {} or bigger", components, domain.vector_.min_);
			}
			else {
				return std::format("Domain is any {}D vector with components from {} to {}", components, domain.vector_.min_, domain.vector_.max_);
			}
		}
	}

	s4::dvar_t* get_dvar_by_hash(std::uint64_t hash) {
		return game::Dvar_FindMalleableVar(hash);
	}

	std::string dvar_get_domain(const s4::DvarType type, const s4::DvarLimits& domain) {
		std::string str;

		switch (type) {
		case s4::DvarType::DVAR_TYPE_BOOL:
			return "Domain is 0 or 1"s;
		case s4::DvarType::DVAR_TYPE_FLOAT:
			if (domain.value_.min_ == -FLT_MAX) {
				if (domain.value_.max_ == FLT_MAX) {
					return "Domain is any number"s;
				}
				else {
					return std::format("Domain is any number {} or smaller", domain.value_.max_);
				}
			}
			else if (domain.value_.max_ == FLT_MAX) {
				return std::format("Domain is any number {} or bigger", domain.value_.min_);
			}
			else {
				return std::format("Domain is any number from {} to {}", domain.value_.min_, domain.value_.max_);
			}
		case s4::DvarType::DVAR_TYPE_FLOAT_2:
			return dvar_get_vector_domain(2, domain);
		case s4::DvarType::DVAR_TYPE_FLOAT_3_COLOR:
		case s4::DvarType::DVAR_TYPE_FLOAT_3:
			return dvar_get_vector_domain(3, domain);
		case s4::DvarType::DVAR_TYPE_FLOAT_4:
			return dvar_get_vector_domain(4, domain);
		case s4::DvarType::DVAR_TYPE_INT:
		case s4::DvarType::DVAR_TYPE_INT64:
		case s4::DvarType::DVAR_TYPE_UINT64:
			if (domain.enumeration_.string_count_ == INT_MIN) {
				if (domain.integer_.max_ == INT_MAX) {
					return "Domain is any integer"s;
				}
				else {
					return std::format("Domain is any integer {} or smaller", domain.integer_.max_);
				}
			}
			else if (domain.integer_.max_ == INT_MAX) {
				return std::format("Domain is any integer {} or bigger", domain.integer_.min_);
			}
			else {
				return std::format("Domain is any integer from {} to {}", domain.integer_.min_, domain.integer_.max_);
			}
		case s4::DvarType::DVAR_TYPE_COLOR:
			return "Domain is any 4-component color, in RGBA format"s;
		case s4::DvarType::DVAR_TYPE_ENUM:
			str = "Domain is one of the following:"s;

			for (auto stringIndex = 0; stringIndex < domain.enumeration_.string_count_; ++stringIndex) {
				str += std::format("\n  {:2}: {}", stringIndex, domain.enumeration_.strings_[stringIndex]);
			}

			return str;
		case s4::DvarType::DVAR_TYPE_STRING:
			return "Domain is any text"s;
		default:
			return std::format("unhandled dvar type '{}'", ENUM_UNDER(type));
		}
	}

	s4::dvar_t* register_bool(const std::string& name, bool value, std::uint32_t flags, const std::string& description) {
		auto hash = game::hash_dvar(name);
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterBool(hash, value, flags, "");
	}

	s4::dvar_t* register_string(const std::string& name, const std::string& value, std::uint32_t flags, const std::string& description) {
		s4::DvarValue dvar_value{};
		s4::DvarLimits dvar_domain{};

		auto hash = game::hash_dvar(name);
		insert_dvar_info(hash, name, description);
		dvar_value.string_ = value.c_str();

		return game::Dvar_RegisterVariant(hash, 9, flags, &dvar_value, &dvar_domain, "");
	}

	s4::dvar_t* register_vec4(const std::string& name, float x, float y, float z, float w, float min, float max, std::uint32_t flags,
		const std::string& description)
	{
		auto hash = game::hash_dvar(name);
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterVec4(hash, x, y, z, w, min, max, flags, "");
	}
}
