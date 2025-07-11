#include "include.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *filename) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Error opening file: %s\n", filename);
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	rewind(f);

	char *buf = malloc(len + 1);
	if (!buf) { perror("malloc"); exit(1); }

	fread(buf, 1, len, f);
	buf[len] = '\0';

	fclose(f);
	return buf;
}

static char *read_line(FILE *f) {
	size_t size = 128, len = 0;
	char *buf = malloc(size);
	if (!buf) { perror("malloc"); exit(1); }

	int c;
	while ((c = fgetc(f)) != EOF) {
		if (len + 1 >= size) {
			size *= 2;
			buf = realloc(buf, size);
			if (!buf) { perror("realloc"); exit(1); }
		}

		buf[len++] = c;
		if (c == '\n') break;
	}

	if (len == 0 && c == EOF) {
		free(buf);
		return NULL;
	}

	buf[len] = '\0';
	return buf;
}

static void append(char **dest, const char *src) {
	if (!*dest) {
		*dest = strdup(src);
	} else {
		size_t old_len = strlen(*dest);
		size_t new_len = old_len + strlen(src) + 1;
		*dest = realloc(*dest, new_len);
		if (!*dest) { perror("realloc"); exit(1); }
		strcat(*dest, src);
	}
}

char *expand_includes(const char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "Failed to open file: %s\n", filename);
		exit(1);
	}

	char *result = NULL;
	char *line;

	while ((line = read_line(f))) {
		if (strncmp(line, "$INCLUDE", 8) == 0) {
			char *start = strchr(line, '<');
			char *end = strchr(line, '>');
			if (start && end && end > start + 1) {
				*end = '\0';
				char *include_path = start + 1;
				char *expanded = expand_includes(include_path);
				append(&result, expanded);
				free(expanded);
				free(line);
				continue;
			}
		}
		append(&result, line);
		free(line);
	}

	fclose(f);
	return result;
}
