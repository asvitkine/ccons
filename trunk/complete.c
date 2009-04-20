#include "complete.h"

#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/param.h>

static char next_prefix_char(DIR *d, const char *prefix, unsigned prefix_length)
{
	struct dirent *entry;
	char nextch = '\0';
	rewinddir(d);
	for (entry = readdir(d); entry; entry = readdir(d)) {
		if (!strncmp(entry->d_name, prefix, prefix_length)) {
			nextch = entry->d_name[prefix_length];
			break;
		}
	}
	if (nextch != '\0') {
		rewinddir(d);
		for (entry = readdir(d); entry; entry = readdir(d)) {
			if (!strncmp(entry->d_name, prefix, prefix_length)) {
				if (entry->d_name[prefix_length] != nextch) {
					nextch = '\0';
					break;
				}
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
						if (prefix[prefix_len] == '\0')
							break;
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
