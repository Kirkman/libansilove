//
//  savefile.c
//  AnsiLove/C
//
//  Copyright (c) 2011-2018 Stefan Vogt, Brian Cassidy, and Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 2-Clause License.
//  See the LICENSE file for details.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "ansilove.h"

int
ansilove_savefile(struct ansilove_ctx *ctx, char *output) {

	FILE *file = fopen(output, "wb");

	if (file) {
		fwrite(ctx->png.buffer, ctx->png.length, 1, file);
		fclose(file);
	} else {
		// XXX Set error code
		return -1;
	}

	return 0;
}
