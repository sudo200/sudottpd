#ifndef ___SERVER_H__
#define ___SERVER_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates a new TCP-Server listening over IPv4.
 * @param addr The four bytes of an IPv4-address to bind to, or NULL if listening on all interfaces.
 * @param port The port to listen on.
 * @param queue The queue length of the socket.
 * @return The socket fd, or -1 if an error occured.
 */
int mkserver_inet(const uint8_t addr[4], const uint16_t port, const int queue);

/**
 * Creates a new TCP-Server listening over a unix domain socket.
 * @param path The path to the unix domain socket.
 * @param queue The queue length of the socket.
 * @return THe socket fd, or -1 if an error occured.
 */
int mkserver_unix(const char *path, const int queue);

#endif//___SERVER_H__

