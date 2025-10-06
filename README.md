# imgdither

Palette-matching image dithering tool for converting images to use a specific palette.

## Description

This tool applies dithering algorithms to convert true-color or paletted images to match a target palette. It supports multiple color spaces and dithering modes, making it suitable for retro graphics work, especially for GBA/NDS development.

The dithering algorithms and color space conversions are based on [qualetize](https://github.com/Aikku93/qualetize) by [Aikku93](https://github.com/Aikku93).

## Features

- Multiple dithering algorithms (Floyd-Steinberg, Atkinson, ordered dithering, etc.)
- Various color space support (sRGB, YCbCr, YCoCg, CIELAB, ICtCp, OkLab)
- Psychovisual optimization modes
- Handles both palettized (8-bit) and direct color (24/32-bit) BMP input
- Automatic palette color count detection

## Building

Simply run:

```bash
make
```

This will produce:
- `release/imgdither` - Command-line tool
- `release/libimgdither.so` (or `.dll` on Windows) - Shared library for Python/other interfaces

## Usage

### Command Line

```bash
./release/imgdither Input.bmp Palette.bmp Output.bmp [options]
```

**Example:**
```bash
./release/imgdither input.bmp palette.bmp output.bmp -dither:floyd,0.5 -colspace:ycbcr-psy
```

## File Format Notes

- **Input**: BMP files (8-bit palettized, 24-bit BGR, or 32-bit BGRA)
- **Palette**: 8-bit palettized BMP (palette count is automatically detected)
- **Output**: 8-bit palettized BMP

## Credits

- **Dithering algorithms and color space code**: Based on [qualetize](https://github.com/Aikku93/qualetize) by [Aikku93](https://github.com/Aikku93)
- **Original qualetize author**: Ruben Nunez (Aikku93)

## License

The dithering and color space conversion code is derived from qualetize, which is released under the Unlicense.

