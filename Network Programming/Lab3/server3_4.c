#include "../mylibrary.h"
#include "types.h"

#define MSG_ERR "-ERR\r\n"
#define MSG_GET "GET"
#define MSG_OK "+OK\r\n"
#define MSG_QUIT "QUIT"

int n_children;
pid_t *pids;

int service(SOCKET);
int serviceXDR(SOCKET);
void sigintHandler(int sig);

int main(int argc, char** argv) {

	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int backlog = 2;									// pending requests queue length
	socklen_t caddr_len;							// client address length
	int retValue;											// service() returning status
	int i, xdrFlag=0;
	
	/* Check number of arguments */
	if (argc == 3) {
		/* Set port number  */
		lport_n = setPortarg(argv[1], &lport_h);
		
		/* The number of children */
		n_children = atoi(argv[2]);
		if (n_children > 10) {
			fprintf(stderr, "- ERROR. Children must be at most 10.\n");
			return -1;
		}
	} else if (argc == 4) {
	
		/* Use XDR */
		xdrFlag = 1;
	
		/* Set port number  */
		lport_n = setPortarg(argv[2], &lport_h);
		
		/* The number of children */
		n_children = atoi(argv[3]);
		if (n_children > 10) {
			fprintf(stderr, "- ERROR. Children must be at most 10.\n");
			return -1;
		}
	} else {
		fprintf(stderr, "ERROR. Wrong arguments. Syntax: %s [-x] port n_children.\n", argv[0]);
		return 1;
	}
	
	/* Alloc memory for pids */
	pids = (pid_t*)malloc(n_children*sizeof(pid_t));
	if (pids == NULL) {
		fprintf(stderr, "- ERROR allocating pids.\n");
		return -1;
	}
	
	/* Instantiate a signal handler for SIGINT */
	Signal(SIGINT, sigintHandler);

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
	
	for (i=0; i<n_children; i++) {
		
		if ((pids[i] = fork()) < 0) {
			fprintf(stderr, "- ERROR. fork() failed.\n");
		} else if ((pids[i]) > 0) {
			// The parent
		} else {
			// The child
			while(1) {
				/* Accept connection requests */
				br();
				caddr_len = sizeof(struct sockaddr_in);
				s_acceptor = Accept(s, (struct sockaddr*) &caddr, &caddr_len);
				fprintf(stdout, "- New connection from client ");
				showAddress(&caddr);
				
				if (xdrFlag) {
					retValue = serviceXDR(s_acceptor);
				} else {
					retValue = service(s_acceptor);
				}
				
				closesocket(s_acceptor);
				fprintf(stdout, "--- Connection closed by %s", (retValue == 0) ? "client\n" : "server\n");
				br();
			}
		}
		
	}
	
	while(1) {
		pause();
	}
	
	return 0;
}

/* Terminate all children when cntrl-c */
void sigintHandler(int sig) {
	int i;
	
	fprintf(stdout, "\n--- SIGINT received, terminating...\n");
	
	for (i=0; i<n_children; i++) {
		kill(pids[i], SIGTERM);
	}
	
	while (wait(NULL) > 0);
	
	exit(0);
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

int serviceXDR(SOCKET s) {
	
	XDR xdrs_in;											// Input XDR stream 
	XDR xdrs_out;											// Output XDR stream 
	FILE* stream_socket_r;						// FILE stream for reading from the socket
	FILE* stream_socket_w;						// FILE stream for writing to the socket
	message reqMessage, resMessage;
	FILE* fp;
	struct stat fileinfo;
	uint32_t mTime, size;
	char *fileContent;
	ssize_t nread;
	int retValue = 1;
	
	/* Open FILE reading stream and bind it to the corresponding XDR stream */
	stream_socket_r = fdopen(s, "r");
	if (stream_socket_r == NULL) {
		fprintf(stderr, "---ERROR. fdopen() failed.\n");
		return 1;
	}
	xdrstdio_create(&xdrs_in, stream_socket_r, XDR_DECODE);
	
	/* Open FILE writing stream and bind it to the corresponding XDR stream */
	stream_socket_w = fdopen(s, "w");
	if (stream_socket_w == NULL) {
		fprintf(stderr, "---ERROR. fdopen() failed.\n");
		xdr_destroy(&xdrs_in);
		fclose(stream_socket_r);
		return 1;
	}
	xdrstdio_create(&xdrs_out, stream_socket_w, XDR_ENCODE);

	/* Main loop */
	while(1) {
	
		/* Receive a message */
		reqMessage.message_u.filename = (char*) malloc(FILELEN*sizeof(char));
		if (!xdr_message(&xdrs_in, &reqMessage)) {
			free(reqMessage.message_u.filename);
			retValue = 0;
			break;
		}
		fprintf(stdout, "--- Received message.\n");
		
		/* Compute the command and reply to the client according to it */
		if (reqMessage.tag == GET) {
			/* If the command is GET */
			
			if (stat(reqMessage.message_u.filename, &fileinfo) == 0) {
				/* The file can be read correctly */
				
				// Get file size and timestamp
				size = (uint32_t) fileinfo.st_size;
				mTime = (uint32_t) fileinfo.st_mtime;
				
				// Open the file
				fp = fopen(reqMessage.message_u.filename, "rb");
				if (fp == NULL) {
					fprintf(stderr, "--- ERROR. fopen() failed.\n");
					free(reqMessage.message_u.filename);
					retValue = 0;
					break;
				}
				
				// Write the file itself
				fileContent = (char*)malloc(size*sizeof(char));
				nread = fread(fileContent, sizeof(char), size, fp);
				if (nread != size) {
						fprintf(stderr, "--- ERROR reading the file.\n");
						free(fileContent);
						free(reqMessage.message_u.filename);
						retValue = 0;
						break;
				}
				
				// Send OK message
				resMessage.tag = OK;
				resMessage.message_u.fdata.last_mod_time = mTime;
				resMessage.message_u.fdata.contents.contents_len = size;
				resMessage.message_u.fdata.contents.contents_val = fileContent;
				if (!xdr_message(&xdrs_out, &resMessage)) {
					fprintf(stderr, "--- ERROR. Response xdr_message() failed.\n");
					free(fileContent);
					free(reqMessage.message_u.filename);
					retValue = 0;
					break;
				}
				fprintf(stdout, "--- +OK message sent.\n");
				fflush(stream_socket_w);
				free(fileContent);
			
				// Close file
				if (fclose(fp) != 0) {
					fprintf(stderr, "--- ERROR. fclose() failed.\n");
					free(reqMessage.message_u.filename);
					retValue = 1;
					break;
				}
				
			} else {
				/* There is some error regarding the file */
				
				fprintf(stdout, "--- ERROR. file not found.\n");
				resMessage.tag = ERR;
				if (!xdr_message(&xdrs_out, &resMessage)) {
					free(reqMessage.message_u.filename);
					retValue = 0;
					break;
				}
				fprintf(stdout, "--- -ERR message sent.\n");
				fflush(stream_socket_w);
			}

		} else if (reqMessage.tag == QUIT) {
			/* If the command is QUIT */
			
			fprintf(stdout, "--- Client asked to terminate the connection.\n");
			free(reqMessage.message_u.filename);
			retValue = 0;
			break;
		} else {
			/* If there is an error */
			
			fprintf(stdout, "--- ERROR. Wrong command.\n");
			resMessage.tag = ERR;
			if (!xdr_message(&xdrs_out, &resMessage)) {
				free(reqMessage.message_u.filename);
				retValue = 0;
				break;
			}
			fprintf(stdout, "--- -ERR message sent.\n");
			fflush(stream_socket_w);
		}
		
		free(reqMessage.message_u.filename);
		fflush(stream_socket_r);
		fflush(stream_socket_w);
	}
	
	xdr_destroy(&xdrs_in);
	fclose(stream_socket_r);
	xdr_destroy(&xdrs_out);
	fclose(stream_socket_w);
	
	return retValue;
}
