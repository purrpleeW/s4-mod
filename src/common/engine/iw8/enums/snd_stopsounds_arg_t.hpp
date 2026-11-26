#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class snd_stopsounds_arg_t {
		SND_STOP_ALL = 0,
		SND_KEEP_REVERB = 1 << 0,
		SND_KEEP_MUSIC = 1 << 1,
		SND_KEEP_AMBIENT = 1 << 2,
		SND_KEEP_MUSIC_AND_AMBIENT = SND_KEEP_AMBIENT | SND_KEEP_MUSIC,
		SND_STOP_STREAMED = 1 << 3,
		SND_KEEP_CHANNEL_VOLUMES = 1 << 4
	};
	DECLARE_BITWISE_OPS(snd_stopsounds_arg_t);
}
