#include "mylibrary.h"

/* Break */
void br(void) {
	fprintf(stdout, "\n");
	return;
}

/* Handle fatal error */
void err_fatal(char* message) {
	fprintf(stdout, "%s, errno=%d\n", message, errno);
	perror("");
	exit(1);
}

/* Handle non fatal error */
void err_continue(char* message) {
	fprintf(stdout, "%s, errno=%d\n", message, errno);
	perror("");
	return;
}

/* Check the number of arguments */
void checkArg(int nArgs, int tot) {
	if (nArgs != tot) {
		fprintf(stderr, "ERROR. %d arguments are needed\n", tot);
		exit(1);
	}
}

/* Check if the port number is correct */
void check_uint16_t(char* buf, uint16_t* port) {
	if (sscanf(buf, "%" SCNu16, port) != 1) {
		err_fatal("ERROR. Invalid port number \n");
	}
	return;
}

/* Checks if the user inputs "end" or "stop" */
int isEndOrStop(char *buf) {
	return (!strcmp(buf, "end") || !strcmp(buf, "stop"));
}

/* Get IP address from arguments */
void setIParg(char* string, struct in_addr* addr) {
	Inet_aton(string, addr);
	return;
}

/* Get IP address from input */
void setIPin(struct in_addr* addr) {
	char buf[BUFLEN];
	getLine(buf, BUFLEN, "Enter host IPv4 address (dotted notation): ");
	Inet_aton(buf, addr);
	return;
}

/* Get port number from arguments */
uint16_t setPortarg(char* string, uint16_t* port_h) {
	check_uint16_t(string, port_h);
	return htons(*port_h);
}

/* Get port number from input */
uint16_t setPortin(uint16_t* port_h) {
	char buf[BUFLEN];
	getLine(buf, BUFLEN, "Enter port number: ");
	check_uint16_t(buf, port_h);
	return htons(*port_h);
}

/* Show an address written in dotted decimal notation */
void showAddress(struct sockaddr_in *sa) {
	uint32_t ip_addr;
  unsigned char *ip_str;

	ip_addr  = sa->sin_addr.s_addr; 
	ip_str 	= (unsigned char *) &ip_addr;
	
	fprintf(stdout, "%d.", ip_str[0] );
	fprintf(stdout, "%d.", ip_str[1] );
	fprintf(stdout, "%d.", ip_str[2] );
	fprintf(stdout, "%d:", ip_str[3] );
	fprintf(stdout, "%u\n", ntohs(sa->sin_port));
}

/* Initialization (only for Windows) */
void SockStartup(void) {
	return;
}

/* Release resources (only for Windows) */
void SockCleanup(void) {
	return;
}

/**************************************************
 *************SOCKET Functions*********************
 *************************************************/

/* Create a socket */
SOCKET Socket(int family, int type, int protocol) {
	SOCKET s;
	
	s = socket(family, type, protocol);
	if (s == INVALID_SOCKET) {
		err_fatal("ERROR. socket() failed");
		return 1;
	}
	
	return s;
}

/* Send connection request */
void Connect(int socket, const struct sockaddr *destaddr, socklen_t addrlen)  {
	int result;
	
	result = connect(socket, destaddr, addrlen);
	if (result == -1) {
		err_fatal("ERROR. connect() failed");
	}
	
	return;
}

/* Assign a local network address to a socket */
void Bind(int socket, const struct sockaddr *addr, socklen_t addrlen)  {
	int result;
	
	result = bind(socket, addr, addrlen);
	if (result == -1) {
		err_fatal("ERROR. bind() failed");
	}
	
	return;
}

/* Listen to connection requests */
void Listen(int socket, int backlog)  {
	int result;
	
	result = listen(socket, backlog);
	if (result == -1) {
		err_fatal("ERROR. listen() failed");
	}
	
	return;
}

/* Accept connection request */
SOCKET Accept(int socket, struct sockaddr *srcaddr, socklen_t *addr_len) {
	
	SOCKET s;
	
	s = accept(socket, srcaddr, addr_len);
	if (s == -1) {
		err_fatal("ERROR. accept() failed");
		return 1;
	}
	
	return s;
}

/* Set the address in the in_addr structure */
void Inet_aton(const char *str, struct in_addr *addr) {
	int result;
	
	result = inet_aton(str, addr);
	if (result == 0) {
		err_fatal("Invalid IPv4 address for server");
	}
	
	return;
}

/* Send data on a connection */
void Send(SOCKET s, const void* data, size_t datalen, int flags) {
	
	if (send(s, data, datalen, flags) != (ssize_t)datalen) {
		err_fatal("ERROR. send() failed");
	}
	
	return;
}

/* Receive data on a connection */
ssize_t Recv(SOCKET s, void* data, size_t datalen, int flags) {

	ssize_t n;

	if ((n = recv(s, data, datalen, flags)) < 0) {
		err_fatal("ERROR. recv() failed");
	}
	
	return n;
}

/* Send a datagram through UDP */
void Sendto (int fd, void* buf, size_t nbytes, int flags, const struct sockaddr* sa, socklen_t len) {
	if (sendto(fd, buf, nbytes, flags, sa, len) != (ssize_t )nbytes) {
		err_fatal("ERROR. sendto() failed");
	}
	return;
}

/* Add I/O multiplexing behaviour */
int Select(int nfd, fd_set* readfile, fd_set* writefile, fd_set* exceptfd, struct timeval* timeOut) {
	int n;
	n = select(nfd, readfile, writefile, exceptfd, timeOut);
	if (n == -1) {
		err_fatal("ERROR. select() failed");
	}
	return n;
}

/* Receive a datagram through UDP */
int Recvfrom(int socket, void* buffer, size_t len, int flags, struct sockaddr* from, socklen_t* addrlen) {
	int n;
	n = recvfrom(socket, buffer, len, flags, from, addrlen);
	if (n == -1) {
		err_fatal("ERROR. recvfrom() failed");
	}
	return n;
}

/**************************************************
 ***************UNIX Functions*********************
 *************************************************/

int Open(const char* filename, int oflags) {
	int n;
	n = open(filename, oflags);
	if (n == -1) {
		err_fatal("ERROR. open() failed");
	}
	return n;
}


ssize_t Read (int fd, void* buf, size_t nbytes) {
	ssize_t n;
	n = read(fd, buf, nbytes);
	if (n == -1) {
		err_fatal("ERROR. read() failed");
	}
	
	return n;
}


ssize_t Write (int fd, void *buf, size_t nbytes) {
	ssize_t n;
	n = write(fd, buf, nbytes);
	if (n == -1) {
		err_fatal("ERROR. write() failed");
	}
	if (n != nbytes) {
		err_fatal("ERROR. write() failed, n != nbytes");
	}
		
	return n;
}

int Close(int fd) {
	int n;
	n = close(fd);
	if (n == -1) {
		err_fatal("ERROR. close() failed");
	}
	
	return 0;
}

void Shutdown (int fd, int how) {
	int n;
	n = shutdown(fd,how);
	if (n != 0) {
		err_fatal("ERROR. shutdown() failed");
	}
	return;
}

pid_t Fork (void) {
	pid_t pid;
	if ((pid=fork()) < 0)
		err_fatal("ERROR. fork() failed");
	return pid;
}

FILE* Fopen(const char *filename, const char *mode) {
	FILE* fp;

	if ((fp = fopen(filename, mode)) == NULL) {
		err_fatal("ERROR. fopen() failed");
	}

	return(fp);
}

void Fclose(FILE *fp) {
	if (fclose(fp) != 0) {
		err_fatal("ERROR. fclose() failed");
	}
	
	return;
}

char* Fgets(char *buf, int n, FILE* stream) {
	char* rbuf;

	if ((rbuf = fgets(buf, n, stream)) == NULL) {
		err_fatal("ERROR. fgets() failed");
	}

	return (rbuf);
}

void Fputs(const char *buf, FILE *stream) {
	if (fputs(buf, stream) == EOF) {
		err_fatal("ERROR. fputs() failed");
	}
	
	return;
}

size_t Fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	int n;
	n = fread(ptr, size, nmemb, stream);
	if (n == 0) {
		err_fatal("ERROR. fread() failed");
	}
	
	return n;
}

size_t Fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
	int n;
	n = fwrite(ptr, size, nmemb, stream);
	if (n == 0) {
		err_fatal("ERROR. fread() failed");
	}
	
	return n;
}

/**************************************************
 ************Communication Functions***************
 *************************************************/

/* Read nbytes from file fd into buffer buf */
ssize_t readn(int fd, char* buf, size_t nbytes) {
	size_t nleft = nbytes;
	ssize_t nread;
	char* lbuf = buf;
	
	while (nleft>0) {
		nread = read(fd, lbuf, nleft);
		if (nread > 0) {
			nleft -= nread;
			lbuf += nread;
		} else if (nread == 0) { 
				// EOF
				break;
		} else {	
				// error
				return (nread);
		}
	}
	
	return nread;
}

void Readn(int fd, char* buf, size_t nbytes) {
	if (readn(fd, buf, nbytes) != nbytes) {
		err_fatal("ERROR. readn() failed");
	}
	
	return;
}

/* Receive nbytes from socket s buffer */
ssize_t recvn(SOCKET s, char* buf, size_t nbytes) {
	size_t nleft = nbytes;
	ssize_t nread;
	char* lbuf = buf;
	
	while (nleft>0) {
		nread = recv(s, lbuf, nleft, 0);
		if (nread > 0) {
			nleft -= nread;
			lbuf += nread;
		} else if (nread == 0) { 
				// Connection closed by party
				break;
		} else {	
				// Error
				return (nread);
		}
	}
	
	return nread;
}

void Recvn(SOCKET s, char* buf, size_t nbytes) {
	if (recvn(s, buf, nbytes) != nbytes) {
		err_fatal("ERROR. recvn() failed");
	}
	
	return;
}

/* Print a string on the console, read a line, substitutes EOL with '\0' and empties the input buffer */
int getLine(char* line, size_t maxline, char* string) {
	char	ch;
	size_t 	i;

	fprintf(stdout, "%s", string);
	for (i=0; i<maxline-1 && (ch = getchar()) != '\n' && ch != EOF; i++) {
		*line++ = ch;
	}
	*line = '\0';
	
	while (ch != '\n' && ch != EOF) {
		ch = getchar();
	}
	
	if (ch == EOF) {
		return(EOF);
	} else { 
		return 1; 
	}
}

/* Reads a line from stream socket s to buffer buf, including the final '\n' */
ssize_t readlineS (SOCKET s, char *buf, size_t maxlen) {
    size_t n;
    ssize_t nread;
    char c;

    for (n=1; n<maxlen; n++) {
			nread=recv(s, &c, 1, 0);
			
			if (nread == 1) {
	    	*buf++ = c;
	    	if (c == '\n') {
					break;
				}
	    } else if (nread == 0) {	
	    		// Connection closed by party
	    		*buf = 0;
	    		return (n-1);
			} else {			
					// Error
	    		return -1;
	    }
    }
    
    *buf = 0;
    return n;
}

ssize_t ReadlineS(SOCKET s, char *buf, size_t maxlen) {
	ssize_t n;

	if ((n = readlineS(s, buf, maxlen)) < 0) {
		err_fatal("ERROR. readlineS() failed");
	}
	return n;
}

/* Writes nbytes from buffer buf to file fd */
ssize_t writen(int fd, char* buf, size_t nbytes) {
    size_t  nleft = nbytes;
    ssize_t nwritten;
    char* lbuf = buf;

    while (nleft > 0) {
			nwritten = write(fd, lbuf, nleft);
			if (nwritten <=0) {
	    	return (nwritten);
			} else {
	    	nleft -= nwritten;
	    	lbuf += nwritten;   
			}
    }
    
    return (nbytes - nleft);
}

void Writen (int fd, char* buf, size_t nbytes) {
	if (writen(fd, buf, nbytes) != nbytes) {
		err_fatal("ERROR. writen() failed");
	}
	
	return;
}

/* Send nbytes from buffer buf to stream socket s */
ssize_t sendn(SOCKET s, char* buf, size_t nbytes) {
    size_t  nleft = nbytes;
    ssize_t nwritten;
    char* lbuf = buf;

    while (nleft > 0) {
			nwritten = send(s, lbuf, nleft, 0);
			if (nwritten <=0) {
	    	return (nwritten);
			} else {
	    	nleft -= nwritten;
	    	lbuf += nwritten;   
			}
    }
    
    return (nbytes - nleft);
}

void Sendn (SOCKET s, char* buf, size_t nbytes) {
	if (sendn(s, buf, nbytes) != nbytes) {
		err_fatal("ERROR. sendn() failed");
	}
	
	return;
}

/**************************************************
 *********************Signals**********************
 *************************************************/
 
void* Signal(int sig, void* sigFunc) {
	void* sigH;
	if ((sigH = signal(sig, sigFunc)) == SIG_ERR) {
		err_fatal("ERROR. signal() failed");
	}
	return(sigH);
}
