#ifndef COLURE_ANIMATION
#define COLURE_ANIMATION

#include "proc.h"

extern const char *ANIMATION_START;
extern const char *ANIMATION_DONE;

typedef double (*ease_f)(double time, double duration);

double easeLinear(double t, double d);
double easeSwing(double t, double d);
double easeInQuad(double t, double d);
double easeOutQuad(double t, double d);
double easeInOutQuad(double t, double d);
double easeInCubic(double t, double d);
double easeOutCubic(double t, double d);
double easeInOutCubic(double t, double d);
double easeInQuart(double t, double d);
double easeOutQuart(double t, double d);
double easeInOutQuart(double t, double d);
double easeInQuint(double t, double d);
double easeOutQuint(double t, double d);
double easeInOutQuint(double t, double d);
double easeInSine(double t, double d);
double easeOutSine(double t, double d);
double easeInOutSine(double t, double d);
double easeInExpo(double t, double d);
double easeOutExpo(double t, double d);
double easeInOutExpo(double t, double d);
double easeInCirc(double t, double d);
double easeOutCirc(double t, double d);
double easeInOutCirc(double t, double d);
double easeInElastic(double t, double d);
double easeOutElastic(double t, double d);
double easeInOutElastic(double t, double d);
double easeInBounce(double t, double d);
double easeOutBounce(double t, double d);
double easeInOutBounce(double t, double d);

typedef void (*setter_f)(void *, double);

struct animation *new_animation(proc_t *proc, double duration, ease_f ease, setter_f fun, void *data);

struct animation *new_intAnimation(proc_t *proc, int *target, int from, int to, double duration, ease_f ease);
struct animation *new_shortAnimation(proc_t *proc, short *target, short from, short to, double duration, ease_f ease);

#endif
