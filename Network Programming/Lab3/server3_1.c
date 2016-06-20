#include "../mylibrary.h"

#define MSG_ERR "-ERR\r\n"
#define MSG_GET "GET"
#define MSG_OK "+OK\r\n"
#define MSG_QUIT "QUIT"

#define MAX_CHILDREN 3

int n_children;

int service(SOCKET);
void sigchldHandler(int sig);

int main(int argc, char** argv) {

	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int backlog = 2;									// pending requests queue length
	socklen_t caddr_len;							// client address length
	int wstatus;											// wait() returning status
	sigset_t x;												// signal set
	int retValue;
	pid_t pid;
	
	/* Check number of arguments */
	checkArg(argc, 2);
	
	/* Instantiate a signal handler for SIGCHLD */
	Signal(SIGCHLD, sigchldHandler);

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket...\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "- OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind the socket to a local network address */
	fprintf(stdout, "Binding the socket...\n");
	Bind(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "- OK. Socket bound.\n");

	/* Listen to connection requests */
	fprintf(stdout, "Listen at socket %d with backlog = %d...\n", s, backlog);
	Listen(s, backlog);
	fprintf(stdout, "- OK. Socket is listening on ");
	showAddress(&saddr);
	
	n_children = 0;
	
	while(1) {
	
		/* Disable signal handler to avoid race conditions */
		sigemptyset(&x);
		sigaddset(&x, SIGCHLD);
		sigprocmask(SIG_BLOCK, &x, NULL);
	
		/* If too many children, wait for one of them to finish */
		if (n_children >= MAX_CHILDREN) {
			wait(&wstatus);
			n_children--;
		}
		
		/* Re-enable signal handler */
		sigemptyset(&x);
		sigaddset(&x, SIGCHLD);
		sigprocmask(SIG_UNBLOCK, &x, NULL);
		
		br();
		
		/* Accept connection requests */
		caddr_len = sizeof(struct sockaddr_in);
		fprintf(stdout, "Waiting for connection requests...\n\n");
		s_acceptor = Accept(s, (struct sockaddr*) &caddr, &caddr_len);
		fprintf(stdout, "- New connection from client ");
		showAddress(&caddr);
		
		br();
		
		if ((pid = fork()) < 0) {
			fprintf(stderr, "- ERROR. fork() failed.\n");
			closesocket(s_acceptor);
		} else if (pid > 0) {
			// The parent
			closesocket(s_acceptor);
			n_children++;
		} else {
			// The child
			closesocket(s);
			
			retValue = service(s_acceptor);
		
			closesocket(s_acceptor);
			fprintf(stdout, "--- Connection closed by %s", (retValue == 0) ? "client\n" : "server\n");
			br();
			exit(0);
		}
	}
	
	return 0;
}

/* Update the children counter when a child terminates */
void sigchldHandler(int sig) {
	int wstatus;
	
	while(waitpid(-1, &wstatus, WNOHANG) > 0) {
		n_children--;
	}
	
	return;
}

int service(SOCKET s) {

	char buf[BUFLEN+1];								// transmitter and receiver buffer
	char c, filename[FILELEN];
	int finished, i, n;
	ssize_t nread, nwritten;
	struct stat fileinfo;
	FILE* fp;
	uint32_t nBytes, nSeconds, mTime, size;

	while(1) {
		/* Read a message */
		nread = 0;
		finished = 0;
		do {
			n = read(s, &c, sizeof(char));
			if (n == -1) {
				fprintf(stderr, "--- ERROR. read() failed.\n");
				return 0;
			} else if (n == 1) {
				buf[nread++] = c;
			} else {
				finished = 1;
			}
		} while(c != '\n' && nread < BUFLEN-1 && !finished);
		if (nread == 0) {
			return 0;
		}
	
		/* Append the \0 at the end of the string */
		buf[nread] = '\0';
		while((nread > 0) && ((buf[nread-1] == '\r') || (buf[nread-1] == '\n'))) {
			buf[nread-1] = '\0';
			nread--;
		}
	
		/* Print the message */
		fprintf(stderr, "--- Received message: %s\n", buf);
	
		/* Compute the command and reply to the client according to it */
		if (nread > strlen(MSG_GET) && strncmp(buf, MSG_GET, strlen(MSG_GET)) == 0) {
			/* If the command is GET */
			strcpy(filename, buf+4);
		
			if (stat(filename, &fileinfo) == 0) {
				// The file can be read correctly
				fp = fopen(filename, "rb");
				if (fp == NULL) {
					fprintf(stderr, "--- ERROR. fopen() failed.\n");
					return 0;
				}
				size = (uint32_t) fileinfo.st_size;
				mTime = (uint32_t) fileinfo.st_mtime;
			
				// Write the response message
				nwritten = write(s, MSG_OK, strlen(MSG_OK)*sizeof(char));
				if (nwritten != strlen(MSG_OK)*sizeof(char)) {
					fprintf(stderr, "--- ERROR. write() of MSG_OK failed.\n");
					return 0;
				}
			
				// Write the number of bytes of the file
				nBytes = htonl(size);
				nwritten = write(s, &nBytes, sizeof(uint32_t));
				if (nwritten != sizeof(size)) {
					fprintf(stderr, "--- ERROR. write() of file size failed.\n");
					return 0;
				}
			
				// Write the timestamp of the last modification
				nSeconds = htonl(mTime);
				nwritten = write(s, &nSeconds, sizeof(uint32_t));
				if (nwritten != sizeof(mTime)) {
					fprintf(stderr, "--- ERROR. write() of file timestamp failed.\n");
					return 0;
				}
			
				// Write the file itself
				for (i=0; i<size; i++) {
					nread = fread(&c, sizeof(char), 1, fp);
					nwritten = write(s, &c, sizeof(char));
					if (nread != 1 || nwritten != 1 || nread != nwritten) {
						fprintf(stderr, "--- ERROR writing the file.\n");
						return 0;
					}
				}
			
				// File correctly sent
				fprintf(stdout, "--- +OK message sent.\n");
			
				// Close file
				if (fclose(fp) != 0) {
					fprintf(stderr, "--- ERROR. fclose() failed.\n");
					return 1;
				}
			
			} else {
				// There is some error regarding the file
				fprintf(stderr, "--- ERROR. Something goes wrong with the file.\n");
				nwritten = write(s, MSG_ERR, strlen(MSG_ERR)*sizeof(char));
				if (nwritten != strlen(MSG_ERR)*sizeof(char)) {
					fprintf(stderr, "--- ERROR. write() of MSG_ERR failed.\n");
					return 0;
				} else {
					fprintf(stdout, "--- -ERR message sent.\n");
					return 1;
				}
			}
		} else if (nread >= strlen(MSG_QUIT) && strncmp(buf, MSG_QUIT, strlen(MSG_QUIT)) == 0) {
			/* If the command is QUIT */
			fprintf(stdout, "--- Client asked to terminate the connection.\n");
			return 0;
		} else {
			/* If there is an error */
			nwritten = write(s, MSG_ERR, strlen(MSG_ERR)*sizeof(char));
			if (nwritten != strlen(MSG_ERR)*sizeof(char)) {
				fprintf(stderr, "--- ERROR. write() of MSG_ERR failed.\n");
				return 0;
			} else {
				fprintf(stdout, "--- -ERR message sent.\n");
				return 1;
			}
		}
	
	}
	
	return 1;
}
