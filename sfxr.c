#include "sfxr.h"

#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#define rnd(n) (rand()%(n+1))

#define PI 3.14159265

float frnd(float range) {
	return (float)rnd(10000)/10000*range;
}

void sfxr_resetParams(sfxr_t *sfxr) {
	sfxr->wave_type=0;

	sfxr->p_base_freq=0.3f;
	sfxr->p_freq_limit=0.0f;
	sfxr->p_freq_ramp=0.0f;
	sfxr->p_freq_dramp=0.0f;
	sfxr->p_duty=0.0f;
	sfxr->p_duty_ramp=0.0f;

	sfxr->p_vib_strength=0.0f;
	sfxr->p_vib_speed=0.0f;
	sfxr->p_vib_delay=0.0f;

	sfxr->p_env_attack=0.0f;
	sfxr->p_env_sustain=0.3f;
	sfxr->p_env_decay=0.4f;
	sfxr->p_env_punch=0.0f;

	sfxr->filter_on=0;
	sfxr->p_lpf_resonance=0.0f;
	sfxr->p_lpf_freq=1.0f;
	sfxr->p_lpf_ramp=0.0f;
	sfxr->p_hpf_freq=0.0f;
	sfxr->p_hpf_ramp=0.0f;
	
	sfxr->p_pha_offset=0.0f;
	sfxr->p_pha_ramp=0.0f;

	sfxr->p_repeat_speed=0.0f;

	sfxr->p_arp_speed=0.0f;
	sfxr->p_arp_mod=0.0f;

	sfxr->sound_vol = 0.5f;
	sfxr->master_vol = 0.5f;
}

void sfxr_reset(sfxr_t *sfxr, int restart) {
	if(!restart)
		sfxr->phase=0;
	sfxr->fperiod=100.0/(sfxr->p_base_freq*sfxr->p_base_freq+0.001);
	sfxr->period=(int)sfxr->fperiod;
	sfxr->fmaxperiod=100.0/(sfxr->p_freq_limit*sfxr->p_freq_limit+0.001);
	sfxr->fslide=1.0-pow((double)sfxr->p_freq_ramp, 3.0)*0.01;
	sfxr->fdslide=-pow((double)sfxr->p_freq_dramp, 3.0)*0.000001;
	sfxr->square_duty=0.5f-sfxr->p_duty*0.5f;
	sfxr->square_slide=-sfxr->p_duty_ramp*0.00005f;
	if(sfxr->p_arp_mod>=0.0f)
		sfxr->arp_mod=1.0-pow((double)sfxr->p_arp_mod, 2.0)*0.9;
	else
		sfxr->arp_mod=1.0+pow((double)sfxr->p_arp_mod, 2.0)*10.0;
	sfxr->arp_time=0;
	sfxr->arp_limit=(int)(pow(1.0f-sfxr->p_arp_speed, 2.0f)*20000+32);
	if(sfxr->p_arp_speed==1.0f)
		sfxr->arp_limit=0;
	if(!restart)
	{
		int i;
		// reset filter
		sfxr->fltp=0.0f;
		sfxr->fltdp=0.0f;
		sfxr->fltw=pow(sfxr->p_lpf_freq, 3.0f)*0.1f;
		sfxr->fltw_d=1.0f+sfxr->p_lpf_ramp*0.0001f;
		sfxr->fltdmp=5.0f/(1.0f+pow(sfxr->p_lpf_resonance, 2.0f)*20.0f)*(0.01f+sfxr->fltw);
		if(sfxr->fltdmp>0.8f) sfxr->fltdmp=0.8f;
		sfxr->fltphp=0.0f;
		sfxr->flthp=pow(sfxr->p_hpf_freq, 2.0f)*0.1f;
		sfxr->flthp_d=1.0+sfxr->p_hpf_ramp*0.0003f;
		// reset vibrato
		sfxr->vib_phase=0.0f;
		sfxr->vib_speed=pow(sfxr->p_vib_speed, 2.0f)*0.01f;
		sfxr->vib_amp=sfxr->p_vib_strength*0.5f;
		// reset envelope
		sfxr->env_vol=0.0f;
		sfxr->env_stage=0;
		sfxr->env_time=0;
		sfxr->env_length[0]=(int)(sfxr->p_env_attack*sfxr->p_env_attack*100000.0f);
		sfxr->env_length[1]=(int)(sfxr->p_env_sustain*sfxr->p_env_sustain*100000.0f);
		sfxr->env_length[2]=(int)(sfxr->p_env_decay*sfxr->p_env_decay*100000.0f);

		sfxr->fphase=pow(sfxr->p_pha_offset, 2.0f)*1020.0f;
		if(sfxr->p_pha_offset<0.0f) sfxr->fphase=-sfxr->fphase;
		sfxr->fdphase=pow(sfxr->p_pha_ramp, 2.0f)*1.0f;
		if(sfxr->p_pha_ramp<0.0f) sfxr->fdphase=-sfxr->fdphase;
		sfxr->iphase=abs((int)sfxr->fphase);
		sfxr->ipp=0;
		for(i=0;i<1024;i++)
			sfxr->phaser_buffer[i]=0.0f;

		for(i=0;i<32;i++)
			sfxr->noise_buffer[i]=frnd(2.0f)-1.0f;

		sfxr->rep_time=0;
		sfxr->rep_limit=(int)(pow(1.0f-sfxr->p_repeat_speed, 2.0f)*20000+32);
		if(sfxr->p_repeat_speed==0.0f)
			sfxr->rep_limit=0;
	}
}

void sfxr_synth(sfxr_t *sfxr, size_t length, float *buffer) {
	int i, si;
	float rfperiod, ssample, fp;
	for(i = 0; i < length; i++) {
		if(!sfxr->playing_sample)
			break;

		sfxr->rep_time++;
		if(sfxr->rep_limit!=0 && sfxr->rep_time>=sfxr->rep_limit)
		{
			sfxr->rep_time=0;
			sfxr_reset(sfxr, 1);
		}

		// frequency envelopes/arpeggios
		sfxr->arp_time++;
		if(sfxr->arp_limit!=0 && sfxr->arp_time>=sfxr->arp_limit)
		{
			sfxr->arp_limit=0;
			sfxr->fperiod*=sfxr->arp_mod;
		}
		sfxr->fslide+=sfxr->fdslide;
		sfxr->fperiod*=sfxr->fslide;
		if(sfxr->fperiod>sfxr->fmaxperiod)
		{
			sfxr->fperiod=sfxr->fmaxperiod;
			if(sfxr->p_freq_limit>0.0f)
				sfxr->playing_sample=0;
		}
		rfperiod=sfxr->fperiod;
		if(sfxr->vib_amp>0.0f)
		{
			sfxr->vib_phase+=sfxr->vib_speed;
			rfperiod=sfxr->fperiod*(1.0+sin(sfxr->vib_phase)*sfxr->vib_amp);
		}
		sfxr->period=(int)rfperiod;
		if(sfxr->period<8) sfxr->period=8;
		sfxr->square_duty+=sfxr->square_slide;
		if(sfxr->square_duty<0.0f) sfxr->square_duty=0.0f;
		if(sfxr->square_duty>0.5f) sfxr->square_duty=0.5f;		
		// volume envelope
		sfxr->env_time++;
		if(sfxr->env_time>sfxr->env_length[sfxr->env_stage])
		{
			sfxr->env_time=0;
			sfxr->env_stage++;
			if(sfxr->env_stage==3)
				sfxr->playing_sample=0;
		}
		if(sfxr->env_stage==0)
			sfxr->env_vol=(float)sfxr->env_time/sfxr->env_length[0];
		if(sfxr->env_stage==1)
			sfxr->env_vol=1.0f+pow(1.0f-(float)sfxr->env_time/sfxr->env_length[1], 1.0f)*2.0f*sfxr->p_env_punch;
		if(sfxr->env_stage==2)
			sfxr->env_vol=1.0f-(float)sfxr->env_time/sfxr->env_length[2];

		// phaser step
		sfxr->fphase+=sfxr->fdphase;
		sfxr->iphase=abs((int)sfxr->fphase);
		if(sfxr->iphase>1023) sfxr->iphase=1023;

		if(sfxr->flthp_d!=0.0f)
		{
			sfxr->flthp*=sfxr->flthp_d;
			if(sfxr->flthp<0.00001f) sfxr->flthp=0.00001f;
			if(sfxr->flthp>0.1f) sfxr->flthp=0.1f;
		}

		ssample=0.0f;
		for(si=0;si<8;si++) // 8x supersampling
		{
			int j;
			float sample=0.0f, pp;
			sfxr->phase++;
			if(sfxr->phase>=sfxr->period)
			{
//				phase=0;
				sfxr->phase%=sfxr->period;
				if(sfxr->wave_type==3)
					for(j=0;j<32;j++)
						sfxr->noise_buffer[j]=frnd(2.0f)-1.0f;
			}
			// base waveform
			fp=(float)sfxr->phase/sfxr->period;
			switch(sfxr->wave_type)
			{
			case 0: // square
				if(fp<sfxr->square_duty)
					sample=0.5f;
				else
					sample=-0.5f;
				break;
			case 1: // sawtooth
				sample=1.0f-fp*2;
				break;
			case 2: // sine
				sample=(float)sin(fp*2*PI);
				break;
			case 3: // noise
				sample=sfxr->noise_buffer[sfxr->phase*32/sfxr->period];
				break;
			}
			// lp filter
			pp=sfxr->fltp;
			sfxr->fltw*=sfxr->fltw_d;
			if(sfxr->fltw<0.0f) sfxr->fltw=0.0f;
			if(sfxr->fltw>0.1f) sfxr->fltw=0.1f;
			if(sfxr->p_lpf_freq!=1.0f)
			{
				sfxr->fltdp+=(sample-sfxr->fltp)*sfxr->fltw;
				sfxr->fltdp-=sfxr->fltdp*sfxr->fltdmp;
			}
			else
			{
				sfxr->fltp=sample;
				sfxr->fltdp=0.0f;
			}
			sfxr->fltp+=sfxr->fltdp;
			// hp filter
			sfxr->fltphp+=sfxr->fltp-pp;
			sfxr->fltphp-=sfxr->fltphp*sfxr->flthp;
			sample=sfxr->fltphp;
			// phaser
			sfxr->phaser_buffer[sfxr->ipp&1023]=sample;
			sample+=sfxr->phaser_buffer[(sfxr->ipp-sfxr->iphase+1024)&1023];
			sfxr->ipp=(sfxr->ipp+1)&1023;
			// final accumulation and envelope application
			ssample+=sample*sfxr->env_vol;
		}
		ssample=ssample/8*sfxr->master_vol;

		ssample*=2.0f*sfxr->sound_vol;

		if(ssample>1.0f) ssample=1.0f;
		if(ssample<-1.0f) ssample=-1.0f;
		// NOTE: changed to += so we can synth mutliple sounds on top of each other
		*buffer++ += ssample;
	}
}

void sfxr_pickup(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	sfxr->p_base_freq=0.4+frnd(0.5);
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=frnd(0.1);
	sfxr->p_env_decay=0.1+frnd(0.4);
	sfxr->p_env_punch=0.3+frnd(0.3);
	if(rnd(1))
	{
		sfxr->p_arp_speed=0.5+frnd(0.2);
		sfxr->p_arp_mod=0.2+frnd(0.4);
	}
	sfxr_reset(sfxr, 0);
}

void sfxr_laser(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	sfxr->wave_type=rnd(2);
	if(sfxr->wave_type==2 && rnd(1))
		sfxr->wave_type=rnd(1);
	sfxr->p_base_freq = 0.5+frnd(0.5);
	sfxr->p_freq_limit = sfxr->p_base_freq-0.2-frnd(0.6);
	if(sfxr->p_freq_limit<0.2) sfxr->p_freq_limit=0.2;
	sfxr->p_freq_ramp=-0.15-frnd(0.2);
	if(rnd(2)==0)
	{
		sfxr->p_base_freq=0.3+frnd(0.6);
		sfxr->p_freq_limit=frnd(0.1);
		sfxr->p_freq_ramp=-0.35-frnd(0.3);
	}
	if(rnd(1))
	{
		sfxr->p_duty=frnd(0.5);
		sfxr->p_duty_ramp=frnd(0.2);
	}
	else
	{
		sfxr->p_duty=0.4+frnd(0.5);
		sfxr->p_duty_ramp=-frnd(0.7);
	}
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=0.1+frnd(0.2);
	sfxr->p_env_decay=frnd(0.4);
	if(rnd(1))
		sfxr->p_env_punch=frnd(0.3);
	if(rnd(2)==0)
	{
		sfxr->p_pha_offset=frnd(0.2);
		sfxr->p_pha_ramp=-frnd(0.2);
	}
	if(rnd(1))
		sfxr->p_hpf_freq=frnd(0.3);
	sfxr_reset(sfxr, 0);
}

void sfxr_explosion(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	sfxr->wave_type=3;
	if(rnd(1))
	{
		sfxr->p_base_freq=0.1+frnd(0.4);
		sfxr->p_freq_ramp=-0.1+frnd(0.4);
	}
	else
	{
		sfxr->p_base_freq=0.2+frnd(0.7);
		sfxr->p_freq_ramp=-0.2-frnd(0.2);
	}
	sfxr->p_base_freq*=sfxr->p_base_freq;
	if(rnd(4)==0)
		sfxr->p_freq_ramp=0.0;
	if(rnd(2)==0)
		sfxr->p_repeat_speed=0.3+frnd(0.5);
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=0.1+frnd(0.3);
	sfxr->p_env_decay=frnd(0.5);
	if(rnd(1)==0)
	{
		sfxr->p_pha_offset=-0.3+frnd(0.9);
		sfxr->p_pha_ramp=-frnd(0.3);
	}
	sfxr->p_env_punch=0.2+frnd(0.6);
	if(rnd(1))
	{
		sfxr->p_vib_strength=frnd(0.7);
		sfxr->p_vib_speed=frnd(0.6);
	}
	if(rnd(2)==0)
	{
		sfxr->p_arp_speed=0.6+frnd(0.3);
		sfxr->p_arp_mod=0.8-frnd(1.6);
	}
	sfxr_reset(sfxr, 0);
}

void sfxr_powerup(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	if(rnd(1))
		sfxr->wave_type=1;
	else
		sfxr->p_duty=frnd(0.6);
	if(rnd(1))
	{
		sfxr->p_base_freq=0.2+frnd(0.3);
		sfxr->p_freq_ramp=0.1+frnd(0.4);
		sfxr->p_repeat_speed=0.4+frnd(0.4);
	}
	else
	{
		sfxr->p_base_freq=0.2+frnd(0.3);
		sfxr->p_freq_ramp=0.05+frnd(0.2);
		if(rnd(1))
		{
			sfxr->p_vib_strength=frnd(0.7);
			sfxr->p_vib_speed=frnd(0.6);
		}
	}
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=frnd(0.4);
	sfxr->p_env_decay=0.1+frnd(0.4);
	sfxr_reset(sfxr, 0);
}

void sfxr_hit(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	sfxr->wave_type=rnd(2);
	if(sfxr->wave_type==2)
		sfxr->wave_type=3;
	if(sfxr->wave_type==0)
		sfxr->p_duty=frnd(0.6);
	sfxr->p_base_freq=0.2+frnd(0.6);
	sfxr->p_freq_ramp=-0.3-frnd(0.4);
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=frnd(0.1);
	sfxr->p_env_decay=0.1+frnd(0.2);
	if(rnd(1))
		sfxr->p_hpf_freq=frnd(0.3);
	sfxr_reset(sfxr, 0);
}

void sfxr_jump(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	sfxr->wave_type=0;
	sfxr->p_duty=frnd(0.6);
	sfxr->p_base_freq=0.3+frnd(0.3);
	sfxr->p_freq_ramp=0.1+frnd(0.2);
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=0.1+frnd(0.3);
	sfxr->p_env_decay=0.1+frnd(0.2);
	if(rnd(1))
		sfxr->p_hpf_freq=frnd(0.3);
	if(rnd(1))
		sfxr->p_lpf_freq=1.0-frnd(0.6);
	sfxr_reset(sfxr, 0);
}

void sfxr_blip(sfxr_t *sfxr) {
	sfxr_resetParams(sfxr);
	sfxr->wave_type=rnd(1);
	if(sfxr->wave_type==0)
		sfxr->p_duty=frnd(0.6);
	sfxr->p_base_freq=0.2+frnd(0.4);
	sfxr->p_env_attack=0.0;
	sfxr->p_env_sustain=0.1+frnd(0.1);
	sfxr->p_env_decay=frnd(0.2);
	sfxr->p_hpf_freq=0.1;
	sfxr_reset(sfxr, 0);
}

