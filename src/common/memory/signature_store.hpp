#pragma once
#include "common_core.hpp"

#include "ci/ci.hpp"
#include "identification/game.hpp"
#include "logger/logger.hpp"
#include "memory/memory.hpp"
#include "memory/scanned_result.hpp"
#include "nlohmann/json.hpp"
#include "utils/concurrency.hpp"
#include "utils/cryptography.hpp"
#include "utils/nt.hpp"
#include "utils/properties.hpp"

#include <filesystem>

namespace memory {
	class signature_store {
	public:
		using res = scanned_result<void>;
		static inline constexpr auto sig_cache_version = 1;

		class signature {
		public:
			signature(const utils::nt::library& library, const std::string& name, void** pointer, const std::string& pattern, std::function<res(res)> mod,
					bool dynamic = false)
				: library_(library)
				, name_(name)
				, pattern_(pattern)
				, pointer_(pointer)
				, is_vec_(false)
				, mod_(std::move(mod))
				, dynamic_(dynamic)
			{}

			signature(const utils::nt::library& library, const std::string& name, std::vector<void*>* pointer, const std::string& pattern,
					std::function<res(res)> mod, bool dynamic = false)
				: library_(library)
				, name_(name)
				, pattern_(pattern)
				, pointer_vec_(pointer)
				, is_vec_(true)
				, mod_(std::move(mod))
				, dynamic_(dynamic)
			{}

			void apply_value(std::uintptr_t value) const {
				if (*this->pointer_ != nullptr) {
					return;
				}

				res sc_temp = scanned_result<void>(value);
				if (sc_temp.as<void*>() != nullptr) {
					*this->pointer_ = this->mod_(sc_temp).as<void*>();
				}
			}

			void apply_values(std::vector<std::uintptr_t> values) const {
				if (*this->pointer_ != nullptr) {
					return;
				}

				std::vector<res> sc_temp_vec = {};
				for (auto& value : values) {
					sc_temp_vec.push_back(scanned_result<void>(value));
				}

				if (!sc_temp_vec.empty()) {
					for (auto& sc_temp : sc_temp_vec) {
						this->pointer_vec_->push_back(this->mod_(sc_temp).as<void*>());
					}
				}
			}

			std::vector<std::uintptr_t> scan() const {
				if (*this->pointer_ != nullptr) {
					return {};
				}

				if (this->is_vec_) {
					std::vector<res> sc_temp_vec = vectored_sig_scan(this->library_, this->pattern_, std::numeric_limits<std::size_t>::max(), this->name_);
					if (!sc_temp_vec.empty()) {
						for (auto& sc_temp : sc_temp_vec) {
							this->pointer_vec_->push_back(this->mod_(sc_temp).as<void*>());
						}
					}

					std::vector<std::uintptr_t> values = {};
					for (auto& sc : sc_temp_vec) {
						values.push_back(sc.as<std::uintptr_t>());
					}

					return values;
				}

				res sc_temp = sig_scan(this->library_, this->pattern_, this->name_);
				if (sc_temp.as<void*>() != nullptr) {
					*this->pointer_ = this->mod_(sc_temp).as<void*>();
					return { sc_temp.as<std::uintptr_t>() };
				}

				return {};
			}

			utils::nt::library library_;
			std::string name_;
			std::string pattern_;
			union {
				void** pointer_;
				std::vector<void*>* pointer_vec_;
			};
			bool is_vec_;
			std::function<res(res)> mod_;
			bool dynamic_;
		};

		signature_store(const utils::nt::library& library)
			: library_(library)
		{}

		signature_store()
			: signature_store(utils::nt::library())
		{}

		void add(const std::string& name, void** pointer, const std::string& pattern, std::function<res(res)> mod) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, std::move(mod)));
		}

		void add(const std::string& name, void** pointer, const std::string& pattern) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, [](res r) { return r; }));
		}

		void add(const std::string& name, std::vector<void*>* pointer, const std::string& pattern, std::function<res(res)> mod) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, std::move(mod)));
		}

		void add(const std::string& name, std::vector<void*>* pointer, const std::string& pattern) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, [](res r) { return r; }));
		}

		void add_dyn(const std::string& name, void** pointer, const std::string& pattern, std::function<res(res)> mod) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, std::move(mod), true));
		}

		void add_dyn(const std::string& name, void** pointer, const std::string& pattern) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, [](res r) { return r; }, true));
		}

		void add_dyn(const std::string& name, std::vector<void*>* pointer, const std::string& pattern, std::function<res(res)> mod) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, std::move(mod), true));
		}

		void add_dyn(const std::string& name, std::vector<void*>* pointer, const std::string& pattern) {
			this->signatures_.push_back(signature(this->library_, name, pointer, pattern, [](res r) { return r; }, true));
		}

		void scan_all(bool run_sync = false, bool cache_signatures = true) {
			ci::record_test_state(ci::Started, ci::Signature, "common/memory/signature_store -> scan_all()", "started signature scan");
			static std::filesystem::path cache_folder = utils::properties::get_game_mod_path() / "sigcache";
			static std::uint32_t signature_found_count = 0;
			static std::uint32_t signature_count_total = 0;

			std::string cache_file = this->library_.get_name() + ".json";
			std::filesystem::path cache_file_path = cache_folder / cache_file;

			nlohmann::json cached_sig_data{};
			auto module_hash = identification::game::get_library_xxh32_checksum(this->library_);

			if (cache_signatures) {
				if (!(std::filesystem::exists(cache_folder) && std::filesystem::is_directory(cache_folder))) {
					std::filesystem::create_directory(cache_folder);
				}

				if (std::filesystem::exists(cache_file_path) && std::filesystem::is_regular_file(cache_file_path)) {
					std::ifstream file(cache_file_path);
					if (file) {
						file >> cached_sig_data;
						file.close();
					}

					bool needs_regen = false;
					if (!cached_sig_data.contains("_version_mh") || !cached_sig_data["_version_mh"].is_number()
						|| cached_sig_data["_version_mh"].get<std::uint32_t>() != module_hash)
					{
						needs_regen = true;
					}
					else if (!cached_sig_data.contains("_version_sc") || !cached_sig_data["_version_sc"].is_number()
						|| cached_sig_data["_version_sc"].get<int>() != sig_cache_version)
					{
						needs_regen = true;
					}

					if (needs_regen) {
						LOG("Scanner", WARN, "Sigcache version mismatch, regenerating cache...");
						cached_sig_data = nlohmann::json{};
						cached_sig_data["_version_mh"] = module_hash;
						cached_sig_data["_version_sc"] = sig_cache_version;
					}
				}
				else {
					cached_sig_data["_version_mh"] = module_hash;
					cached_sig_data["_version_sc"] = sig_cache_version;
					std::ofstream file(cache_file_path);
					if (file) {
						file << cached_sig_data.dump(1, '\t');
						file.close();
					}
				}
			}

			auto handle_scan = [&](signature& sig) {
				std::uintptr_t image_base = PTR_AS(std::uintptr_t, sig.library_.get_ptr());
				auto json_key = std::format("{:X}", utils::cryptography::jenkins_one_at_a_time::compute(sig.pattern_));

				if (!sig.dynamic_ && cache_signatures && cached_sig_data.contains(json_key)) {
					if (cached_sig_data[json_key].is_number()) {
						auto addr = image_base + VAL_AS(std::uintptr_t, cached_sig_data[json_key].get<std::int64_t>());
						if (sig.library_.is_address_in_accessible_range(addr) && memory::matches(addr, sig.pattern_)) {
							sig.apply_value(addr);
							signature_found_count++;
							signature_count_total++;
							return;
						}
					}
					if (cached_sig_data[json_key].is_array()) {
						std::vector<std::uintptr_t> values = {};
						for (auto& elem : cached_sig_data[json_key]) {
							if (!elem.is_number()) {
								continue;
							}
							auto addr = image_base + VAL_AS(std::uintptr_t, elem.get<std::int64_t>());
							if (sig.library_.is_address_in_accessible_range(addr) && memory::matches(addr, sig.pattern_)) {
								values.push_back(addr);
							}
						}
						sig.apply_values(values);
						signature_found_count++;
						signature_count_total++;
						return;
					}
				}

				std::vector<std::uintptr_t> values_to_save = sig.scan();

				signature_count_total++;
				if (!values_to_save.empty()) {
					const std::uintptr_t offset_adr = values_to_save.at(0) - image_base;
					ci::record_test_state(ci::InProgress, ci::Signature, sig.name_, std::format("signature hit -> 0x{:X}", offset_adr));
					signature_found_count++;
				}
				else {
					ci::record_test_state(ci::Failure, ci::Signature, sig.name_, std::format("invalid signature, aob: {}", sig.pattern_));
				}

				if (!sig.dynamic_ && cache_signatures && !values_to_save.empty()) {
					if (sig.is_vec_) {
						for (auto& value : values_to_save) {
							cached_sig_data[json_key].push_back(VAL_AS(std::int64_t, value - image_base));
						}
					}
					else {
						cached_sig_data[json_key] = VAL_AS(std::int64_t, values_to_save.at(0) - image_base);
					}
				}
			};

			if (run_sync) {
				for (auto& sig : this->signatures_) {
					handle_scan(sig);
				}
			}
			else {
				utils::concurrency::async_for_each<signature>(this->signatures_, [=](auto& sig) {
					handle_scan(sig);
				});
			}

			if (cache_signatures) {
				cached_sig_data["_version_mh"] = module_hash;
				cached_sig_data["_version_sc"] = sig_cache_version;
				std::ofstream file(cache_file_path);
				if (file) {
					file << cached_sig_data.dump(1, '\t');
					file.close();
				}
			}

			double error_rate = signature_count_total == 0 ? 0.0 : (1.0 - VAL_AS(double, signature_found_count) / VAL_AS(double, signature_count_total)) * 100.0;

			ci::record_test_state(ci::Ended, ci::Signature, "common/memory/signature_store -> scan_all()", std::format("{}/{} (error rate: {:.2f}%)", signature_found_count, signature_count_total, error_rate));
			LOG("Scanner", INFO, "Found {} out of {} signatures", signature_found_count, signature_count_total);
		}

		static nlohmann::json dump_make_entry(void* address) {
			nlohmann::json j;
			if (!address) {
				j["module"] = nullptr;
				j["offset"] = nullptr;
				return j;
			}

			auto lib = utils::nt::library::get_by_address(address);
			if (lib.is_valid()) {
				j["module"] = lib.get_name();
				j["offset"] = PTR_AS(std::uintptr_t, address) - PTR_AS(std::uintptr_t, lib.get_ptr());
			}
			else {
				j["module"] = nullptr;
				j["offset"] = PTR_AS(std::uintptr_t, address);
			}

			return j;
		}

		void dump() {
			nlohmann::json result = nlohmann::json::object();
			auto signatures = this->signatures_;

			std::sort(signatures.begin(), signatures.end(), [](const signature& a, const signature& b) {
				return a.name_ < b.name_;
			});

			for (const auto& inst : signatures) {
				if (!inst.is_vec_) {
					void* value = inst.pointer_ ? *inst.pointer_ : nullptr;
					result[inst.name_] = dump_make_entry(value);
				}
				else {
					nlohmann::json arr = nlohmann::json::array();

					if (inst.pointer_vec_) {
						for (void* value : *inst.pointer_vec_) {
							arr.push_back(dump_make_entry(value));
						}
					}

					std::sort(arr.begin(), arr.end(), [](const nlohmann::json& a, const nlohmann::json& b) {
						if (a["module"] != b["module"]) {
							return a["module"] < b["module"];
						}
						return a["offset"] < b["offset"];
					});

					result[inst.name_] = arr;
				}
			}

			std::ofstream(std::format("{}.sigdump.json", this->library_.get_name())) << result.dump(1, '\t');
		}

		std::vector<signature> get_signatures() {
			return this->signatures_;
		}
	private:
		utils::nt::library library_;
		std::vector<signature> signatures_{};
	};
}