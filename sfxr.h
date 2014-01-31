#ifndef COLURE_SFXR
#define COLURE_SFXR

#include <stdlib.h>

enum {
	SFXR_WAVE_SQUARE,
	SFXR_WAVE_SAWTOOTH,
	SFXR_WAVE_SINE,
	SFXR_WAVE_NOISE
};

typedef struct {
	int wave_type;

	float p_base_freq;
	float p_freq_limit;
	float p_freq_ramp;
	float p_freq_dramp;
	float p_duty;
	float p_duty_ramp;

	float p_vib_strength;
	float p_vib_speed;
	float p_vib_delay;

	float p_env_attack;
	float p_env_sustain;
	float p_env_decay;
	float p_env_punch;

	int filter_on;
	float p_lpf_resonance;
	float p_lpf_freq;
	float p_lpf_ramp;
	float p_hpf_freq;
	float p_hpf_ramp;

	float p_pha_offset;
	float p_pha_ramp;

	float p_repeat_speed;

	float p_arp_speed;
	float p_arp_mod;

	float master_vol;

	float sound_vol;

	int playing_sample;
	int phase;
	double fperiod;
	double fmaxperiod;
	double fslide;
	double fdslide;
	int period;
	float square_duty;
	float square_slide;
	int env_stage;
	int env_time;
	int env_length[3];
	float env_vol;
	float fphase;
	float fdphase;
	int iphase;
	float phaser_buffer[1024];
	int ipp;
	float noise_buffer[32];
	float fltp;
	float fltdp;
	float fltw;
	float fltw_d;
	float fltdmp;
	float fltphp;
	float flthp;
	float flthp_d;
	float vib_phase;
	float vib_speed;
	float vib_amp;
	int rep_time;
	int rep_limit;
	int arp_time;
	int arp_limit;
	double arp_mod;
} sfxr_t;

void sfxr_resetParams(sfxr_t *sfxr);
void sfxr_reset(sfxr_t *sfxr, int restart);
void sfxr_synth(sfxr_t *sfxr, size_t length, float *buffer);
void sfxr_pickup(sfxr_t *sfxr);
void sfxr_laser(sfxr_t *sfxr);
void sfxr_explosion(sfxr_t *sfxr);
void sfxr_powerup(sfxr_t *sfxr);
void sfxr_hit(sfxr_t *sfxr);
void sfxr_jump(sfxr_t *sfxr);
void sfxr_blip(sfxr_t *sfxr);

#endif