#ifndef COLURE_SOUND
#define COLURE_SOUND

enum {
	SFXR_PICKUP,
	SFXR_LASER,
	SFXR_EXPLOSION,
	SFXR_POWERUP,
	SFXR_HIT,
	SFXR_JUMP,
	SFXR_BLIP
};

void sound_init(void);
void sound_play(int type);

#endif