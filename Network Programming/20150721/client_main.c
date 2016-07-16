#include "../mylibrary.h"
#include "../types.h"

char *prog_name;

int main(int argc, char** argv) {

	char rbuf[MAXBUF];							// transmitter and receiver buffers
	SOCKET s; 											// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr; 			// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	char *filename;
	FILE* fp;
	uint32_t fileBytesN, fileBytes, nNext, nLeft;
	ssize_t nread, nwritten;
	
	XDR xdrs_in;											// Input XDR stream 
	XDR xdrs_out;											// Output XDR stream 
	FILE* stream_socket_r;						// FILE stream for reading from the socket
	FILE* stream_socket_w;						// FILE stream for writing to the socket
	call_msg reqMessage;
	response_msg resMessage;
	
	/* Check number of arguments */
	checkArg(argc, 4);
	
	prog_name = argv[0];
	
	/* Set IP address */
	setIParg(argv[1], &sIPaddr);
	
	/* Set port number  */
	tport_n = setPortarg(argv[2], &tport_h);
	
	/* Save the file to request */
	filename = argv[3];
	
	/* Create the socket */
	fprintf(stdout, "Creating the socket...\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "- OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr = sIPaddr;

	/* Send connection request */
	fprintf(stdout, "Connecting to target address...\n");
	Connect(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "- OK. Connected to ");
	showAddress(&saddr);
	
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
	
	while(1) {
		/* Send a file request */
		reqMessage.ctype = GET;
		reqMessage.call_msg_u.filename = filename;
		if (!xdr_call_msg(&xdrs_out, &reqMessage)) {
			fprintf(stdout, "- ERROR sending GET message.\n");
			break;
		}
		fflush(stream_socket_w);
	
		/* Receive a message */
		if (!xdr_response_msg(&xdrs_in, &resMessage)) {
			fprintf(stdout, "- ERROR. Response xdr_response_msg() failed.\n");
			break;
		}
		fprintf(stdout, "- Received response.\n");
		
		
		if (resMessage == OK) {
				
			fprintf(stdout, "- File received: %s\n", filename);
			
			// Read the file size
			nread = read(s, (void*)&fileBytesN, sizeof(uint32_t));

			fileBytes = ntohl(fileBytesN);
			fprintf(stdout, "- File size: %u\n", fileBytesN);

			// Received and write file
			fp = Fopen(filename, "wb");
			nLeft = fileBytes;
			while(nLeft > 0) {
				
				if (nLeft < MAXBUF) {
					nNext = nLeft;
				} else {
					nNext = MAXBUF;
				}

				nread = Read(s, rbuf, nNext*sizeof(char));
				nwritten = Fwrite(rbuf, sizeof(char), nNext, fp);
				
				if (nread != nNext || nwritten != nNext) {
					fprintf(stdout, "--- ERROR saving file.\n");
					break;
				}
				
				nLeft -= nNext;

			}
			
			Fclose(fp);
			fprintf(stdout, "--- File written: %s\n", filename);
		
		} else if (resMessage == ERR) {
			fprintf(stderr, "- Received ERR message.\n");
			break;
		} else {
			fprintf(stderr, "- ERROR. Something goes wrong with the communication protocol.\n");
			break;
		}
		
		/* End the communication */
		reqMessage.ctype = QUIT;
		reqMessage.call_msg_u.filename = NULL;
		if (!xdr_call_msg(&xdrs_out, &reqMessage)) {
			fprintf(stdout, "- ERROR sending QUIT message.\n");
			break;
		}
		fprintf(stdout, "- QUIT message sent.\n");
		
		fflush(stream_socket_r);
		fflush(stream_socket_w);
		break;
	}
	
	xdr_destroy(&xdrs_in);
	fclose(stream_socket_r);
	xdr_destroy(&xdrs_out);
	fclose(stream_socket_w);
	
	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection...\n");
	closesocket(s);
	fprintf(stdout, "- OK. Closed.\n");
	
	return 0;
}
