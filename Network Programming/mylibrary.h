#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/xdr.h>

#define BUFLEN 255
#define MAXBUF 1024
#define FILELEN 1023
#define TIMEOUT 10
#define INVALID_SOCKET -1
#define closesocket(x) close(x)

typedef int SOCKET;

/* Utility functions */
void br(void);
void err_fatal(char*);
void err_continue(char*);
void checkArg(int, int);
void check_uint16_t(char*, uint16_t*);
int isEndOrStop(char*);
void setIParg(char*, struct in_addr*);
void setIPin(struct in_addr*);
uint16_t setPortarg(char*, uint16_t*);
uint16_t setPortin(uint16_t*);
void showAddress(struct sockaddr_in*);
void SockStartup(void);
void SockCleanup(void);

/* Standard SOCKET functions + error check */
SOCKET Socket(int, int, int);
void Connect(int, const struct sockaddr*, socklen_t);
void Bind(int, const struct sockaddr*, socklen_t);
void Listen(int, int);
SOCKET Accept(int, struct sockaddr*, socklen_t*);
void Inet_aton(const char*, struct in_addr*);
void Send(SOCKET, const void*, size_t, int);
ssize_t Recv(SOCKET, void*, size_t, int);
void Sendto (int, void*, size_t, int, const struct sockaddr*, socklen_t);
int Select(int, fd_set*, fd_set*, fd_set*, struct timeval*);
int Recvfrom(int, void*, size_t, int, struct sockaddr*, socklen_t*);

/* Standard UNIX functions + error check */
int Open(const char*, int);
ssize_t Read (int, void*, size_t);
ssize_t Write (int, void*, size_t);
int Close(int);
void Shutdown (int, int);
pid_t Fork (void);
FILE* Fopen(const char*, const char*);
void Fclose(FILE*);
char* Fgets(char*, int, FILE*);
void Fputs(const char*, FILE*);
size_t Fread(void*, size_t, size_t, FILE*);
size_t Fwrite(const void*, size_t, size_t, FILE*);

/* Communication functions */
ssize_t readn(int, char*, size_t);
void Readn(int, char*, size_t);
ssize_t recvn(SOCKET, char*, size_t);
void Recvn(SOCKET, char*, size_t);
int getLine(char*, size_t, char*);
ssize_t readlineS (SOCKET, char*, size_t);
ssize_t ReadlineS (SOCKET, char*, size_t);
ssize_t writen(int, char*, size_t);
void Writen (int, char*, size_t);
ssize_t sendn(SOCKET, char*, size_t);
void Sendn (SOCKET, char*, size_t);

/* Signals */
void* Signal(int, void*);
