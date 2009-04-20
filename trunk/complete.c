#include "complete.h"

#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/param.h>

unsigned complete(const char *path, char *suggest, unsigned suggest_length)
{
	const char *dir, *base;
	char pathcopy[MAXPATHLEN];

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
					closedir(dd);
					strncpy(suggest, entry->d_name + len, suggest_length);
					suggest[suggest_length - 1] = '\0';
					return strlen(suggest);
				}
			}
			closedir(dd);
		}
	}

	return 0;
}
