#include "common.hpp"
#include "component/console/console.hpp"
#include "component/key_catchers.hpp"
#include "component/command.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>

#include "resource.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

namespace console::ingame {
	struct component final : generic_component {
		void post_load() override {
			CreateThread(nullptr, 0, [](PVOID) -> DWORD {
				auto dvar_list_json_str = utils::nt::load_resource(DVAR_LIST_JSON);

				rapidjson::Document doc;
				doc.Parse(dvar_list_json_str.c_str());

				if (doc.IsArray() && !doc.Empty()) {
					using namespace std::chrono;

					const auto timepoint = steady_clock::now();

					std::size_t dvar_count = 0;
					for (const auto& obj : doc.GetArray()) {
						dvar_count++;

						auto name = obj.HasMember("name") && obj["name"].IsString() ? obj["name"].GetString() : ""s;
						auto desc = [&obj] -> std::optional<std::string> {
							if (!obj.HasMember("desc") || !obj["desc"].IsString()) {
								return {};
							}

							return obj["desc"].GetString();
						}();

						if (name.size() > 0) {
							dvars::insert_dvar_info(name, desc.value_or(""));
							game_console.register_suggestion({
								.kind = detail::suggestion_kind::dvar,
								.name = name,
								.description = desc,
								.checksum = game::hash_dvar(name)
							});
						}
					}

					const auto elapsed_time = duration_cast<milliseconds>(steady_clock::now() - timepoint).count();
					LOG("Component/Console/InGame", INFO, "Initialized {} dvar{} in {}ms.", dvar_count, dvar_count == 1 ? "" : "s", elapsed_time);
				}
				else {
					LOG("Component/Console/InGame", WARN, "Failed to load Dvar list.");
				}

				return 0;
			}, nullptr, 0, nullptr);
		}

		void post_unpack() override {
			scheduler::loop<scheduler::pipeline::client>([] {
				return game_console.draw_console();
			});

			scheduler::once<scheduler::pipeline::client>([] {
				key_catchers::add_key_event([](int client_num, s4::keyNum_t key_event, bool down) -> bool {
					return game_console.key_catcher_key_event_hook(client_num, ENUM_UNDER(key_event), down);
				});

				key_catchers::add_char_event([](int client_num, s4::keyNum_t key_event) -> bool {
					return game_console.key_catcher_char_event_hook(client_num, ENUM_UNDER(key_event));
				});
			});

			command::add("console_clear", []{
				return game_console.clear_console();
			});
		}
	};
}

REGISTER_COMPONENT(console::ingame::component)
