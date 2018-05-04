/*
  Hat tip to: http://fy.chalmers.se/~appro/LD_*-gallery/libhost.c
*/

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <override_lookup.h>

struct hostent *ogethostbyname(const char *name);
struct hostent *ogethostbyname2(const char *name, int af);
struct hostent *ogetipnodebyname(const char *name, int af,
    int flags, int *error_num);

int ogetaddrinfo(const char *nodename, const char *servname,
    const struct addrinfo *hints, struct addrinfo **res);

const char *hostoverride(const char *name, char *buf, size_t bufsize);

#define DYLD_INTERPOSE(_replacment,_replacee) \
  __attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
  __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };

DYLD_INTERPOSE(ogethostbyname, gethostbyname)
DYLD_INTERPOSE(ogethostbyname2, gethostbyname2)
DYLD_INTERPOSE(ogetaddrinfo, getaddrinfo)
DYLD_INTERPOSE(ogetipnodebyname, getipnodebyname)

/*
 * gethostbyname
 */
struct hostent *ogethostbyname(const char *name) {
  char tmp[1024];
  const char *newname = hostoverride(name, tmp, sizeof(tmp));;

  return gethostbyname(newname);
}

/*
 * gethostbyname2
 */
struct hostent *ogethostbyname2(const char *name, int af) {
  char tmp[1024];
  const char *newname = hostoverride(name, tmp, sizeof(tmp));;

  return gethostbyname2(newname, af);
}

/*
 * getipnodebyname
 */
struct hostent *ogetipnodebyname(const char *name, int af,
    int flags, int *error_num) {
  char tmp[1024];
  const char *newname = hostoverride(name, tmp, sizeof(tmp));

  return getipnodebyname(newname, af, flags, error_num);
}

/*
 * getaddrinfo
 */
int ogetaddrinfo(const char *nodename, const char *servname,
    const struct addrinfo *hints, struct addrinfo **res) {
  char tmp[1024];
  const char *newname = hostoverride(nodename, tmp, sizeof(tmp));

  return getaddrinfo(newname, servname, hints, res);
}

/*
 * Returns overridden name if host is overridden, original name otherwise.
 * Reads environment variable HOSTOVERRIDE, which should be of the form:
 *   foo=1.1.2.2 foo.baz=2.3.3.4 bar=4.5.6.3
 *
 * NOTE The current implementation *does not* populate buf at the beginning.
 * It may be desirable to change this in the future.
 */
const char *hostoverride(const char *name, char *buf, size_t bufsize) {
  const char *override = getenv("HOSTOVERRIDE");
  if (!override) {
    return name;
  }

  return override_lookup(name, buf, bufsize, override, ' ', '=');
}

