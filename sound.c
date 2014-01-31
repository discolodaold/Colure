#include "sound.h"

#include "sfxr.h"
#include "list.h"

#include <sdl/SDL.h>

static struct list_head head_active;
static struct list_head head_free;

typedef struct {
	struct list_head list;
	sfxr_t sfxr;
} sound_t;

void _callback(void *up, Uint8 *stream, int len) {
	float *samples;
	sound_t *sound, *n;
	int l = len/2;

	samples = (float *)_alloca(sizeof(float) * l);
	memset(samples, 0, sizeof(float) * l);

	list_for_each_entry_safe(sound, n, sound_t, &head_active, list) {
		sfxr_synth(&sound->sfxr, l, samples);
		if(sound->sfxr.playing_sample == 0) {
		//	list_move(&sound->list, &head_free);
		}
	}

	while(l--) {
		float f = samples[l];
		if (f < -1.0) f = -1.0;
		if (f > 1.0) f = 1.0;
		((Sint16 *)stream)[l] = (Sint16)(f * 32767/8);
	}
}

void sound_init(void) {
	SDL_AudioSpec desired;

	desired.freq = 44100;
	desired.format = AUDIO_S16SYS;
	desired.channels = 1;
	desired.samples = 512;
	desired.callback = _callback;
	desired.userdata = NULL;

	if(SDL_OpenAudio(&desired, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	
	INIT_LIST_HEAD(&head_active);
	INIT_LIST_HEAD(&head_free);

	SDL_PauseAudio(0);
}

void sound_play(int type) {
	sfxr_t sfxr;
	sound_t *result;

	SDL_LockAudio();

	if(list_empty(&head_free)) {
		result = (sound_t *)malloc(sizeof(*result));
		list_add_tail(&(result->list), &head_active);
	} else {
		result = list_entry(head_free.next, sound_t, list);
		list_move_tail(&(result->list), &head_active);
	}

	memset(&result->sfxr, 0, sizeof(result->sfxr));

	switch(type) {
	case SFXR_LASER:
		sfxr_laser(&result->sfxr);
		break;
	case SFXR_EXPLOSION:
		sfxr_explosion(&result->sfxr);
		break;
	case SFXR_POWERUP:
		sfxr_powerup(&result->sfxr);
		break;
	case SFXR_HIT:
		sfxr_hit(&result->sfxr);
		break;
	case SFXR_JUMP:
		sfxr_jump(&result->sfxr);
		break;
	case SFXR_BLIP:
		sfxr_blip(&result->sfxr);
		break;
	case SFXR_PICKUP:
	default:
		sfxr_pickup(&result->sfxr);
		break;
	}

	result->sfxr.playing_sample = 1;

	SDL_UnlockAudio();
}
