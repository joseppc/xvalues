/*
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *   (c) 2018-2022 Josep Puigdemont <josep.puigdemont@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0+
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#define KB 1024ULL
#define MB 1024ULL * KB
#define GB 1024ULL * MB
#define TB 1024ULL * GB
#define PB 1024ULL * TB
#define EB 1024ULL * PB

enum multipliers {BYTE = 0, KILO, MEGA, GIGA, TERA, PETA, EXI};

static const struct {
	char suffix;
	int width_hex;
	int width_dec;
	uint64_t multi;
} data[] = {{'b',  4,  4, 1},
	    {'K',  8,  7, 1 * KB},
	    {'M',  8, 10, 1 * MB},
	    {'G', 12, 13, 1 * GB},
	    {'T', 16, 16, 1 * TB},
	    {'P', 16, 19, 1 * PB},
	    {'E', 16, 20, 1 * EB}};

static char zero_ch = ' ';

static void print_binary(uint64_t v)
{
	char str[65];
	int len;

	if (v <= 0xfULL)
		len = 4;
	else if (v <= 0xffULL)
		len = 8;
	else if (v <= 0xffffULL)
		len = 16;
	else if (v <= 0xffffffffULL)
		len = 32;
	else
		len = 64;

	str[len] = '\0';
	while (len--) {
		if (v & 1ULL)
			str[len] = '1';
		else
			str[len] = zero_ch;
		v >>= 1;
	}
	printf("  %s", str);
}

static enum multipliers get_multiplier(uint64_t v)
{
	if (v < 1 * KB)
		return BYTE;
	else if (v < 1 * MB)
		return KILO;
	else if (v < 1 * GB)
		return MEGA;
	else if (v < 1 * TB)
		return GIGA;
	else if (v < 1 * PB)
		return TERA;
	else if (v < 1 * EB)
		return PETA;
	else
		return EXI;
}

static void print_size(uint64_t v)
{
	enum multipliers multi;

	multi = get_multiplier(v);

	printf("%6.1f%c", (v * 1.0) / data[multi].multi, data[multi].suffix);
}

static void print_number(uint64_t v, enum multipliers width, int show_bin)
{
	printf("0x%0*" PRIx64 " %*" PRIu64 " ",
	       data[width].width_hex, v, data[width].width_dec, v);
	print_size(v);
	if (show_bin)
		print_binary(v);
	printf("\n");
}

static int parse_binary(const char *const s, uint64_t *const v)
{
	uint64_t mask = 1;

	if (strlen(s) > 66) {
		fprintf(stderr, "Binary number too big, max 64 bits.\n");
		return -1;
	}

	*v = 0;

	for (int i = strlen(s) - 1; i > 1; i--) {
		if (s[i] == '1') {
			*v |= mask;
		} else if (s[i] != '0') {
			fprintf(stderr, "Binary numbers can only contain 0 or 1.\n");
			return -1;
		}
		mask <<= 1;
	}

	return 0;
}

static int get_value(const char *const s, uint64_t *const v)
{
	char *ptr;

	if (strlen(s) > 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B'))
		return parse_binary(s, v);

	*v = strtoull(s, &ptr, 0);
	if (*ptr != '\0') {
		switch (*ptr) {
		case 'K':
			/* fall through */
		case 'k':
			*v *= data[KILO].multi;
			break;
		case 'M':
			/* fall through */
		case 'm':
			*v *= data[MEGA].multi;
			break;
		case 'G':
			/* fall through */
		case 'g':
			*v *= data[GIGA].multi;
			break;
		case 'T':
			/* fall through */
		case 't':
			*v *= data[TERA].multi;
			break;
		case 'P':
			/* fall through */
		case 'p':
			*v *= data[PETA].multi;
			break;
		case 'E':
			/* fall through */
		case 'e':
			*v *= data[EXI].multi;
			break;
		default:
			fprintf(stderr, "Error in value %s:%lu\n", s, ptr - s);
			return -1;
		}
	}

	return 0;
}

static void print_string(const char *s, enum multipliers width, int show_bin)
{
	uint64_t v;

	if (get_value(s, &v))
		return;

	print_number(v, width, show_bin);
}

static void print_all(int show_bin)
{
	const uint64_t values[] = {8, 16, 64, 128, 256, 512,
				   1 * KB, 4 * KB, 16 * KB, 64 * KB,
				   1 * MB, 16 * MB, 64 * MB, 256 * MB, 512 * MB,
				   1 * GB, 4 * GB,
				   1 * TB,
				   1 * PB,
				   1 * EB,
				   0};

	for (int i = 0; values[i] != 0; i++)
		print_number(values[i], EXI, show_bin);
}

int main(int argc, char *argv[])
{
	uint64_t v;
	enum multipliers width = 0;
	int show_bin = 0;
	int first = 1;

	if (argc > 1) {
		if (strncmp(argv[1], "-b", 2) == 0) {
			show_bin = 1;
			first = 2;
			zero_ch = '.';
		} else if (strncmp(argv[1], "-B", 2) == 0) {
			show_bin = 1;
			first = 2;
			zero_ch = '0';
		}
	}

	if (first >= argc) {
		print_all(show_bin);
		exit(EXIT_SUCCESS);
	}

	for (int i = first; i < argc; i++) {
		int tmp_width;

		if (get_value(argv[i], &v))
			exit(EXIT_FAILURE);

		tmp_width = get_multiplier(v);
		if (width < tmp_width)
			width = tmp_width;
	}

	for (int i = first; i < argc; i++)
		print_string(argv[i], width, show_bin);

	exit(EXIT_SUCCESS);
}
