/************************************************/
#include <stdint.h>
#include <stdlib.h>
/************************************************/
#include "DitherImage.h"
#include "DitherImage-Colourspace.h"
#include "Vec4f.h"
/************************************************/

//! Find closest colour in given palette
static uint8_t FindNearestColour(const Vec4f_t *x, const Vec4f_t *Pal, uint32_t nCols) {
	uint32_t n;
	uint8_t BestIdx = 0;
	float BestDist = INFINITY;
	for(n=0;n<nCols;n++) {
		float Dist = Vec4f_Dist2(x, &Pal[n]);
		if(Dist < BestDist) {
			BestIdx  = (uint8_t)n;
			BestDist = Dist;
		}
	}
	return BestIdx;
}
static uint8_t FindNearestDitheredColour(const Vec4f_t *x, const Vec4f_t *Bias, const Vec4f_t *Pal, uint32_t nCols) {
	uint32_t n;

	//! Find closest two matches
	//! Note that we ensure to not find a duplicate entry,
	//! and if we only have one match, we use it anyway.
	uint8_t BestIdxA = 0, BestIdxB = 0;
	float BestDistA = INFINITY;
	float BestDistB = INFINITY;
	for(n=0;n<nCols;n++) {
		float Dist = Vec4f_Dist2(x, &Pal[n]);
		if(Dist < BestDistA) {
			BestIdxB  = BestIdxA;
			BestDistB = BestDistA;
			BestIdxA  = (uint8_t)n;
			BestDistA = Dist;
		} else if(Dist < BestDistB && Dist > BestDistA) {
			BestIdxB  = (uint8_t)n;
			BestDistB = Dist;
		}
	}
	if(BestDistB == INFINITY) return BestIdxA;
	if(BestDistA < 0.25f*BestDistB) { //! DistA/DistB < (1/2)^2
		//! We are very out of range, so don't bother dithering
		return BestIdxA;
	}

	//! Scale the bias by their differences, and find closest match to this
	Vec4f_t xNew = Vec4f_Sub(&Pal[BestIdxA], &Pal[BestIdxB]);
	        xNew = Vec4f_Abs(&xNew);
	        xNew = Vec4f_Mul(&xNew, Bias);
	        xNew = Vec4f_Add(&xNew, x);
	return FindNearestColour(&xNew, Pal, nCols);
}

//! Calculate checkered dithering offset
static inline float CheckerDitherOffset(uint32_t x, uint32_t y) {
	return (float)((x^y) & 1) - 0.5f;
}

//! Calculate ordered dithering offset
static inline float OrderedDitherOffset(uint32_t x, uint32_t y, uint8_t Log2Size) {
	uint8_t Bit = Log2Size;
	uint32_t Threshold = 0, xKey = x, yKey = x^y;
	do {
		Threshold = Threshold*2 + (yKey & 1), yKey >>= 1; //! <- Hopefully turned into "SHR, ADC"
		Threshold = Threshold*2 + (xKey & 1), xKey >>= 1;
	} while(--Bit);
	return (float)Threshold * (1.0f / (float)(1 << (2*Log2Size))) - 0.5f;
}

//! Propagate error to neighbouring pixels (Floyd-Steinberg)
static inline void FloydSteinberg_PropagateError(const Vec4f_t *Error, Vec4f_t *y0, Vec4f_t *y1) {
	Vec4f_t t;
	t = Vec4f_Muli(Error, 7.0f/16); //! {x+1,y}   @ 7/16
	y0[+1] = Vec4f_Add(&y0[+1], &t);
	t = Vec4f_Muli(Error, 3.0f/16); //! {x-1,y+1} @ 3/16
	y1[-1] = Vec4f_Add(&y1[-1], &t);
	t = Vec4f_Muli(Error, 5.0f/16); //! {x+0,y+1} @ 5/16
	y1[+0] = Vec4f_Add(&y1[+0], &t);
	t = Vec4f_Muli(Error, 1.0f/16); //! {x+1,y+1} @ 1/16
	y1[+1] = Vec4f_Add(&y1[+1], &t);
}

//! Propagate error to neighbouring pixels (Atkinson diffusion)
static inline void Atkinson_PropagateError(const Vec4f_t *Error, Vec4f_t *y0, Vec4f_t *y1, Vec4f_t *y2) {
	Vec4f_t t = Vec4f_Muli(Error, 1.0f/8);
	y0[+1] = Vec4f_Add(&y0[+1], &t);
	y0[+2] = Vec4f_Add(&y0[+2], &t);
	y1[-1] = Vec4f_Add(&y1[-1], &t);
	y1[ 0] = Vec4f_Add(&y1[ 0], &t);
	y1[+1] = Vec4f_Add(&y1[+1], &t);
	y2[ 0] = Vec4f_Add(&y2[ 0], &t);
}

/************************************************/

//! Dither palettized, tiled image data
void DitherPaletteImage(
	      uint8_t *DstPx,
	const uint8_t *SrcPx,   //! RGBA
	const uint8_t *Palette, //! RGBA
	uint32_t Width,
	uint32_t Height,
	uint8_t  DitherType,
	float    DitherLevel,
	uint8_t  Colourspace,
	uint8_t  PremultipliedAlpha,
	uint32_t nPaletteColours
) {
	uint32_t n;

	//! Convert palette to target colourspace
	Vec4f_t *NewPal = malloc(nPaletteColours * sizeof(Vec4f_t));
	for(n=0;n<nPaletteColours;n++) {
		Vec4f_t t;
		t.f32[0] = Palette[n*4+0] / 255.0f;
		t.f32[1] = Palette[n*4+1] / 255.0f;
		t.f32[2] = Palette[n*4+2] / 255.0f;
		t.f32[3] = Palette[n*4+3] / 255.0f;
		t = ConvertToColourspace(&t, Colourspace);
		if(!PremultipliedAlpha) {
			t.f32[0] *= t.f32[3];
			t.f32[1] *= t.f32[3];
			t.f32[2] *= t.f32[3];
		}
		NewPal[n] = t;
	}

	//! If we requested a diffusion dither, allocate buffer
	Vec4f_t *DitherBuffer = NULL;
	Vec4f_t *Diffuse_y0   = NULL;
	Vec4f_t *Diffuse_y1   = NULL;
	Vec4f_t *Diffuse_y2   = NULL;
	if(DitherType == DITHER_FLOYDSTEINBERG) {
		DitherBuffer = (Vec4f_t*)malloc((Width*2+3) * sizeof(Vec4f_t));
		if(DitherBuffer) {
			Diffuse_y0 = DitherBuffer + 1;
			Diffuse_y1 = Diffuse_y0   + Width+1;
			for(n=0;n<Width;n++) Diffuse_y1[n] = VEC4F_EMPTY;
		} else {
			//! If we have no memory, disable dithering
			DitherType = DITHER_NONE;
		}
	} else if(DitherType == DITHER_ATKINSON) {
		DitherBuffer = (Vec4f_t*)malloc((Width*3+7) * sizeof(Vec4f_t));
		if(DitherBuffer) {
			Diffuse_y0 = DitherBuffer + 1;
			Diffuse_y1 = Diffuse_y0   + Width+2;
			Diffuse_y2 = Diffuse_y1   + Width+2;
			for(n=0;n<Width;n++) Diffuse_y1[n] = VEC4F_EMPTY;
			for(n=0;n<Width;n++) Diffuse_y2[n] = VEC4F_EMPTY;
		} else {
			//! If we have no memory, disable dithering
			DitherType = DITHER_NONE;
		}
	}

	//! Begin dithering
	//! NOTE: We can't clamp values here, because the input colourspaces
	//! do not necessarily have a nominal range of 0.0 to 1.0. This may
	//! cause issues at times, but hopefully this is minor.
	uint32_t x, y;
	for(y=0;y<Height;y++) {
		//! Swap diffusion buffers and clear for the next line
		if(DitherType == DITHER_FLOYDSTEINBERG) {
			Vec4f_t *t = Diffuse_y0;
			Diffuse_y0 = Diffuse_y1;
			Diffuse_y1 = t;
			for(n=0;n<Width;n++) Diffuse_y1[n] = VEC4F_EMPTY;
		} else if(DitherType == DITHER_ATKINSON) {
			Vec4f_t *t = Diffuse_y0;
			Diffuse_y0 = Diffuse_y1;
			Diffuse_y1 = Diffuse_y2;
			Diffuse_y2 = t;
			for(n=0;n<Width;n++) Diffuse_y2[n] = VEC4F_EMPTY;
		}
		for(x=0;x<Width;x++) {
			//! Grab pixel and apply dithering, palette mapping
			Vec4f_t Px, PxOrig; {
				const uint8_t *Src = SrcPx + (y*Width+x)*4;
				Vec4f_t t;
				t.f32[0] = Src[0] / 255.0f;
				t.f32[1] = Src[1] / 255.0f;
				t.f32[2] = Src[2] / 255.0f;
				t.f32[3] = Src[3] / 255.0f;
				t = ConvertToColourspace(&t, Colourspace);
				if(!PremultipliedAlpha) {
					t.f32[0] *= t.f32[3];
					t.f32[1] *= t.f32[3];
					t.f32[2] *= t.f32[3];
				}
				PxOrig = t;
			}
			uint8_t BestFitIdx = 0;
			if(DitherType != DITHER_NONE) {
				if(DitherType == DITHER_FLOYDSTEINBERG || DitherType == DITHER_ATKINSON) {
					//! Subtract diffused error and propagate new error
					Px = Diffuse_y0[x];
					Px = Vec4f_Muli (&Px, DitherLevel);
					Px = Vec4f_Add  (&Px, &PxOrig);
					BestFitIdx = FindNearestColour(&Px, NewPal, nPaletteColours);
					Px = NewPal[BestFitIdx];
					Vec4f_t Error = Vec4f_Sub(&PxOrig, &Px);
					if(DitherType == DITHER_FLOYDSTEINBERG) {
						FloydSteinberg_PropagateError(&Error, Diffuse_y0+x, Diffuse_y1+x);
					} else {
						Atkinson_PropagateError(&Error, Diffuse_y0+x, Diffuse_y1+x, Diffuse_y2+x);
					}
				} else {
					//! Adjust for dither matrix
					float Offs;
					if(DitherType != DITHER_CHECKER) {
						Offs = OrderedDitherOffset(x, y, DitherType);
					} else {
						Offs = CheckerDitherOffset(x, y);
					}
					Vec4f_t vOffs = Vec4f_Broadcast(Offs * DitherLevel);
					BestFitIdx = FindNearestDitheredColour(&PxOrig, &vOffs, NewPal, nPaletteColours);
				}
			} else {
				BestFitIdx = FindNearestColour(&PxOrig, NewPal, nPaletteColours);
			}
			DstPx[y*Width+x] = BestFitIdx;
		}
	}

	//! Release memory
	/*if(DitherBuffer)*/ free(DitherBuffer);
	free(NewPal);
}

/************************************************/
//! EOF
/************************************************/
