#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/enums/LocalClientNum_t.hpp"

namespace iw9 {
	inline constexpr int CMD_MAX_NESTING = 8;

	inline bool Com_Keys_IsValidGamePadChar(int key) {
		if (VAL_AS(std::uint32_t, key) > 0x1F) {
			return false;
		}
		long mask = 0xF3FFC07E;
		return _bittest(&mask, key) != 0;
	}

	class CmdArgs {
	public:
		int nesting_;											// 0x0000
		LocalClientNum_t local_client_num_[CMD_MAX_NESTING];	// 0x0004
		int controller_index_[CMD_MAX_NESTING];					// 0x0024
		int argc_[CMD_MAX_NESTING];								// 0x0044
	private:
		char pad_0064[0x0004];									// 0x0064
	public:
		const char** argv_[CMD_MAX_NESTING];					// 0x0068
		char text_pool_[0x4000];								// 0x00A8
		const char* argv_pool_[0x200];							// 0x40A8
		int used_text_pool_[CMD_MAX_NESTING];					// 0x50A8
		int total_used_argv_pool_;								// 0x50C8
		int total_used_text_pool_;								// 0x50CC
	public:
		const char* arg_v(int arg_index) {
			if (arg_index >= this->argc_[this->nesting_]) {
				return "";
			}
			return this->argv_[this->nesting_][arg_index];
		}

		int tokenize_string_internal(const char* text_in, int max_tokens) {
			int token_count = 0;
			int arg_index = 0;

			while (true) {
				while (*text_in) {
					char c = *text_in;

					// whitespace (with special gamepad exception)
					if (((c - 22) & 0xF6) != 0 || c == 23) {
						if ((unsigned char)c <= 0x20 && !Com_Keys_IsValidGamePadChar(c)) {
							++text_in;
							continue;
						}
					}

					// comments
					if (c == '/') {
						if (text_in[1] == '/') {
							return token_count; // stop entirely
						}
						if (text_in[1] == '*') {
							text_in += 2;
							while (*text_in && !(text_in[0] == '*' && text_in[1] == '/')) {
								++text_in;
							}
							if (*text_in) {
								text_in += 2;
							}
							continue;
						}
					}

					break;
				}

				if (!*text_in) {
					return token_count;
				}

				// --- Start new token ---
				++token_count;
				this->argv_[this->nesting_][arg_index++] = &this->text_pool_[this->total_used_text_pool_];

				char c = *text_in;

				// --- Max token handling ---
				if (--max_tokens == 0) {
					while (*text_in) {
						this->text_pool_[this->total_used_text_pool_] = *text_in++;
						this->total_used_text_pool_ = std::min(this->total_used_text_pool_ + 1, 0x3FFF - 1);
					}
					return token_count;
				}

				// --- Quoted string ---
				if (c == '"') {
					++text_in;
					while (*text_in) {
						char ch = *text_in++;
						if (ch == '"') {
							break;
						}
						if (ch == '\\' && *text_in == '"') {
							ch = '"';
							++text_in;
						}
						this->text_pool_[this->total_used_text_pool_] = ch;
						this->total_used_text_pool_ = std::min(this->total_used_text_pool_ + 1, 0x3FFF - 1);
					}
					this->text_pool_[this->total_used_text_pool_] = '\0';
					this->total_used_text_pool_ = std::min(this->total_used_text_pool_ + 1, 0x3FFF - 1);
					if (!*text_in) {
						return token_count;
					}
					continue;
				}

				// --- Normal token ---
				while (*text_in) {
					char ch = *text_in;

					// stop conditions
					if ((((ch - 22) & 0xF6) == 0 && ch != 23) || VAL_AS(std::uint8_t, ch) > 0x20 || Com_Keys_IsValidGamePadChar(ch)) {
						if (ch == '/' && (text_in[1] == '/' || text_in[1] == '*')) {
							break;
						}
					}
					else {
						break;
					}
					this->text_pool_[this->total_used_text_pool_] = ch;
					this->total_used_text_pool_ = std::min(this->total_used_text_pool_ + 1, 0x3FFF - 1);
					++text_in;
				}

				this->text_pool_[this->total_used_text_pool_] = '\0';
				this->total_used_text_pool_ = std::min(this->total_used_text_pool_ + 1, 0x3FFF - 1);
			}
		}

		void tokenize_string_kernel(const char* text_in, int max_tokens) {
			++this->nesting_;
			this->used_text_pool_[this->nesting_] = -this->total_used_text_pool_;
			this->local_client_num_[this->nesting_] = iw9::LocalClientNum_t::LOCAL_CLIENT_INVALID;
			this->controller_index_[this->nesting_] = 0;
			this->argv_[this->nesting_] = &this->argv_pool_[this->total_used_argv_pool_];
			this->argc_[this->nesting_] = this->tokenize_string_internal(text_in, max_tokens);
			this->total_used_argv_pool_ += this->argc_[this->nesting_];
			this->used_text_pool_[this->nesting_] += this->total_used_text_pool_;
		}

		void end_tokenized_string_kernel() {
			this->total_used_argv_pool_ -= this->argc_[this->nesting_];
			this->total_used_text_pool_ -= this->used_text_pool_[this->nesting_];
			--this->nesting_;
		}
	};
	ENGINE_ASSERT_SZ(CmdArgs, 0x50D0);
}
