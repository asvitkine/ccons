//
// Functions which provide file system path auto-completion.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "complete.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>

// Returns the next auto-completion character, or '\0' if there isn't one.
// A character will only be returned if can complete the prefix and there
// are no other such matches.
static char next_prefix_char(DIR *d, const char *prefix, unsigned prefix_length)
{
	struct dirent *entry;
	char nextch = '\0';
	rewinddir(d);
	for (entry = readdir(d); entry; entry = readdir(d)) {
		if (!strncmp(entry->d_name, prefix, prefix_length)) {
			if (nextch == '\0') {
				nextch = entry->d_name[prefix_length];
			} else if (nextch != entry->d_name[prefix_length]) {
				// Multiple matches => no completion.
				nextch = '\0';
				break;
			}
		}
	}
	return nextch;
}

unsigned complete(const char *path, char *suggest, unsigned suggest_length)
{
	const char *dir, *base;
	char pathcopy[MAXPATHLEN];

	*suggest = 0;

	if (path[strlen(path) - 1] == '/')
		return 0;

	strncpy(pathcopy, path, sizeof(pathcopy));
	pathcopy[sizeof(pathcopy) - 1] = '\0';

	dir = dirname(pathcopy);
	base = basename(pathcopy);
	if (dir && base) {
		size_t len = strlen(base);
		DIR *dd = opendir(dir);
		if (dd != NULL) {
			struct dirent *entry;
			for (entry = readdir(dd); entry; entry = readdir(dd)) {
				if (!strncmp(entry->d_name, base, len)) {
					char prefix[MAXPATHLEN];
					unsigned prefix_len = len;
					strncpy(prefix, base, sizeof(prefix) - 1);
					prefix[sizeof(prefix) - 1] = '\0';
					while (prefix_len < sizeof(prefix)) {
						prefix[prefix_len] = next_prefix_char(dd, prefix, prefix_len);
						if (prefix[prefix_len] == '\0') {
							if (prefix_len > 0 && prefix[prefix_len - 1] != '/') {
								struct stat f;
								char newpath[MAXPATHLEN];
								snprintf(newpath, sizeof(newpath), "%s%s", path, prefix + len);
								if (!stat(newpath, &f) && S_ISDIR(f.st_mode)) {
									prefix[prefix_len] = '/';
									prefix[++prefix_len] = '\0';								
								}
							}
							break;
						}
						prefix[++prefix_len] = '\0';
					}
					strncpy(suggest, prefix + len, suggest_length - 1);
					suggest[suggest_length - 1] = '\0';
					closedir(dd);
					return prefix_len - len;
				}
			}
			closedir(dd);
		}
	}

	return 0;
}
