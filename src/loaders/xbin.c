//
//  xbin.c
//  AnsiLove/C
//
//  Copyright (c) 2011-2018 Stefan Vogt, Brian Cassidy, and Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 2-Clause License.
//  See the LICENSE file for details.
//

#include "../ansilove.h"

int ansilove_xbin(struct ansilove_ctx *ctx, struct ansilove_options *options)
{
	const unsigned char *font_data;
	unsigned char *font_data_xbin = NULL;

	if (strncmp((char *)ctx->buffer, "XBIN\x1a", 5) != 0) {
		ctx->error = FORMAT_ERROR;
		return -1;
	}

	int32_t xbin_width = (ctx->buffer[6] << 8) + ctx->buffer[5];
	int32_t xbin_height = (ctx->buffer[8] << 8) + ctx->buffer[7];
	int32_t xbin_fontsize = ctx->buffer[9];
	int32_t xbin_flags = ctx->buffer[10];

	gdImagePtr canvas;

	canvas = gdImageCreate(8 * xbin_width, xbin_fontsize * xbin_height);

	if (!canvas) {
		ctx->error = GD_ERROR;
		return -1;
	}

	// allocate black color
	gdImageColorAllocate(canvas, 0, 0, 0);

	uint32_t colors[16];
	uint32_t offset = 11;

	// palette
	if ((xbin_flags & 1) == 1) {
		int32_t loop;
		int32_t index;

		for (loop = 0; loop < 16; loop++) {
			index = (loop * 3) + offset;

			colors[loop] = gdImageColorAllocate(canvas, (ctx->buffer[index] << 2 | ctx->buffer[index] >> 4),
			    (ctx->buffer[index + 1] << 2 | ctx->buffer[index + 1] >> 4),
			    (ctx->buffer[index + 2] << 2 | ctx->buffer[index + 2] >> 4));
		}

		offset += 48;
	} else {
		for (int i = 0; i < 16; i++) {
			colors[i] = gdImageColorAllocate(canvas, binary_palette[i*3],
			    binary_palette[i*3+1],
			    binary_palette[i*3+2]);
		}
	}

	// font
	if ((xbin_flags & 2) == 2) {
		int32_t numchars = (xbin_flags & 0x10 ? 512 : 256);

		// allocate memory to contain the XBin font
		font_data_xbin = (unsigned char *)malloc(xbin_fontsize * numchars);
		if (font_data_xbin == NULL) {
			ctx->error = MEMORY_ERROR;
			return -1;
		}
		memcpy(font_data_xbin, ctx->buffer+offset, (xbin_fontsize * numchars));

		font_data = font_data_xbin;

		offset += (xbin_fontsize * numchars);
	} else {
		// using default 80x25 font
		font_data = font_pc_80x25;
	}

	int32_t column = 0, row = 0;
	int32_t character, attribute, foreground, background;

	// read compressed xbin
	if ((xbin_flags & 4) == 4) {
		while (offset < ctx->length && row != xbin_height) {
			int32_t ctype = ctx->buffer[offset] & 0xC0;
			int32_t counter = (ctx->buffer[offset] & 0x3F) + 1;

			character = -1;
			attribute = -1;

			offset++;
			while (counter--) {
				// none
				if (ctype == 0) {
					character = ctx->buffer[offset];
					attribute = ctx->buffer[offset + 1];
					offset += 2;
				}
				// char
				else if (ctype == 0x40) {
					if (character == -1) {
						character = ctx->buffer[offset];
						offset++;
					}
					attribute = ctx->buffer[offset];
					offset++;
				}
				// attr
				else if (ctype == 0x80) {
					if (attribute == -1) {
						attribute = ctx->buffer[offset];
						offset++;
					}
					character = ctx->buffer[offset];
					offset++;
				}
				// both
				else {
					if (character == -1) {
						character = ctx->buffer[offset];
						offset++;
					}
					if (attribute == -1) {
						attribute = ctx->buffer[offset];
						offset++;
					}
				}

				background = (attribute & 240) >> 4;
				foreground = attribute & 15;

				drawchar(canvas, font_data, 8, 16, column, row, colors[background], colors[foreground], character);

				column++;

				if (column == xbin_width) {
					column = 0;
					row++;
				}
			}
		}
	} else {
		// read uncompressed xbin
		while (offset < ctx->length && row != xbin_height) {
			if (column == xbin_width) {
				column = 0;
				row++;
			}

			character = ctx->buffer[offset];
			attribute = ctx->buffer[offset+1];

			background = (attribute & 240) >> 4;
			foreground = attribute & 15;

			drawchar(canvas, font_data, 8, xbin_fontsize, column, row, colors[background], colors[foreground], character);

			column++;
			offset += 2;
		}
	}

	// create output file
	output(ctx, options, canvas);

	// nuke garbage
	free(font_data_xbin);

	return 0;
}
