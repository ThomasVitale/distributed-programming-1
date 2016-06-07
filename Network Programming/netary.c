/**********************************************************************
 *********************SOCKET API FUNCTIONS*****************************
 *********************************************************************/

/* Creating a socket */
#include <sys/socket.h>
#include <sys/types.h>
int socket(int family, int type, int protocol); // -> socket file descriptor

/* Assigning a local network address to a socket */
int bind(int socket, const struct sockaddr *addr, socklen_t addrlen);

/* Struct sockaddr */
struct sockaddr {
	unsigned short sa_family;
	char sa_data[14];
};

/* Byte order conversion (big endian representation is used in Internet) */
#include <netinet/in.h>
uint16_t htons(uint16_t x); // host to network
uint16_t ntohs(uint16_t x); // network to host
uint32_t htonl(uint32_t x);
uint32_t ntohl(uint32_t x);

/* Conversion functions for addresses */
#include <arpa/inet.h>
int inet_aton(const char *str, struct in_addr *addr); // -> 1 (ok), -> 0 (invalid
char* inet_ntoa(struct in_addr addr); // -> dotted decimal string

/* Internet address */
struct in_addr {
	unsigned long s_addr; // provided by inet_aton()
};

/* sockaddr_in (sockaddr for AF_INET) */
struct sockaddr_in {
	short sin_family; // should be AF_INET
	u_short sin_port; // provided by htons()
	struct in_addr sin_addr;
	char sin_zero[8]; // not used, must be zero
};

/* Server: listen to connection requests */
int listen(int socket, int backlog); // backlog: pending requests queue length

/* Client: send connection request */
int connect(int socket, const struct sockaddr *destaddr, socklen_t addrlen);

/* Server: accept connection request */
int accept(int socket, struct sockaddr *srcaddr, socklen_t *addr_len); // -> local id of connected socket

/* Send data on a connection */
ssize_t send(int socket, const void* data, size_t datalen, int flags); // -> n. of sent bytes

/* Receive data on a connection */
ssize_t recv(int socket, void* buffer, size_t buffer, int flags); // -> 0 (closed connection), n. of received bytes

/* Close a connection */
int close(int fd);
int closesocket(int socket);
int shutdown(int socket, int how); // SHUT_RD, SHUT_WR, SHUT_RDWR

/* Send a datagram */
ssize_t sendto(int socket, const void* data, size_t datalen, int flags, const struct sockaddr* to, socklen_t addrlen);

/* Receive a datagram */
int recvfrom(int socket, void* buffer, size_t buflen, int flags, struct sockaddr* from, size_t addrlen);

/* Select and set of sockets */
int select(int nfd, fd_set* readfile, fd_set* writefile, fd_set* exceptfd, const struct timeval *timeOut);
void FD_SET(int s, fd_set* fd);
void FD_CLEAR(int s, fd_set* fd);
void FD_ZERO(fd_set* fd);
void FD_ISSET(int s, fd_set* fd);

/* Getting information about a socket state */
int getsockname(int s, struct sockaddr* addr, socklen_t* addrlen); // local address
int getpeername(int s, struct sockaddr* addr, socklen_t* addrlen); // remote address
int getsockopt(int s, int level, int opt_name, void* optval, socklen_t* optlen);
int setsockopt(int s, int level, int opt_name, void* optval, socklen_t optlen);

/* DNS Access and Name Resolution */
struct hostent *gethostbyname(const char* hostname); // -> NULL (deprecated)
struct hostent *gethostbyaddress(const char* addr, socklen_t len, int family); // -> NULL (use in_addr)

/* The hostent structure */
struct hostent {
	char* h_name;
	char** h_aliases;
	int h_addrtype;
	int h_length;
	char** h_addr_list;
};

/* IPv4 and IPv6 Name Resolution */
int getaddrinfo(
	const char* hostname,
	const char* service,
	const struct addrinfo* hints, // NULL, addrinfo
	struct addrinfo** result); // -> 0, errCode
	
/* The addrinfo structure */
struct addrinfo {
	int ai_flags; // AI_PASSIVE, AI_CANONNAME, AI_v4MAPPED
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	socklen_t ai_addrlen;
	char* ai_canonname;
	struct sockaddr* ai_addr;
	struct addrinfo* ai_next;
};

/* Looking for registered servers */
struct servent* getservbyname(const char* servname, const char* protoname); // -> NULL
struct servent* getservbyport(int port, const char* protoname); // -> NULL

/* The servent structure */
struct servent(
	char* s_name;
	char** h_aliases;
	int s_port;
	char* s_proto;
};

/**********************************************************************
 ****************************XDR FUNCTIONS*****************************
 *********************************************************************/
 
#include <rpc/xdr.h>
 
/*-------Buffer Paradigm------------*/

XDR xdrs; // XDR stream

/* Create stream */
void xdrmem_create(XDR *xdrs, char *addr, unsigned int size, enum xdr_op op);
// XDR_ENCODE, XDR_DECODE, XDR_FREE

/* Destroy stream */
void xdr_destroy(XDR *xdrs);
 
/*-------Direct Connection------------*/

XDR xdrs; // XDR stream
FILE* fstream; // FILE stream

/* Create file stream */
#include <stdio.h>
FILE *fdopen(int fd, const char *mode); // fd = socket id

/* Create XDR stream */
void xdrstdio_create(XDR *xdrs, FILE *file, enum xdr_op op);
	// XDR_ENCODE, XDR_DECODE, XDR_FREE
	
/* Destroy stream */
void xdr_destroy(XDR *xdrs);

/*-------Code generation with rpcgen------------*/

types.xdr
rpcgen -h -o types.h types.xdr
rpcgen -c -o types.c types.xdr

/*-------Useful functions------------*/

/* Get the position of the XDR stream */
unsigned int xdr_getpos(XDR *xdrs);

/* Set the position of the XDR stream */
xdr_setpos(XDR *xdrs, unsigned int pos);

/* Convert integers XDR <-> C */
bool_t xdr_int(XDR *xdrs, int *ip);
