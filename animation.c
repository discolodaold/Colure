#include "animation.h"

#include <sdl/SDL.h>
#include <math.h>

#define PI 3.14

const char *ANIMATION_START = "animation start";
const char *ANIMATION_DONE = "animation done";

double easeLinear(double t, double d) {
	return t/d;
}

double easeSwing(double t, double d) {
	return ((-cos((t/d)*PI)/2) + 0.5);
}

double easeInQuad(double t, double d) {
	return (t/=d)*t;
}

double easeOutQuad(double t, double d) {
	return -1 *(t/=d)*(t-2);
}

double easeInOutQuad(double t, double d) {
	if ((t/=d/2) < 1) return 1/2*t*t + 0;
	return -1/2 * ((--t)*(t-2) - 1) + 0;
}

double easeInCubic(double t, double d) {
	return 1*(t/=d)*t*t + 0;
}

double easeOutCubic(double t, double d) {
	return 1*((t=t/d-1)*t*t + 1) + 0;
}

double easeInOutCubic(double t, double d) {
	if ((t/=d/2) < 1) return 1/2*t*t*t + 0;
	return 1/2*((t-=2)*t*t + 2) + 0;
}

double easeInQuart(double t, double d) {
	return 1*(t/=d)*t*t*t + 0;
}

double easeOutQuart(double t, double d) {
	return -1 * ((t=t/d-1)*t*t*t - 1) + 0;
}

double easeInOutQuart(double t, double d) {
	if ((t/=d/2) < 1) return 1/2*t*t*t*t + 0;
	return -1/2 * ((t-=2)*t*t*t - 2) + 0;
}

double easeInQuint(double t, double d) {
	return 1*(t/=d)*t*t*t*t + 0;
}

double easeOutQuint(double t, double d) {
	return 1*((t=t/d-1)*t*t*t*t + 1) + 0;
}

double easeInOutQuint(double t, double d) {
	if ((t/=d/2) < 1) return 1/2*t*t*t*t*t + 0;
	return 1/2*((t-=2)*t*t*t*t + 2) + 0;
}

double easeInSine(double t, double d) {
	return -1 * cos(t/d * (PI/2)) + 1 + 0;
}

double easeOutSine(double t, double d) {
	return 1 * sin(t/d * (PI/2)) + 0;
}

double easeInOutSine(double t, double d) {
	return -1/2 * (cos(PI*t/d) - 1) + 0;
}

double easeInExpo(double t, double d) {
	return (t==0) ? 0 : 1 * pow(2, 10 * (t/d - 1)) + 0;
}

double easeOutExpo(double t, double d) {
	return (t==d) ? 0+1 : 1 * (-pow(2, -10 * t/d) + 1) + 0;
}

double easeInOutExpo(double t, double d) {
	if (t==0) return 0;
	if (t==d) return 0+1;
	if ((t/=d/2) < 1) return 1/2 * pow(2, 10 * (t - 1)) + 0;
	return 1/2 * (-pow(2, -10 * --t) + 2) + 0;
}

double easeInCirc(double t, double d) {
	return -1 * (sqrt(1 - (t/=d)*t) - 1) + 0;
}

double easeOutCirc(double t, double d) {
	return 1 * sqrt(1 - (t=t/d-1)*t) + 0;
}

double easeInOutCirc(double t, double d) {
	if ((t/=d/2) < 1) return -1/2 * (sqrt(1 - t*t) - 1) + 0;
	return 1/2 * (sqrt(1 - (t-=2)*t) + 1) + 0;
}

double easeInElastic(double t, double d) {
	double s = 1.70158, p = 0, a = 1;
	if (t==0) return 0;
	if ((t/=d)==1) return 0+1;
	if (!p) p = d*.3;
	if (a < abs(1)) { a=1; s=p/4; }
	else s = p/(2*PI) * asin (1/a);
	return -(a*pow(2,10*(t-=1)) * sin( (t*d-s)*(2*PI)/p )) + 0;
}

double easeOutElastic(double t, double d) {
	double s = 1.70158, p = 0, a = 1;
	if (t == 0) return 0;
	if ((t/=d) == 1) return 0+1;
	if (!p) p = d*.3;
	if (a < abs(1)) { a=1; s=p/4; }
	else s = p/(2*PI) * asin (1/a);
	return a*pow(2,-10*t) * sin( (t*d-s)*(2*PI)/p ) + 1 + 0;
}

double easeInOutElastic(double t, double d) {
	double s = 1.70158, p = 0, a = 1;
	if (t==0) return 0;
	if ((t/=d/2)==2) return 0+1;
	if (!p) p=d*(.3*1.5);
	if (a < abs(1)) { a=1; s=p/4; }
	else s = p/(2*PI) * asin (1/a);
	if (t < 1) return -.5*(a*pow(2,10*(t-=1)) * sin( (t*d-s)*(2*PI)/p )) + 0;
	return a*pow(2,-10*(t-=1)) * sin( (t*d-s)*(2*PI)/p )*.5 + 1 + 0;
}

double easeInBounce(double t, double d) {
	return 1 - easeOutBounce(d-t, d) + 0;
}

double easeOutBounce(double t, double d) {
	if ((t/=d) < (1/2.75)) {
		return 1*(7.5625*t*t) + 0;
	} else if (t < (2/2.75)) {
		return 1*(7.5625*(t-=(1.5/2.75))*t + .75) + 0;
	} else if (t < (2.5/2.75)) {
		return 1*(7.5625*(t-=(2.25/2.75))*t + .9375) + 0;
	} else {
		return 1*(7.5625*(t-=(2.625/2.75))*t + .984375) + 0;
	}
}

double easeInOutBounce(double t, double d) {
	if (t < d/2)
		return easeInBounce(t*2, d) * .5 + 0;
	return easeOutBounce(t*2-d, d) * .5 + 1*.5 + 0;
}

struct animation {
	proc_t   *proc;
	double    duration;
	ease_f    ease;
	setter_f  fun;
	void     *data;

	proc_t   *self;
	Uint32    start;
	Uint32    end;
};

static PT_THREAD(animation_step(proc_t *p)) {
	static struct animation *animation;
	static double delta;
	animation = (struct animation *)p->data;
	PT_BEGIN(&(p->pt));
	proc_send(animation->proc, ANIMATION_START, animation);
	while(SDL_GetTicks() < animation->end) {
		delta = animation->ease(SDL_GetTicks() - animation->start, animation->duration);
		animation->fun(animation->data, delta);
		PT_YIELD(&(p->pt));
	}
	free(animation->data);
	proc_send(animation->proc, ANIMATION_DONE, animation);
	PT_END(&(p->pt));
}

struct animation *new_animation(proc_t *proc, double duration, ease_f ease, setter_f fun, void *data) {
	struct animation *result;
	fun(data, 0);
	result = (struct animation *)malloc(sizeof(*result));
	result->proc = proc;
	result->start = SDL_GetTicks();
	result->duration = duration;
	result->end = (Uint32)(result->start + duration);
	result->fun = fun;
	result->data = data;
	result->ease = ease ? ease : easeLinear;
	proc_new(&result->self, animation_step, 1, result);
	return result;
}

struct intAnimation {
	int    *target;
	double  from;
	double  to;
};

void intAnimation_fun(struct intAnimation *data, double delta) {
	*data->target = (int)(data->from + ((data->to - data->from) * delta));
}

struct animation *new_intAnimation(proc_t *proc, int *target, int from, int to, double duration, ease_f ease) {
	struct intAnimation *result;
	result = (struct intAnimation *)malloc(sizeof(*result));
	result->target = target;
	result->from = (double)from;
	result->to = (double)to;
	return new_animation(proc, duration, ease, (setter_f)intAnimation_fun, result);
}

struct shortAnimation {
	short   *target;
	double  from;
	double  to;
};

void shortAnimation_fun(struct shortAnimation *data, double delta) {
	*data->target = (short)(data->from + ((data->to - data->from) * delta));
}

struct animation *new_shortAnimation(proc_t *proc, short *target, short from, short to, double duration, ease_f ease) {
	struct shortAnimation *result;
	result = (struct shortAnimation *)malloc(sizeof(*result));
	result->target = target;
	result->from = (double)from;
	result->to = (double)to;
	return new_animation(proc, duration, ease, (setter_f)intAnimation_fun, result);
}
