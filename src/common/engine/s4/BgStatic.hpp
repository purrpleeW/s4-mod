#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/vec3_t.hpp"
#include "engine/s4/enums/CharacterModelType.hpp"

namespace s4 {
	class BgStatic {
	public:
		virtual ~BgStatic() {}
		virtual BgAnimStatic* GetAnimStatics() const { return nullptr; }
		virtual BgAnimStatic* GetAnimStatics() { return nullptr; }
		virtual bool HasCharacterInfo(const int ent_num) { return false; }
		virtual characterAnimState_t* GetCharacterAnimState(const int ent_num) const { return nullptr; }
		virtual characterAnimState_t* GetCharacterAnimState(const int ent_num) { return nullptr; }
		virtual BgPlayerEvents* GetPlayerEvents() { return nullptr; }
		virtual XModel* GetXModel(const char* name) { return nullptr; }
		virtual XModel* GetAttachXModel(const characterInfo_t* ci, const int attach_model_index, const bool is_reference_model) { return nullptr; }
		virtual DObj* CreateDObj(DObjModel* dobj_models, std::uint16_t num_models, XAnimTree* tree, int handle, int other_client_num,
			bool requires_ik_pre_cache) { return nullptr; }
		virtual void SafeDObjFree(int handle) {}
		virtual bool WeaponModelLoadStatusMaybeDirtyThisFrame() { return false; }
		virtual void WeaponModelLoadStatusMaybeDirtyThisFrameVerification(const characterInfo_t* ci) {}
		virtual bool IsWeaponWorldModelLoaded(const Weapon* r_weapon) { return false; }
		virtual std::uint16_t AttachWeapon(DObjModel* dobj_models, std::uint16_t num_models, const characterInfo_t* ci, const bool use_reference_models,
			std::uint32_t* in_out_first_weapon_model_idx, std::uint32_t* in_out_first_weapon_bone_idx, int* held_weapon_model_idx_1,
			int* held_weapon_model_idx_2, int* stowed_weapon_model_idx, CharacterModelType(*out_model_types)[32]) { return 0; }
		virtual std::uint16_t BodyModel(DObjModel* dobj_models, std::uint16_t num_models, const entityState_t* es, const characterInfo_t* ci,
			const bool is_reference_model) { return 0; }
		virtual void CreateWeaponPhysics(const DObj* obj, const Weapon* held_weapon) {}
		virtual XModel* GetLeftHandIKTargetModel() { return nullptr; }
		virtual XModel* GetRightHandIKTargetModel() { return nullptr; }
		virtual void GetAngles(int end_index, vec3_t* out_angles) {}
		virtual bool IsClient() { return false; }
		virtual int BuildReferenceCharacterModels(const int ent_num, const int attach_ignore_collision, DObjModel** out_dobj_models,
			CharacterModelType(*out_model_types)[32]) { return 0; }
		virtual int BuildReferenceCorpseModels(const int ent_num, const int attach_ignore_collision, DObjModel** out_dobj_models,
			CharacterModelType(*out_model_types)[32]) { return 0; }
		virtual void GetPlayerWorldIkStatus(const int ent_num, bool(*out_world_ik_enabled)[2]) {}
		virtual void OverridePlayerHideParts(const int ent_num, const DObjPartBits* hide_part_bits) {}
		virtual bool UsingLowLodClientEntityEvents() { return false; }
	};
	ENGINE_ASSERT_SZ(BgStatic, 0x0008);
}
