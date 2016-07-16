#include "../mylibrary.h"
#include "../types.h"

char *prog_name;

int service(SOCKET);

int main(int argc, char** argv) {

	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int backlog = 2;									// pending requests queue length
	socklen_t caddr_len;							// client address length
	int retValue;											// service() returning status
	
	/* Check number of arguments */
	checkArg(argc, 2);
	
	prog_name = argv[0];
	
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
	
	/* Main loop */
	while(1) {
		/* Accept connection requests */
		caddr_len = sizeof(struct sockaddr_in);
		s_acceptor = Accept(s, (struct sockaddr*) &caddr, &caddr_len);
		fprintf(stdout, "- New connection from client ");
		showAddress(&caddr);
		
		service(s_acceptor);
		
		close(s_acceptor);
		//fprintf(stdout, "--- Connection closed by %s", (retValue == 0) ? "client\n" : "server\n");
	}
	
	return 0;
}

int service(SOCKET s) {
	
	XDR xdrs_in;											// Input XDR stream 
	XDR xdrs_out;											// Output XDR stream 
	FILE* stream_socket_r;						// FILE stream for reading from the socket
	FILE* stream_socket_w;						// FILE stream for writing to the socket
	call_msg reqMessage;
	response_msg resMessage;
	FILE* fp;
	struct stat fileinfo;
	uint32_t size, nBytes, nLeft, nNext;
	ssize_t nread, nwritten;
	int retValue = 1;
	char buf[MAXBUF];
	
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
		reqMessage.call_msg_u.filename = (char*)malloc(128*sizeof(char));
		if (!xdr_call_msg(&xdrs_in, &reqMessage)) {
			free(reqMessage.call_msg_u.filename);
			retValue = 0;
			break;
		}
		fprintf(stdout, "--- Received message.\n");
		
		/* Compute the command and reply to the client according to it */
		if (reqMessage.ctype == GET) {
			/* If the command is GET */
			
			if (stat(reqMessage.call_msg_u.filename, &fileinfo) == 0) {
				/* The file can be read correctly */
				
				// Get file size and timestamp
				size = (uint32_t) fileinfo.st_size;
				
				// Send OK message
				resMessage = OK;
				if (!xdr_response_msg(&xdrs_out, &resMessage)) {
					fprintf(stderr, "--- ERROR. Response xdr_message() failed.\n");
					free(reqMessage.call_msg_u.filename);
					retValue = 0;
					break;
				}
				fprintf(stdout, "--- +OK message sent.\n");
				fflush(stream_socket_w);
				
				// Write the number of bytes of the file
				nBytes = htonl(size);
				nwritten = write(s, &nBytes, sizeof(uint32_t));
				if (nwritten != sizeof(uint32_t)) {
					fprintf(stderr, "--- ERROR. write() of file size failed.\n");
					free(reqMessage.call_msg_u.filename);
					retValue = 0;
					break;
				}
				
				// Open the file
				fp = fopen(reqMessage.call_msg_u.filename, "rb");
				if (fp == NULL) {
					fprintf(stderr, "--- ERROR. fopen() failed.\n");
					free(reqMessage.call_msg_u.filename);
					retValue = 0;
					break;
				}
				
				// Write the file itself	
				nLeft = size;
				while(nLeft > 0) {
					
					if (nLeft < MAXBUF) {
						nNext = nLeft;
					} else {
						nNext = MAXBUF;
					}
					nread = fread(buf, sizeof(char), nNext, fp);
					nwritten = write(s, buf, nNext*sizeof(char));
					
					if (nread != nNext || nwritten != nNext) {
						fprintf(stderr, "--- ERROR writing the file.\n");
						free(reqMessage.call_msg_u.filename);
						retValue = 0;
						break;
					}
				
					nLeft -= nNext;
				}
				
				// Close file
				if (fclose(fp) != 0) {
					fprintf(stderr, "--- ERROR. fclose() failed.\n");
					free(reqMessage.call_msg_u.filename);
					retValue = 1;
					break;
				}
				
			} else {
				/* There is some error regarding the file */
				
				fprintf(stdout, "--- ERROR. file not found.\n");
				resMessage = ERR;
				if (!xdr_response_msg(&xdrs_out, &resMessage)) {
					free(reqMessage.call_msg_u.filename);
					retValue = 0;
					break;
				}
				fprintf(stdout, "--- -ERR message sent.\n");
				fflush(stream_socket_w);
			}

		} else if (reqMessage.ctype == QUIT) {
			/* If the command is QUIT */
			
			fprintf(stdout, "--- Client asked to terminate the connection.\n");
			free(reqMessage.call_msg_u.filename);
			retValue = 0;
			break;
		} else {
			/* If there is an error */
			
			fprintf(stdout, "--- ERROR. Wrong command.\n");
			resMessage = ERR;
			if (!xdr_response_msg(&xdrs_out, &resMessage)) {
				free(reqMessage.call_msg_u.filename);
				retValue = 0;
				break;
			}
			fprintf(stdout, "--- -ERR message sent.\n");
			fflush(stream_socket_w);
		}
		
		free(reqMessage.call_msg_u.filename);
		fflush(stream_socket_r);
		fflush(stream_socket_w);
	}
	
	xdr_destroy(&xdrs_in);
	fclose(stream_socket_r);
	xdr_destroy(&xdrs_out);
	fclose(stream_socket_w);
	
	return retValue;
}
