#include "common_core.hpp"
#include "lua_decompiler/ast/function.hpp"

#include "lua_decompiler/ast/local.hpp"
#include "lua_decompiler/ast/slot_scope.hpp"
#include "lua_decompiler/bytecode/prototype.hpp"

namespace lua_decompiler {
	ast::slot_scope** ast::function::slot_scope_collector::new_slot_scope() {
		return &this->slot_scopes_.emplace_back(new slot_scope)->slot_scope_;
	}

	std::uint32_t ast::function::slot_scope_collector::add_upvalue_info(const std::uint32_t& id, const upvalue_info::type& type_in) {
		for (auto x = 0; x < this->upvalue_infos_.size(); x++) {
			if (id > this->upvalue_infos_[x].id_) {
				continue;
			}
			this->upvalue_infos_.emplace(this->upvalue_infos_.begin() + x);
			this->upvalue_infos_[x].type_ = type_in;
			this->upvalue_infos_[x].id_ = id;
			return x;
		}
		this->upvalue_infos_.emplace_back();
		this->upvalue_infos_.back().type_ = type_in;
		this->upvalue_infos_.back().id_ = id;
		return this->upvalue_infos_.size() - 1;
	}

	void ast::function::slot_scope_collector::add_upvalues(const std::uint32_t& id, std::vector<std::uint8_t>& upvalues) {
		this->upvalue_infos_[this->add_upvalue_info(id, upvalue_info::type::t_upvalues)].upvalues_ = upvalues;
	}

	void ast::function::slot_scope_collector::add_jump(const std::uint32_t& id, const std::uint32_t& target) {
		this->upvalue_infos_[this->add_upvalue_info(id, upvalue_info::type::t_jump)].target_ = target;
	}

	void ast::function::slot_scope_collector::add_upvalue_close(const std::uint32_t& id, const std::uint32_t& target, const std::uint8_t& base_slot) {
		const auto index = this->add_upvalue_info(id, upvalue_info::type::t_upvalue_close);
		this->upvalue_infos_[index].target_ = target;
		this->upvalue_infos_[index].base_slot_ = base_slot;
	}

	void ast::function::slot_scope_collector::add_loop(const std::uint32_t& id, const std::uint32_t& target) {
		auto index = this->add_upvalue_info(id, upvalue_info::type::t_jump);
		this->upvalue_infos_[index].target_ = target;
		do {
			index++;
		} while (index < this->upvalue_infos_.size() && this->upvalue_infos_[index].id_ < target);
		this->upvalue_infos_.emplace(this->upvalue_infos_.begin() + index);
		this->upvalue_infos_[index].id_ = target - 1;
		this->upvalue_infos_[index].target_ = id;
	}

	void ast::function::slot_scope_collector::add_upvalue_scope(const std::uint8_t& slot, const std::uint32_t& min_scope_begin,
		const std::uint32_t& min_scope_end)
	{
		for (auto x = 0; x < this->upvalue_scopes_.size(); x++) {
			if (min_scope_end > this->upvalue_scopes_[x].min_scope_end_) {
				continue;
			}
			this->upvalue_scopes_.emplace(this->upvalue_scopes_.begin() + x);
			this->upvalue_scopes_[x].slot_ = slot;
			this->upvalue_scopes_[x].min_scope_begin_ = min_scope_begin;
			this->upvalue_scopes_[x].min_scope_end_ = min_scope_end;
			return;
		}
		this->upvalue_scopes_.emplace_back();
		this->upvalue_scopes_.back().slot_ = slot;
		this->upvalue_scopes_.back().min_scope_begin_ = min_scope_begin;
		this->upvalue_scopes_.back().min_scope_end_ = min_scope_end;
	}

	void ast::function::slot_scope_collector::build_upvalue_scopes() {
		std::uint32_t index, min_scope_begin, min_scope_end;
		for (auto i = this->upvalue_infos_.size(); i--;) {
			if (this->upvalue_infos_[i].type_ != upvalue_info::type::t_upvalues) {
				continue;
			}
			for (auto j = this->upvalue_infos_[i].upvalues_.size(); j--;) {
				index = i;
				min_scope_begin = this->upvalue_infos_[i].id_;
				min_scope_end = this->upvalue_infos_[i].id_;
				for (auto k = i + 1; k < this->upvalue_infos_.size(); k++) {
					switch (this->upvalue_infos_[k].type_) {
					case upvalue_info::type::t_upvalue_close:
						if (this->upvalue_infos_[i].upvalues_[j] >= this->upvalue_infos_[k].base_slot_) {
							if (min_scope_end > this->upvalue_infos_[k].id_) {
								continue;
							}
							min_scope_end = this->upvalue_infos_[k].id_;
							break;
						}
					case upvalue_info::type::t_jump:
						if (min_scope_end < this->upvalue_infos_[k].target_) {
							min_scope_end = this->upvalue_infos_[k].target_;
						}
						else if (min_scope_begin >= this->upvalue_infos_[k].target_) {
							min_scope_begin = this->upvalue_infos_[k].target_ - 1;
							for (auto l = index; l-- && min_scope_begin < this->upvalue_infos_[l].id_; index = l) {
								switch (this->upvalue_infos_[l].type_) {
								case upvalue_info::type::t_upvalue_close:
									if (this->upvalue_infos_[i].upvalues_[j] >= this->upvalue_infos_[l].base_slot_) {
										continue;
									}
								case upvalue_info::type::t_jump:
									if (min_scope_end < this->upvalue_infos_[l].target_) {
										min_scope_end = this->upvalue_infos_[l].target_;
									}
									else if (min_scope_begin >= this->upvalue_infos_[l].target_) {
										min_scope_begin = this->upvalue_infos_[l].target_ - 1;
									}
								default:
									continue;
								}
							}
						}
					default:
						continue;
					}
					break;
				}
				this->add_upvalue_scope(this->upvalue_infos_[i].upvalues_[j], min_scope_begin, min_scope_end);
			}
		}
	}

	void ast::function::slot_scope_collector::begin_scope(const std::uint8_t& slot, const std::uint32_t& id) {
		if (this->slot_infos_[slot].active_slot_scope_) {
			return;
		}
		this->slot_infos_[slot].slot_scopes_.emplace_back(this->new_slot_scope());
		this->slot_infos_[slot].active_slot_scope_ = this->slot_infos_[slot].slot_scopes_.back();
		(*this->slot_infos_[slot].active_slot_scope_)->scope_end_ = id;
	}

	void ast::function::slot_scope_collector::begin_upvalue_scopes(const std::uint32_t& id) {
		while (this->upvalue_scopes_.size() && this->upvalue_scopes_.back().min_scope_end_ >= id) {
			if (this->upvalue_scopes_.back().min_scope_begin_ <= id) {
				this->begin_scope(this->upvalue_scopes_.back().slot_, id);
				this->extend_scope(this->upvalue_scopes_.back().slot_, this->upvalue_scopes_.back().min_scope_begin_);
			}
			this->upvalue_scopes_.pop_back();
		}
	}

	void ast::function::slot_scope_collector::add_to_scope(const std::uint8_t& slot, slot_scope**& slot_scope_in, const std::uint32_t& id) {
		this->begin_scope(slot, id);
		slot_scope_in = this->slot_infos_[slot].active_slot_scope_;
		(*this->slot_infos_[slot].active_slot_scope_)->usages_++;
	}

	void ast::function::slot_scope_collector::close_scope(const std::uint8_t& slot, slot_scope**& slot_scope_in, const std::uint32_t& id) {
		if (this->slot_infos_[slot].is_parameter_ || (this->slot_infos_[slot].min_scope_begin_ != INVALID_ID
			&& this->slot_infos_[slot].min_scope_begin_ < id))
		{
			return this->add_to_scope(slot, slot_scope_in, id);
		}
		this->begin_scope(slot, id);
		slot_scope_in = this->slot_infos_[slot].active_slot_scope_;
		(*this->slot_infos_[slot].active_slot_scope_)->scope_begin_ = id;
		this->slot_infos_[slot].active_slot_scope_ = nullptr;
		this->slot_infos_[slot].min_scope_begin_ = INVALID_ID;
	}

	void ast::function::slot_scope_collector::extend_scope(const std::uint8_t& slot, const std::uint32_t& id) {
		if (!this->slot_infos_[slot].is_parameter_ && this->slot_infos_[slot].active_slot_scope_
			&& (this->slot_infos_[slot].min_scope_begin_ == INVALID_ID || this->slot_infos_[slot].min_scope_begin_ > id))
		{
			this->slot_infos_[slot].min_scope_begin_ = id;
		}
	}

	void ast::function::slot_scope_collector::extend_scopes(const std::uint32_t& id) {
		for (auto x = this->slot_infos_.size(); x--;) {
			this->extend_scope(x, id);
		}
	}

	void ast::function::slot_scope_collector::merge_scopes(const std::uint32_t& id) {
		for (auto x = this->slot_infos_.size(); x--;) {
			if (this->slot_infos_[x].is_parameter_ || !this->slot_infos_[x].active_slot_scope_) {
				continue;
			}

			for (auto y = this->slot_infos_[x].slot_scopes_.size() - 1; y-- && (*this->slot_infos_[x].slot_scopes_[y])->scope_begin_ <= id;) {
				(*this->slot_infos_[x].active_slot_scope_)->scope_end_ = (*this->slot_infos_[x].slot_scopes_[y])->scope_end_;
				(*this->slot_infos_[x].active_slot_scope_)->usages_ += (*this->slot_infos_[x].slot_scopes_[y])->usages_ + 1;

				for (std::uint32_t k = (*this->slot_infos_[x].slot_scopes_[y])->merged_scopes_.size(); k--;) {
					*(*this->slot_infos_[x].slot_scopes_[y])->merged_scopes_[k] = *this->slot_infos_[x].active_slot_scope_;
				}

				(*this->slot_infos_[x].active_slot_scope_)->merged_scopes_.insert((*this->slot_infos_[x].active_slot_scope_)->merged_scopes_.end(),
					(*this->slot_infos_[x].slot_scopes_[y])->merged_scopes_.begin(), (*this->slot_infos_[x].slot_scopes_[y])->merged_scopes_.end());
				(*this->slot_infos_[x].active_slot_scope_)->merged_scopes_.emplace_back(&(*this->slot_infos_[x].slot_scopes_[y])->slot_scope_);
				*this->slot_infos_[x].slot_scopes_[y] = *this->slot_infos_[x].active_slot_scope_;
				this->slot_infos_[x].slot_scopes_.erase(this->slot_infos_[x].slot_scopes_.begin() + y);
			}

			if ((*this->slot_infos_[x].active_slot_scope_)->scope_end_ < id) {
				(*this->slot_infos_[x].active_slot_scope_)->scope_end_ = id;
			}
		}
	}

	bool ast::function::slot_scope_collector::assert_scopes_closed() {
		for (auto x = this->slot_infos_.size(); x--;) {
			if (!this->slot_infos_[x].is_parameter_ && this->slot_infos_[x].active_slot_scope_) {
				return false;
			}
		}
		return true;
	}

	void ast::function::slot_scope_collector::remove_scope(const std::uint8_t& slot, slot_scope** const& slot_scope_in) {
		for (std::uint32_t i = this->slot_infos_[slot].slot_scopes_.size(); i--;) {
			if (this->slot_infos_[slot].slot_scopes_[i] != slot_scope_in) {
				continue;
			}
			this->slot_infos_[slot].slot_scopes_.erase(this->slot_infos_[slot].slot_scopes_.begin() + i);
			return;
		}
	}

	ast::function::function(const bytecode::prototype& prototype, const std::uint32_t& level, const bool& ignore_debug_info)
		: prototype_(prototype)
		, is_variadic_(prototype.header_.flags_ & bytecode::proto_vararg)
		, has_debug_info_(!ignore_debug_info && prototype_.header_.has_debug_info_)
		, level_(level)
	{
		this->slot_scope_collector_.slot_infos_.resize(prototype.header_.frame_size_);
		for (auto i = prototype.header_.parameters_; i--;) {
			this->slot_scope_collector_.slot_infos_[i].is_parameter_ = true;
			this->slot_scope_collector_.slot_infos_[i].active_slot_scope_ = this->slot_scope_collector_.new_slot_scope();
		}
		this->slot_scope_collector_.previous_id_ = prototype.instructions_.size();
	}

	ast::function::~function() {
		for (auto x = this->slot_scope_collector_.slot_scopes_.size(); x--;) {
			delete this->slot_scope_collector_.slot_scopes_[x];
		}
	}

	const bytecode::constant& ast::function::get_constant(const std::uint16_t& index) const {
		return this->prototype_.constants_[this->prototype_.constants_.size() - 1 - index];
	}

	const bytecode::number_constant& ast::function::get_number_constant(const std::uint16_t& index) const {
		return this->prototype_.number_constants_[index];
	}

	void ast::function::add_jump(const std::uint32_t& id, const std::uint32_t& target) {
		for (auto x = 0; x < this->labels_.size(); x++) {
			if (target > this->labels_[x].target_) {
				continue;
			}
			if (target != this->labels_[x].target_) {
				this->labels_.emplace(this->labels_.begin() + x);
				this->labels_[x].target_ = target;
			}
			for (auto y = 0; y < this->labels_[x].jump_ids_.size(); y++) {
				if (id > this->labels_[x].jump_ids_[y]) {
					continue;
				}
				if (id < this->labels_[x].jump_ids_[y]) {
					this->labels_[x].jump_ids_.emplace(this->labels_[x].jump_ids_.begin() + y, id);
				}
				return;
			}
			this->labels_[x].jump_ids_.emplace_back(id);
			return;
		}
		this->labels_.emplace_back();
		this->labels_.back().target_ = target;
		this->labels_.back().jump_ids_.emplace_back(id);
	}

	void ast::function::remove_jump(const std::uint32_t& id, const std::uint32_t& target) {
		for (auto x = this->labels_.size(); x--;) {
			if (this->labels_[x].target_ != target) {
				continue;
			}
			for (auto y = this->labels_[x].jump_ids_.size(); y--;) {
				if (this->labels_[x].jump_ids_[y] != id) {
					continue;
				}
				this->labels_[x].jump_ids_.erase(this->labels_[x].jump_ids_.begin() + y);
				return;
			}
		}
	}

	std::uint32_t ast::function::get_label_from_id(const std::uint32_t& id) {
		for (std::uint32_t x = this->labels_.size(); x-- && this->labels_[x].target_ >= id;) {
			if (this->labels_[x].target_ != id) {
				continue;
			}
			return x;
		}
		return INVALID_ID;
	}

	bool ast::function::is_valid_label(const std::uint32_t& label) {
		return label != INVALID_ID && this->labels_[label].jump_ids_.size();
	}

	std::uint32_t ast::function::get_scope_begin_from_label(const std::uint32_t& label, const std::uint32_t& scope_end) {
		auto scope_begin = this->labels_[label].target_ - 1;
		for (auto x = label; x < this->labels_.size() && this->labels_[x].target_ <= scope_end; x++) {
			if (this->labels_[x].jump_ids_.size() && this->labels_[x].jump_ids_.front() <= scope_begin) {
				scope_begin = this->labels_[x].jump_ids_.front() - 1;
			}
		}
		return scope_begin;
	}

	std::uint32_t ast::function::get_scope_end_from_label(const std::uint32_t& label) {
		auto scope_end = this->labels_[label].target_;
		for (auto x = label; x < this->labels_.size() && this->labels_[x].target_ <= scope_end; x++) {
			if (this->labels_[x].jump_ids_.size() && this->labels_[x].jump_ids_.back() > scope_end) {
				scope_end = this->labels_[x].jump_ids_.back();
			}
		}
		return scope_end;
	}

	bool ast::function::is_valid_block_range(const std::uint32_t& block_begin, const std::uint32_t& block_end, const bool& ignore_front_label) {
		for (auto x = this->labels_.size(); x-- && this->labels_[x].target_ >= block_begin;) {
			if (this->labels_[x].target_ <= block_end && this->labels_[x].jump_ids_.size()
				&& ((this->labels_[x].jump_ids_.front() < block_begin && (this->labels_[x].target_ != block_begin || !ignore_front_label))
				|| this->labels_[x].jump_ids_.back() > block_end))
			{
				return false;
			}
		}
		return true;
	}
}
