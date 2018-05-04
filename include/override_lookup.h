#include <string.h>

/*
 * Returns overridden value for name from overrides if found, original
 * name otherwise.
 *
 * Reads from the string overrides (of the form):
 * x{minor_seperator}y{major_separator}a{minor_separator}b...
 *
 * in this case, if name was x, y would be returned.
 * if name was z, z would be returned.
 *
 * NOTE The current implementation *does not* populate buf at the beginning.
 * It may be desirable to change this in the future.
 */

static const char *override_lookup(const char *name, char *buf, size_t bufsize,
    const char *overrides, char major_separator, char minor_separator) {
  int ix;
  int len;

  if (!name) {
    return NULL;
  }

  strlcpy(buf, overrides, bufsize);
  len = strlen(name);

  while (*buf) {
    // Search for the first \0 or minor_separator.
    ix = 0;

    while (buf[ix] && buf[ix] != minor_separator) {
      ++ix;
    }

    // If \0, we have an incomplete entry. Just give up.
    if (! buf[ix]) { break; }

    // Otherwise replace the current position with \0.
    buf[ix] = '\0';

    // Is this entry a match?
    if (!strncmp(buf, name, len)) {
      buf += ix + 1;
      ix = 0;

      // Place an ending \0 if we find a subsequent seperator.
      while (buf[ix] && buf[ix] != major_separator) {
        ++ix;
      }

      buf[ix] = '\0';

      if (*buf) {
        return *buf ? buf : name;
      }
    }

    // Find the start of the next entry.
    buf += ix + 1;
    while (*buf && *buf != major_separator) {
      ++buf;
    }
    // Move pointer to start of next entry.
    ++buf;
  }

  return name;
}
