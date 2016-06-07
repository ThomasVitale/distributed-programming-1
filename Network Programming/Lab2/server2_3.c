/**********************************************************************
 ************************SERVER CONNECTION*****************************
 *********************************************************************/

#include "mylibrary.h"

#define BUFLEN 255
#define FILELEN 1023

#define MSG_ERR "-ERR\r\n"
#define MSG_GET "GET"
#define MSG_OK "+OK\r\n"
#define MSG_QUIT "QUIT"

int main(int argc, char** argv) {

	char buf[BUFLEN+1];								// transmitter and receiver buffers
	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int backlog = 2;									// pending requests queue length
	int finished, i, n, nread, size;
	socklen_t caddr_len;
	char c, filename[FILELEN];
	struct stat fileinfo;
	FILE* fp;
	uint32_t nBytes, nSeconds;
	time_t mTime;
	
	/* Check number of arguments */
	checkArg(argc, 2);

	/* Initialize the socket API (only for Windows) */
	SockStartup();

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind the socket to a local network address */
	fprintf(stdout, "Binding the socket\n");
	Bind(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "OK.\n");

	/* Listen to connection requests */
	fprintf(stdout, "Listening at socket %d with backlog = %d\n", s, backlog);
	Listen(s, backlog);
	fprintf(stdout, "OK.\n");
	
	while(1) {
		/* Accept connection requests */
		caddr_len = sizeof(struct sockaddr_in);
		fprintf(stdout, "Accepting connection requests\n");
		s_acceptor = Accept(s, (struct sockaddr*) &caddr, &caddr_len);
		fprintf(stdout, "OK.\n");
		
		/* Read a message */
		nread = 0;
		finished = 0;
		do {
			n = Read(s_acceptor, &c, sizeof(char));
			if (n==1) {
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
		
		/* Compute the command and reply to the client according to it */
		if (nread > strlen(MSG_GET) && strncmp(buf, MSG_GET, strlen(MSG_GET)) == 0) {
			/* If the command is GET */
			strcpy(filename, buf+4);
			
			if (stat(filename, &fileinfo) == 0) {
				// The file can be read correctly
				fp = Fopen(filename, "rb");
				size = fileinfo.st_size;
				mTime = fileinfo.st_mtime;
				
				// Write the response message
				Write(s_acceptor, MSG_OK, strlen(MSG_OK)*sizeof(char));
				
				// Write the number of bytes of the file
				nBytes = htonl(size);
				Write(s_acceptor, &nBytes, sizeof(size));
				
				// Write the timestamp of the last modification
				nSeconds = htonl(mTime);
				Write(s_acceptor, &nSeconds, sizeof(mTime));
				
				// Write the file itself
				for (i=0; i<size; i++) {
					Fread(&c, sizeof(char), 1, fp);
					Write(s_acceptor, &c, sizeof(char));
				}
				
				// Close file
				Fclose(fp);
				
			} else {
				// There is some error regarding the file
				fprintf(stderr, "ERROR. Something goes wrong with the file.\n");
			}
		} else if (nread >= strlen(MSG_QUIT) && strncmp(buf, MSG_QUIT, strlen(MSG_QUIT)) == 0) {
			/* If the command is QUIT */
			Close(s_acceptor);
		} else {
			/* If there is an error */
			Write(s_acceptor, MSG_ERR, strlen(MSG_ERR)*sizeof(char));
		}
	}
	
	return 0;
}
