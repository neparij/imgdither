/************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/************************************************/
#include "Bitmap.h"
#include "DitherImage-Colourspace.h"
#include "DitherImage.h"
/************************************************/

//! strcmp() implementation that ACTUALLY returns the difference between
//! characters instead of just the signs. Blame the C standard -_-
static int mystrcmp(const char *s1, const char *s2, const char **s1End) {
	while(*s1 != '\0' && *s1 == *s2) s1++, s2++;
	if(s1End) *s1End = s1;
	return (int)(*s1) - (int)(*s2);
}

/************************************************/

//! Convert symbolic colourspace name to pretty string
static const char *ColourspaceNameString(uint8_t Colourspace) {
	switch(Colourspace) {
		case COLOURSPACE_SRGB:      return "sRGB";
		case COLOURSPACE_YCBCR:     return "YCbCr";
		case COLOURSPACE_YCOCG:     return "YCoCg";
		case COLOURSPACE_CIELAB:    return "CIELAB";
		case COLOURSPACE_ICTCP:     return "ICtCp";
		case COLOURSPACE_OKLAB:     return "OkLab";
		case COLOURSPACE_RGB_PSY:   return "RGB + Psyopt";
		case COLOURSPACE_YCBCR_PSY: return "YCbCr + Psyopt";
		case COLOURSPACE_YCOCG_PSY: return "YCoCg + Psyopt";
		default: return "[Unknown colourspace]";
	}
}

/************************************************/

static int ParseColourspace(const char *s) {
	     if(!strcmp(s, "srgb"))      return COLOURSPACE_SRGB;
	else if(!strcmp(s, "ycbcr"))     return COLOURSPACE_YCBCR;
	else if(!strcmp(s, "ycocg"))     return COLOURSPACE_YCOCG;
	else if(!strcmp(s, "cielab"))    return COLOURSPACE_CIELAB;
	else if(!strcmp(s, "ictcp"))     return COLOURSPACE_ICTCP;
	else if(!strcmp(s, "oklab"))     return COLOURSPACE_OKLAB;
	else if(!strcmp(s, "rgb-psy"))   return COLOURSPACE_RGB_PSY;
	else if(!strcmp(s, "ycbcr-psy")) return COLOURSPACE_YCBCR_PSY;
	else if(!strcmp(s, "ycocg-psy")) return COLOURSPACE_YCOCG_PSY;
	else return -1;
}

static int ParseDitherMode(const char *s, uint8_t *ModePtr, float *LevelPtr) {
	int   r; //! <- Will contain output from mystrcmp()
	int   Mode;
	float Level;
	const char *MatchEnd;
#define DITHERMODE_MATCH(Name) \
	(((r = mystrcmp(s, Name, &MatchEnd)) || 1) && (r == '\0' || r == ','))
	     if(DITHERMODE_MATCH("none" ))    Mode = DITHER_NONE,           Level = 0.0f;
	else if(DITHERMODE_MATCH("floyd"))    Mode = DITHER_FLOYDSTEINBERG, Level = 0.5f;
	else if(DITHERMODE_MATCH("atkinson")) Mode = DITHER_ATKINSON,       Level = 0.5f;
	else if(DITHERMODE_MATCH("checker"))  Mode = DITHER_CHECKER,        Level = 1.0f;
	else if(DITHERMODE_MATCH("ord2" ))    Mode = DITHER_ORDERED(1),     Level = 1.0f;
	else if(DITHERMODE_MATCH("ord4" ))    Mode = DITHER_ORDERED(2),     Level = 1.0f;
	else if(DITHERMODE_MATCH("ord8" ))    Mode = DITHER_ORDERED(3),     Level = 1.0f;
	else if(DITHERMODE_MATCH("ord16"))    Mode = DITHER_ORDERED(4),     Level = 1.0f;
	else if(DITHERMODE_MATCH("ord32"))    Mode = DITHER_ORDERED(5),     Level = 1.0f;
	else if(DITHERMODE_MATCH("ord64"))    Mode = DITHER_ORDERED(6),     Level = 1.0f;
	else return -1;
#undef DITHERMODE_MATCH
	if(Mode != DITHER_NONE && r == ',') {
		Level = atof(MatchEnd+1); //! <- Skip over the ',' symbol
	}
	if(Level < 0.0f) Level = 0.0f;
	if(Level > 2.0f) Level = 2.0f;
	*ModePtr  = (uint8_t)Mode;
	*LevelPtr = Level;
	return Mode;
}

static int ParseClearColour(const char *s, BGRA8_t *Col) {
	if(!strcmp(s, "none")) { Col->a = 0; return 0; }
	if(*s++ != '#') return -1;

	uint32_t Value;
	if(!sscanf(s, "%x", &Value)) return -1;
	Col->r = (uint8_t)(Value >> 16);
	Col->g = (uint8_t)(Value >>  8);
	Col->b = (uint8_t)(Value >>  0);
	Col->a = 255;
	return 0;
}

int main(int argc, const char *argv[]) {
	//! Check arguments
	if(argc < 4) {
		printf(
			"imgdither - Palette-matching image dithering tool\n"
			"Usage:\n"
			" imgdither Input.bmp Palette.bmp Output.bmp [options]\n"
			"Options:\n"
			"  -premulalpha:n       - Alpha is pre-multiplied (y/n)\n"
			"                         While most formats generally pre-multiply the colours\n"
			"                         by the alpha value, 32-bit BMP files generally do not.\n"
			"                         Note that if this option is set to `y`, then output\n"
			"                         colours in the palette will also be pre-multiplied.\n"
			"  -colspace:ycbcr-psy  - Set colourspace\n"
			"                         Different colourspaces may give better/worse results\n"
			"                         depending on the input image, and it may be necessary\n"
			"                         to experiment to find the optimal one.\n"
			"  -dither:floyd,0.5    - Set dither mode, level for output\n"
			"                         This can reduce some of the banding artifacts caused\n"
			"                         when the colours per palette is very small, at the\n"
			"                         expense of added \"noise\".\n"
			"  -col0isclear:y       - First colour of every palette is transparent (y/n)\n"
			"                         Note that this affects both input AND output images.\n"
			"                         To set transparency in a direct-colour input bitmap,\n"
			"                         an alpha channel must be used (ie. 32-bit input);\n"
			"                         translucent alpha values are supported by this tool.\n"
			"  -clearcol:none       - Set colour of transparent pixels. Note that as long\n"
			"                         as the RGB values match the clear colour, then the\n"
			"                         pixel will be made fully transparent, regardless of\n"
			"                         any alpha information.\n"
			"                         Can be `none`, or a `#RRGGBB` hex triad.\n"
			"Colourspaces available:\n"
			"  srgb\n"
			"  rgb-psy      (Psy = Non-linear light, weighted components)\n"
			"  ycbcr[-psy]  (Psy = Non-linear luma, weighted chroma)\n"
			"  ycocg[-psy]  (Psy = Non-linear luma)\n"
			"  cielab\n"
			"    NOTE: CIELAB has poor performance in most cases.\n"
			"  ictcp\n"
			"  oklab\n"
			"Dither modes available (and default level):\n"
			"  none         - No dithering\n"
			"  floyd,0.5    - Floyd-Steinberg\n"
			"  atkinson,0.5 - Atkinson diffusion\n"
			"  checker,1.0  - Chekerboard dithering\n"
			"  ord2,1.0     - 2x2 ordered dithering\n"
			"  ord4,1.0     - 4x4 ordered dithering\n"
			"  ord8,1.0     - 8x8 ordered dithering\n"
			"  ord16,1.0    - 16x16 ordered dithering\n"
			"  ord32,1.0    - 32x32 ordered dithering\n"
			"  ord64,1.0    - 64x64 ordered dithering\n"
		);
		return 1;
	}
	uint8_t FirstColourIsTransparent = 1;
	uint8_t PremultipliedAlpha   = 0;
	uint8_t DitherType           = DITHER_FLOYDSTEINBERG;
	float DitherLevel          = 0.5f;
	uint8_t Colourspace          = COLOURSPACE_YCBCR_PSY;

	{
		int argi;
		for(argi=4;argi<argc;argi++) {
			uint8_t ArgOk = 0;

			const char *ArgStr;
#define ARGMATCH(Input, Target) \
	ArgStr = Input + strlen(Target); \
	if(!memcmp(Input, Target, strlen(Target)))
			ARGMATCH(argv[argi], "-premulalpha:") ArgOk = 1, PremultipliedAlpha = (ArgStr[0] == 'y') ? 1 : 0;
			ARGMATCH(argv[argi], "-colspace:") {
				int c = ParseColourspace(ArgStr);
				if(c != -1) Colourspace = (uint8_t)c;
				else printf("WARNING: Unrecognized colourspace: %s\n", ArgStr);
				ArgOk = 1;
			}
			ARGMATCH(argv[argi], "-dither:") {
				if(ParseDitherMode(ArgStr, &DitherType, &DitherLevel) == -1) {
					printf("WARNING: Unrecognized output dither mode: %s\n", ArgStr);
				}
				ArgOk = 1;
			}
			ARGMATCH(argv[argi], "-col0isclear:")  ArgOk = 1, FirstColourIsTransparent = (ArgStr[0] == 'y') ? 1 : 0;
#undef ARGMATCH
			//! Unrecognized?
			if(!ArgOk) printf("WARNING: Unrecognized argument: %s\n", argv[argi]);
		}
	}

	//! Open input image
	struct BmpCtx_t Image;
	if(!BmpCtx_FromFile(&Image, argv[1])) {
		printf("ERROR: Unable to read input file.\n");
		return -1;
	}

	//! Open palette image
	struct BmpCtx_t PaletteImage;
	if(!BmpCtx_FromFile(&PaletteImage, argv[2])) {
		printf("ERROR: Unable to read palette image file.\n");
		return -1;
	}

	//! Create output image
	struct BmpCtx_t Output;
	if(!BmpCtx_Create(&Output, Image.Width, Image.Height, 1)) {
		printf("ERROR: Couldn't create output image.\n");
		BmpCtx_Destroy(&Image);
		return -1;
	}

	//! Get palette colour count from loaded BMP
	uint32_t nPaletteColours = PaletteImage.PaletteCount;
	if(nPaletteColours == 0) {
		fprintf(stderr, "ERROR: Palette image must be 8-bit palettized BMP.\n");
		BmpCtx_Destroy(&Image);
		BmpCtx_Destroy(&PaletteImage);
		BmpCtx_Destroy(&Output);
		return -1;
	}
	printf("Using palette with %u colours\n", nPaletteColours);

	//! Build byte palette in R,G,B,A order
	uint32_t nPaletteBytes = nPaletteColours * 4u;
	uint8_t *palBytes = malloc(nPaletteBytes);
	if(!palBytes) {
		fprintf(stderr, "ERROR: out of memory (palBytes)\n");
		BmpCtx_Destroy(&Image);
		BmpCtx_Destroy(&PaletteImage);
		BmpCtx_Destroy(&Output);
		return -1;
	}

	//! Convert palette from BGRA8_t array to byte array in RGBA order
	for(uint32_t i = 0; i < nPaletteColours; ++i) {
		BGRA8_t c = PaletteImage.Palette[i];
		palBytes[i*4 + 0] = c.r; /* R */
		palBytes[i*4 + 1] = c.g; /* G */
		palBytes[i*4 + 2] = c.b; /* B */
		palBytes[i*4 + 3] = c.a; /* A */
	}

	/* Convert input image to RGBA format for DitherPaletteImage */
	uint32_t nPixels = Image.Width * Image.Height;
	uint8_t *srcRGBA = malloc(nPixels * 4);
	if(!srcRGBA) {
		fprintf(stderr, "ERROR: out of memory (srcRGBA)\n");
		free(palBytes);
		BmpCtx_Destroy(&Image);
		BmpCtx_Destroy(&PaletteImage);
		BmpCtx_Destroy(&Output);
		return -1;
	}

	/* Check if input is palettized or direct color */
	if(Image.Palette) {
		/* Palettized input - convert indices to RGBA */
		uint8_t *indices = Image.PxIdx;
		for(uint32_t i = 0; i < nPixels; ++i) {
			BGRA8_t c = Image.Palette[indices[i]];
			srcRGBA[i*4 + 0] = c.r; /* R */
			srcRGBA[i*4 + 1] = c.g; /* G */
			srcRGBA[i*4 + 2] = c.b; /* B */
			srcRGBA[i*4 + 3] = c.a; /* A */
		}
	} else {
		/* Direct color input - convert from BGRA to RGBA */
		BGRA8_t *bgra = Image.PxBGR;
		for(uint32_t i = 0; i < nPixels; ++i) {
			srcRGBA[i*4 + 0] = bgra[i].r; /* R */
			srcRGBA[i*4 + 1] = bgra[i].g; /* G */
			srcRGBA[i*4 + 2] = bgra[i].b; /* B */
			srcRGBA[i*4 + 3] = bgra[i].a; /* A */
		}
	}

	DitherPaletteImage(
		Output.PxIdx,
		srcRGBA,
		palBytes,
		Image.Width,
		Image.Height,
		DitherType,
		DitherLevel,
		Colourspace,
		PremultipliedAlpha,
		nPaletteColours
	);

	free(srcRGBA);
	free(palBytes);
	BmpCtx_Destroy(&Image);

	//! Copy palette to output image
	memcpy(Output.Palette, PaletteImage.Palette, BMP_PALETTE_COLOURS * sizeof(BGRA8_t));

	//! Save image to file
	if(!BmpCtx_ToFile(&Output, argv[3])) {
		printf("ERROR: Unable to write output file.\n");
		BmpCtx_Destroy(&PaletteImage);
		BmpCtx_Destroy(&Output);
		return -1;
	}

	//! Success
	BmpCtx_Destroy(&PaletteImage);
	BmpCtx_Destroy(&Output);
	return 0;
}

/************************************************/
//! EOF
/************************************************/
