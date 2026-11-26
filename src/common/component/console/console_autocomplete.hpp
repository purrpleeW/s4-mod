#pragma once
#include "console_shared.hpp"
#include "console_providers.hpp"
#include "utils/cryptography.hpp"
#include "utils/string.hpp"

#include <vector>
#include <unordered_map>
#include <string>

namespace console::detail {

	class autocomplete_engine {
		std::vector<suggestion_entry> suggestions_;
		std::unordered_map<char, std::vector<suggestion_entry>> available_suggestions_;
		std::unordered_set<std::uint64_t> registered_checksums_;
		std::string current_choice_;
		std::string last_input_;
		std::string last_input_for_iteration_{};
		bool has_active_suggestion_ = false;
		std::mutex mutex;

		static constexpr std::string_view hashed_dvar_prefix = "#x3";
	public:
		std::function<std::vector<std::string>()> get_command_names_fn{};

		autocomplete_engine() {
			this->available_suggestions_.reserve(27 /*A-Z + #*/);
			this->suggestions_.reserve(1024);
			this->registered_checksums_.reserve(8192);

			this->available_suggestions_['#'] = {};
			for (auto c = 'a'; c <= 'z'; c++) {
				this->available_suggestions_[c] = {};
			}
		}

		void register_suggestion(const suggestion_entry& entry) noexcept {
			std::lock_guard _(this->mutex);

			if (entry.name.empty()) {
				return;
			}

			if (this->registered_checksums_.contains(entry.checksum)) {
				return;
			}

			const char prefix = std::tolower(entry.name.at(0));
			this->available_suggestions_[prefix].emplace_back(entry);
			this->registered_checksums_.insert(entry.checksum);

			if (entry.obfuscated_name.has_value() && !entry.obfuscated_name.value().empty()) {
				const char obf_prefix = std::tolower(entry.obfuscated_name.value().at(0));
				if (obf_prefix != prefix) {
					this->available_suggestions_[obf_prefix].emplace_back(entry);
				}
			}
		}

		void remove_suggestion(std::uint64_t checksum) {
			std::lock_guard _(this->mutex);

			if (!this->registered_checksums_.contains(checksum)) {
				return;
			}

			for (auto& [_, entries] : this->available_suggestions_) {
				std::erase_if(entries, [checksum](const suggestion_entry& entry) {
					return entry.checksum == checksum;
				});
			}

			std::erase_if(this->suggestions_, [checksum](const suggestion_entry& entry) {
				return entry.checksum == checksum;
			});

			this->registered_checksums_.erase(checksum);
		}

		void remove_kind(suggestion_kind kind) {
			std::lock_guard _(this->mutex);
			std::unordered_set<std::uint64_t> checksums{};

			auto remove_cb = [&](const suggestion_entry& entry) {
				auto res = entry.kind == kind;
				if (res) {
					checksums.insert(entry.checksum);
				}
				return res;
			};

			for (auto& [_, entries] : this->available_suggestions_) {
				std::erase_if(entries, remove_cb);
			}
			std::erase_if(this->suggestions_, remove_cb);

			for (auto& checksum : checksums) {
				this->registered_checksums_.erase(checksum);
			}
		}

 		void clear() {
			this->suggestions_.clear();
			this->current_choice_.clear();
			this->has_active_suggestion_ = false;
		}

		void reset() {
			this->clear();
			this->last_input_.clear();
		}

		void update(const std::string& input) noexcept {
			if (input == this->last_input_) {
				return;
			}

			this->clear();
			this->last_input_ = input;

			if (input.empty()) {
				return;
			}

			if (const auto space_idx = input.find(' '); space_idx != std::string::npos) {
				this->find_suggestions(input.substr(0, space_idx), true);
			} 
			else {
				this->find_suggestions(input, false);
			}
		}

		void set_active_choice(const std::string& choice) {
			this->current_choice_ = choice;
			this->has_active_suggestion_ = true;
		}

		void clear_active_choice() {
			this->has_active_suggestion_ = false;
		}

		[[nodiscard]] const auto& get_suggestions() const noexcept {
			return this->suggestions_;
		}

		[[nodiscard]] auto suggestion_count() const noexcept {
			return this->suggestions_.size();
		}

		[[nodiscard]] const std::string& get_current_choice() const noexcept {
			return this->current_choice_;
		}

		[[nodiscard]] bool has_active_suggestion() const noexcept {
			return this->has_active_suggestion_;
		}

		[[nodiscard]] bool has_overflow() const noexcept {
			return this->suggestions_.size() > layout::max_visible_suggestions;
		}

		[[nodiscard]] bool has_multiple() const noexcept {
			return this->suggestions_.size() > 1 && this->suggestions_.size() <= layout::max_visible_suggestions;
		}

		[[nodiscard]] bool has_single() const noexcept {
			return this->suggestions_.size() == 1;
		}
	private:
		void find_suggestions(const std::string& raw_input, bool exact) noexcept {
			if (raw_input.empty()) {
				return;
			}

			const auto normalized_input = utils::string::to_lower(raw_input);
			if (this->is_hashed_dvar_input(normalized_input)) {
				this->find_hashed_dvar_suggestion(normalized_input, exact);
				return;
			}

			if (get_command_names_fn && this->last_input_for_iteration_ != raw_input) {
				this->remove_kind(suggestion_kind::command);
				for (auto& cmd : get_command_names_fn()) {
					this->register_suggestion({
						.kind = console::detail::suggestion_kind::command,
						.name = cmd,
						.description = std::nullopt,
						.checksum = utils::cryptography::jenkins_one_at_a_time::compute(cmd)
					});
				}
				this->last_input_for_iteration_ = raw_input;
			}

			std::unordered_set<std::uint64_t> checked_checksums;
			checked_checksums.reserve(this->registered_checksums_.size());

			const auto try_suggestion = [this, &normalized_input, exact, &checked_checksums](const suggestion_entry& match) {
				if (!checked_checksums.insert(match.checksum).second) {
					return false;
				}

				if (this->is_valid_match(normalized_input, match, exact)) {
					this->suggestions_.emplace_back(match);
				}

				return exact && this->suggestions_.size() > 1;
			};

			if (exact) {
				const auto prefix = normalized_input.at(0);
				auto it = this->available_suggestions_.find(prefix);
				if (it == this->available_suggestions_.end()) {
					return;
				}

				for (const auto& match : it->second) {
					if (try_suggestion(match)) {
						return;
					}
				}
			}
			else {
				for (const auto& [_, entries] : this->available_suggestions_) {
					for (const auto& match : entries) {
						try_suggestion(match);
					}
				}
			}

			if (this->suggestions_.empty()) {
				this->try_find_dvar_fallback(raw_input);
			}
		}

		void find_hashed_dvar_suggestion(const std::string& normalized_input, bool exact) noexcept {
			const auto checksum_opt = this->parse_hashed_dvar(normalized_input);
			bool found_exact = false;

			if (checksum_opt.has_value()) {
				const auto checksum = checksum_opt.value();

				const auto registered = this->find_by_checksum(checksum);
				if (registered && game_data_provider::find_dvar_by_hash(registered->get().checksum)) {
					this->suggestions_.emplace_back(registered->get());
					found_exact = true;
				}

				if (!found_exact && game_data_provider::find_dvar_by_hash(checksum)) {
					const auto hash_name = this->format_hash_name(checksum);

					suggestion_entry new_entry{
						.kind = suggestion_kind::dvar,
						.name = hash_name,
						.checksum = checksum
					};

					this->register_suggestion(new_entry);
					this->suggestions_.emplace_back(std::move(new_entry));
					found_exact = true;
				}
			}

			this->find_partial_hash_matches(normalized_input, exact);
		}

		void find_partial_hash_matches(const std::string& partial_hash_input, bool exact) {
			for (const auto& [prefix, entries] : this->available_suggestions_) {
				for (const auto& candidate : entries) {
					if (this->has_checksum_been_suggested(candidate.checksum)) {
						continue;
					}

					const auto candidate_hash_name = this->format_hash_name(candidate.checksum);
					const auto normalized_candidate = utils::string::to_lower(candidate_hash_name);

					if (utils::string::match_compare(partial_hash_input, normalized_candidate, exact)) {
						if (game_data_provider::find_dvar_by_hash(candidate.checksum)) {
							this->suggestions_.emplace_back(candidate);
						}
					}
				}
			}
		}

		[[nodiscard]] bool has_checksum_been_suggested(std::uint64_t checksum) const {
			for (const auto& suggestion : this->suggestions_) {
				if (suggestion.checksum == checksum) {
					return true;
				}
			}
			return false;
		}

		void try_find_dvar_fallback(const std::string& input) noexcept {
			const auto normalized_input = utils::string::to_lower(input);
			const auto fallback_checksum = game_data_provider::generate_dvar_checksum(normalized_input);
			if (!game_data_provider::find_dvar_by_hash(fallback_checksum)) {
				return;
			}

			const auto entry = suggestion_entry{
				.kind = suggestion_kind::dvar,
				.name = input,
				.description = std::nullopt,
				.checksum = fallback_checksum
			};

			if (auto registered = this->find_by_checksum(fallback_checksum)) {
				auto& entry = registered->get();

				if (this->is_hashed_dvar_input(entry.name) && !this->is_hashed_dvar_input(normalized_input)) {
					const char old_prefix = std::tolower(entry.name.at(0));
					const char new_prefix = std::tolower(normalized_input.at(0));

					entry.name = input;
					if (old_prefix != new_prefix) {
						this->available_suggestions_[new_prefix].emplace_back(entry);
					}
				}

				this->suggestions_.emplace_back(entry);
				return;
			}

			this->register_suggestion(entry);
			this->suggestions_.emplace_back(entry);
		}

		[[nodiscard]] std::optional<std::reference_wrapper<suggestion_entry>> find_by_checksum(std::uint64_t checksum) noexcept {
			for (auto& [prefix, entries] : this->available_suggestions_) {
				for (auto& entry : entries) {
					if (entry.checksum == checksum) {
						return std::ref(entry);
					}
				}
			}
			return {};
		}

		[[nodiscard]] bool is_hashed_dvar_input(const std::string& input) const noexcept {
			return input.length() > this->hashed_dvar_prefix.length()
				&& input.compare(0, this->hashed_dvar_prefix.length(), this->hashed_dvar_prefix) == 0
			;
		}

		[[nodiscard]] std::optional<std::uint64_t> parse_hashed_dvar(const std::string& input) const noexcept {
			if (!this->is_hashed_dvar_input(input)) {
				return std::nullopt;
			}

			const auto hex_part = input.substr(this->hashed_dvar_prefix.length());
			if (hex_part.empty()) {
				return std::nullopt;
			}

			try {
				return std::stoull(hex_part, nullptr, 16);
			} catch (...) {
				return std::nullopt;
			}
		}

		[[nodiscard]] std::string format_hash_name(std::uint64_t checksum) const noexcept {
			return std::format("#x3{:X}", checksum);
		}

		[[nodiscard]] bool is_valid_match(const std::string& input, const suggestion_entry& candidate, bool exact) const noexcept {
			switch (candidate.kind) {
			case suggestion_kind::command:
				return this->is_valid_generic_match(input, candidate, exact);
			case suggestion_kind::dvar:
				if (!game_data_provider::find_dvar_by_hash(candidate.checksum)) {
					return false;
				}

				return this->is_valid_generic_match(input, candidate, exact);
			case suggestion_kind::omnvar:
				// return this->is_valid_omnvar_match(input, candidate, exact);
			default:
				return false;
			}
		}

		[[nodiscard]] bool is_valid_generic_match(const std::string& input, const suggestion_entry& candidate, bool exact) const noexcept {
			const auto normalized_name = utils::string::to_lower(candidate.name);
			if (utils::string::match_compare(input, normalized_name, exact)) {
				return true;
			}

			if (candidate.obfuscated_name.has_value()) {
				const auto normalized_obfuscated = utils::string::to_lower(candidate.obfuscated_name.value());
				if (utils::string::match_compare(input, normalized_obfuscated, exact)) {
					return true;
				}
			}

			return false;
		}
	};
}