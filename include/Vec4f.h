/************************************************/
#pragma once
/************************************************/
#include <math.h>
#include <stddef.h> // NULL
/************************************************/

typedef struct {
	float f32[4];
} Vec4f_t;

/************************************************/

#define VEC4F_EMPTY (Vec4f_t){{0,0,0,0}}

/************************************************/

static inline Vec4f_t Vec4f_Add(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] + b->f32[0];
	y.f32[1] = a->f32[1] + b->f32[1];
	y.f32[2] = a->f32[2] + b->f32[2];
	y.f32[3] = a->f32[3] + b->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_Addi(const Vec4f_t *a, float b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] + b;
	y.f32[1] = a->f32[1] + b;
	y.f32[2] = a->f32[2] + b;
	y.f32[3] = a->f32[3] + b;
	return y;
}

/************************************************/

static inline Vec4f_t Vec4f_Sub(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] - b->f32[0];
	y.f32[1] = a->f32[1] - b->f32[1];
	y.f32[2] = a->f32[2] - b->f32[2];
	y.f32[3] = a->f32[3] - b->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_Subi(const Vec4f_t *a, float b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] - b;
	y.f32[1] = a->f32[1] - b;
	y.f32[2] = a->f32[2] - b;
	y.f32[3] = a->f32[3] - b;
	return y;
}

/************************************************/

static inline Vec4f_t Vec4f_Mul(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] * b->f32[0];
	y.f32[1] = a->f32[1] * b->f32[1];
	y.f32[2] = a->f32[2] * b->f32[2];
	y.f32[3] = a->f32[3] * b->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_Muli(const Vec4f_t *a, float b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] * b;
	y.f32[1] = a->f32[1] * b;
	y.f32[2] = a->f32[2] * b;
	y.f32[3] = a->f32[3] * b;
	return y;
}

/************************************************/

static inline Vec4f_t Vec4f_Div(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] / b->f32[0];
	y.f32[1] = a->f32[1] / b->f32[1];
	y.f32[2] = a->f32[2] / b->f32[2];
	y.f32[3] = a->f32[3] / b->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_DivSafe(const Vec4f_t *a, const Vec4f_t *b, const Vec4f_t *DivByZeroValue) {
	static const Vec4f_t Zero = VEC4F_EMPTY;
	if(!DivByZeroValue) DivByZeroValue = &Zero;

	Vec4f_t y;
	y.f32[0] = (b->f32[0] == 0.0f) ? DivByZeroValue->f32[0] : (a->f32[0] / b->f32[0]);
	y.f32[1] = (b->f32[1] == 0.0f) ? DivByZeroValue->f32[1] : (a->f32[1] / b->f32[1]);
	y.f32[2] = (b->f32[2] == 0.0f) ? DivByZeroValue->f32[2] : (a->f32[2] / b->f32[2]);
	y.f32[3] = (b->f32[3] == 0.0f) ? DivByZeroValue->f32[3] : (a->f32[3] / b->f32[3]);
	return y;
}

static inline Vec4f_t Vec4f_Divi(const Vec4f_t *a, float b) {
	Vec4f_t y;
	y.f32[0] = a->f32[0] / b;
	y.f32[1] = a->f32[1] / b;
	y.f32[2] = a->f32[2] / b;
	y.f32[3] = a->f32[3] / b;
	return y;
}

static inline Vec4f_t Vec4f_InverseDivi(const Vec4f_t *a, float b) {
	Vec4f_t y;
	y.f32[0] = b / a->f32[0];
	y.f32[1] = b / a->f32[1];
	y.f32[2] = b / a->f32[2];
	y.f32[3] = b / a->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_InverseDiviSafe(const Vec4f_t *a, float b, const Vec4f_t *DivByZeroValue) {
	static const Vec4f_t Zero = VEC4F_EMPTY;
	if(!DivByZeroValue) DivByZeroValue = &Zero;

	Vec4f_t y;
	y.f32[0] = (a->f32[0] == 0.0f) ? DivByZeroValue->f32[0] : (b / a->f32[0]);
	y.f32[1] = (a->f32[1] == 0.0f) ? DivByZeroValue->f32[1] : (b / a->f32[1]);
	y.f32[2] = (a->f32[2] == 0.0f) ? DivByZeroValue->f32[2] : (b / a->f32[2]);
	y.f32[3] = (a->f32[3] == 0.0f) ? DivByZeroValue->f32[3] : (b / a->f32[3]);
	return y;
}

/************************************************/

static inline Vec4f_t Vec4f_Abs(const Vec4f_t *x) {
	Vec4f_t y;
	y.f32[0] = fabsf(x->f32[0]);
	y.f32[1] = fabsf(x->f32[1]);
	y.f32[2] = fabsf(x->f32[2]);
	y.f32[3] = fabsf(x->f32[3]);
	return y;
}

static inline Vec4f_t Vec4f_Sqrt(const Vec4f_t *x) {
	Vec4f_t y;
	y.f32[0] = sqrtf(x->f32[0]);
	y.f32[1] = sqrtf(x->f32[1]);
	y.f32[2] = sqrtf(x->f32[2]);
	y.f32[3] = sqrtf(x->f32[3]);
	return y;
}

static inline float Vec4f_SumOf(const Vec4f_t *x) {
	return x->f32[0] +
	       x->f32[1] +
	       x->f32[2] +
	       x->f32[3] ;
}

static inline float Vec4f_Dot(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y = Vec4f_Mul(a, b);
	return Vec4f_SumOf(&y);
}

static inline float Vec4f_Length2(const Vec4f_t *x) {
	return Vec4f_Dot(x, x);
}

static inline float Vec4f_Length(const Vec4f_t *x) {
	return sqrtf(Vec4f_Length2(x));
}

static inline float Vec4f_Dist2(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t x = Vec4f_Sub(a, b);
	return Vec4f_Length2(&x);
}

static inline float Vec4f_Dist(const Vec4f_t *a, const Vec4f_t *b) {
	return sqrtf(Vec4f_Dist2(a, b));
}

static inline float Vec4f_DistL1(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t x = Vec4f_Sub(a, b);
	        x = Vec4f_Abs(&x);
	return Vec4f_SumOf(&x);
}

/************************************************/

static inline Vec4f_t Vec4f_Broadcast(float x) {
	Vec4f_t y;
	y.f32[0] = y.f32[1] = y.f32[2] = y.f32[3] = x;
	return y;
}

static inline float Vec4f_MinOf(const Vec4f_t *x) {
	float y = x->f32[0];
	if(x->f32[1] < y) y = x->f32[1];
	if(x->f32[2] < y) y = x->f32[2];
	if(x->f32[3] < y) y = x->f32[3];
	return y;
}

static inline float Vec4f_MaxOf(const Vec4f_t *x) {
	float y = x->f32[0];
	if(x->f32[1] > y) y = x->f32[1];
	if(x->f32[2] > y) y = x->f32[2];
	if(x->f32[3] > y) y = x->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_Min(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y;
	y.f32[0] = (a->f32[0] < b->f32[0]) ? a->f32[0] : b->f32[0];
	y.f32[1] = (a->f32[1] < b->f32[1]) ? a->f32[1] : b->f32[1];
	y.f32[2] = (a->f32[2] < b->f32[2]) ? a->f32[2] : b->f32[2];
	y.f32[3] = (a->f32[3] < b->f32[3]) ? a->f32[3] : b->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_Max(const Vec4f_t *a, const Vec4f_t *b) {
	Vec4f_t y;
	y.f32[0] = (a->f32[0] > b->f32[0]) ? a->f32[0] : b->f32[0];
	y.f32[1] = (a->f32[1] > b->f32[1]) ? a->f32[1] : b->f32[1];
	y.f32[2] = (a->f32[2] > b->f32[2]) ? a->f32[2] : b->f32[2];
	y.f32[3] = (a->f32[3] > b->f32[3]) ? a->f32[3] : b->f32[3];
	return y;
}

static inline Vec4f_t Vec4f_Round(const Vec4f_t *x) {
	Vec4f_t y;
	y.f32[0] = roundf(x->f32[0]);
	y.f32[1] = roundf(x->f32[1]);
	y.f32[2] = roundf(x->f32[2]);
	y.f32[3] = roundf(x->f32[3]);
	return y;
}

static inline Vec4f_t Vec4f_Floor(const Vec4f_t *x) {
	Vec4f_t y;
	y.f32[0] = floorf(x->f32[0]);
	y.f32[1] = floorf(x->f32[1]);
	y.f32[2] = floorf(x->f32[2]);
	y.f32[3] = floorf(x->f32[3]);
	return y;
}

static inline Vec4f_t Vec4f_Ceil(const Vec4f_t *x) {
	Vec4f_t y;
	y.f32[0] = ceilf(x->f32[0]);
	y.f32[1] = ceilf(x->f32[1]);
	y.f32[2] = ceilf(x->f32[2]);
	y.f32[3] = ceilf(x->f32[3]);
	return y;
}

static inline Vec4f_t Vec4f_Quantize(const Vec4f_t *x, const Vec4f_t *Depth) {
	Vec4f_t y = Vec4f_Mul(x, Depth);
	        y = Vec4f_Round(&y);
	return Vec4f_DivSafe(&y, Depth, NULL);
}

static inline Vec4f_t Vec4f_QuantizeFloor(const Vec4f_t *x, const Vec4f_t *Depth) {
	Vec4f_t y = Vec4f_Mul(x, Depth);
	        y = Vec4f_Floor(&y);
	return Vec4f_DivSafe(&y, Depth, NULL);
}

static inline Vec4f_t Vec4f_QuantizeCeil(const Vec4f_t *x, const Vec4f_t *Depth) {
	Vec4f_t y = Vec4f_Mul(x, Depth);
	        y = Vec4f_Ceil(&y);
	return Vec4f_DivSafe(&y, Depth, NULL);
}

static inline Vec4f_t Vec4f_Clamp(const Vec4f_t *x, float Min, float Max) {
	Vec4f_t y;
	y.f32[0] = (x->f32[0] < Min) ? Min : (x->f32[0] > Max) ? Max : x->f32[0];
	y.f32[1] = (x->f32[1] < Min) ? Min : (x->f32[1] > Max) ? Max : x->f32[1];
	y.f32[2] = (x->f32[2] < Min) ? Min : (x->f32[2] > Max) ? Max : x->f32[2];
	y.f32[3] = (x->f32[3] < Min) ? Min : (x->f32[3] > Max) ? Max : x->f32[3];
	return y;
}

/************************************************/
//! EOF
/************************************************/
