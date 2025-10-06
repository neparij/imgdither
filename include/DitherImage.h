/************************************************/
#pragma once
/************************************************/
#include <stdint.h>
/************************************************/

void DitherPaletteImage(
          uint8_t *DstPx,
    const uint8_t *SrcPx,
    const uint8_t *Palette,
    uint32_t Width,
    uint32_t Height,
    uint8_t  DitherType,
    float    DitherLevel,
    uint8_t  Colourspace,
    uint8_t  PremultipliedAlpha,
    uint32_t nPaletteColours
);

/************************************************/
//! EOF
/************************************************/