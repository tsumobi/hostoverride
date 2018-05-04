/*
  Hat tip to: http://fy.chalmers.se/~appro/LD_*-gallery/libhost.c
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <override_lookup.h>

size_t MAX_CONNECTOVERRIDE_LEN = 2048;

int oconnect(int socket, const struct sockaddr *address,
    socklen_t address_len);

int rindex_of(const char *s, char c);

#define DYLD_INTERPOSE(_replacment,_replacee) \
  __attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
  __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };

DYLD_INTERPOSE(oconnect, connect)

/*
 * Overrides connect
 *
 * Reads environment variable CONNECTOVERRIDE, which should be of the form:
 *   127.56.42.26:80=127.0.0.1:8080 ::ffff:127.169.203.186:25=::ffff:127.0.0.1:20145
 *
 * It will override any connect requests where *sa includes an address/port pair
 * found in the CONNECTOVERRIDE environment variable.
 */
int oconnect(int socket, const struct sockaddr *sa, socklen_t address_len) {
  const char *override = getenv("CONNECTOVERRIDE");

  if (override) {
    int orig_port;
    // INET6_ADDRSTRLEN is the max expected length of an address
    char orig_address[INET6_ADDRSTRLEN], buf[MAX_CONNECTOVERRIDE_LEN];
    // Max expected length of "ip:port"
    char key[(INET6_ADDRSTRLEN) + 7] = { 0 };
    const char *result;

    struct sockaddr_in *sa_in, private_sa_in = { 0 };
    struct sockaddr_in6 *sa_in6, private_sa_in6 = { 0 };
    // Used to keep track of which sockaddr we'll be using.
    void *new_sa;

    switch (sa->sa_family) {
    case AF_INET:
      sa_in = (struct sockaddr_in *)sa;
      inet_ntop(AF_INET, &sa_in->sin_addr, orig_address, INET_ADDRSTRLEN);
      orig_port = ntohs(sa_in->sin_port);

      new_sa = &private_sa_in;
      break;
    case AF_INET6:
      sa_in6 = (struct sockaddr_in6 *)sa;
      inet_ntop(AF_INET6, &sa_in6->sin6_addr, orig_address, INET6_ADDRSTRLEN);
      orig_port = ntohs(sa_in6->sin6_port);

      new_sa = &private_sa_in6;
      break;
    default:
      orig_port = 0;
      break;
    }

    sprintf(key, "%s:%d", orig_address, orig_port);
    result = override_lookup(key, buf, sizeof(buf), override, ' ', '=');

    // Rely on the fact that override_lookup returns key if no match found
    if (result != key) {
      char new_address[INET6_ADDRSTRLEN];
      int colon_index, new_port;

      // Give up if we find no colon (likely user error)
      if ( (colon_index = rindex_of(result, ':')) == -1 ) {
        return connect(socket, sa, address_len);
      }

      strlcpy(new_address, result, colon_index + 1);
      new_port = atoi(&result[colon_index + 1]);

      // Copy wholesale the provided (struct sockaddr) *sa
      memcpy(new_sa, sa, address_len);

      switch (sa->sa_family) {
        case AF_INET:
          inet_pton(AF_INET, new_address, &private_sa_in.sin_addr);
          private_sa_in.sin_port = htons(new_port);

          break;
        case AF_INET6:
          inet_pton(AF_INET6, new_address, &private_sa_in6.sin6_addr);
          private_sa_in6.sin6_port = htons(new_port);

          break;
        default:
          break;
      }

      return connect(socket, (const struct sockaddr *) new_sa, address_len);
    }
  }

  return connect(socket, sa, address_len);
}

int rindex_of(const char *s, char c) {
  int i = strlen(s);
  while (i >= 0 && s[i] != c) {
    --i;
  }

  return i;
}
