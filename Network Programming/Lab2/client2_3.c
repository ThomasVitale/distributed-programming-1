/**********************************************************************
 ************************CLIENT CONNECTION*****************************
 *********************************************************************/

#include "mylibrary.h"

#define BUFLEN 255
#define FILELEN 1023

#define MSG_OK "+OK"
#define MSG_QUIT "QUIT\r\n"

int main(int argc, char** argv) {

	char buf[BUFLEN], rbuf[BUFLEN];	// transmitter and receiver buffers
	SOCKET s; 											// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr; 			// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	uint32_t taddr_n; 							// server IP address by htonl()
	char filename[FILELEN], fileDwnld[FILELEN], c;
	FILE* fp;
	int nread, i;
	uint32_t fileBytes, timeStamp;

	/* Check number of arguments */
	checkArg(argc, 3);

	/* Initialize the socket API (only for Windows) */
	SockStartup();
	
	/* Set IP address and port number of Server */
	setIParg(argv[1], &sIPaddr);
	tport_n = setPortarg(argv[2], &tport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr = sIPaddr;

	/* Send connection request */
	fprintf(stdout, "Connecting to target address\n");
	Connect(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "OK.\n");
	
	/* Main */
	while(1) {
		/* Ask for a filename */
		fprintf(stdout, "Insert a filename, 'end' to finish.\n");
		fscanf(stdin, "%s", filename);
		
		if (strcmp(filename, "end") == 0) {
			// End the communication
			sprintf(buf, MSG_QUIT);
			Write(s, buf, strlen(buf)*sizeof(char));
			break;
		} else {
			// Request a filename 
			sprintf(buf, "GET %s\r\n", filename);
			Write(s, buf, strlen(buf));
			
			// Receive a file
			nread = 0;
			
			do {
				Read(s, &c, sizeof(char));
				rbuf[nread++] = c;
			} while((c != '\n') && (nread < BUFLEN-1));
			rbuf[nread] = '\0';
			
			while((nread > 0) && ((rbuf[nread-1] == '\r') || (rbuf[nread-1] == '\n'))) {
				rbuf[nread-1] = '\0';
				nread--;
			}
			
			fprintf(stdout, "Type of message: %s\n", rbuf);
			
			/* Compute the type of message received and behave according to it */
			if ((nread >= strlen(MSG_OK)) && (strncmp(rbuf, MSG_OK, strlen(MSG_OK)) == 0)) {
				// OK, right response received
				sprintf(fileDwnld, "Dwnld_%s", filename); // filename of received file
				
				// Read the file size
				Read(s, rbuf, 4*sizeof(char));
				fileBytes = ntohl(*(uint32_t*)rbuf);
				fprintf(stdout, "File size: %u\n", fileBytes);
				
				// Read the file timestamp
				Read(s, rbuf, 4*sizeof(char));
				timeStamp = ntohl(*(uint32_t*)rbuf);
				fprintf(stdout, "File timestamp: %u\n", timeStamp);
				
				// Received and write file
				fp = Fopen(fileDwnld, "wb");
				for (i=0; i<fileBytes; i++) {
					Read(s, &c, sizeof(char));
					Fwrite(&c, sizeof(char), 1, fp);
				}
				Fclose(fp);
				fprintf(stdout, "File received and written: %s\n", fileDwnld);
			} else {
				// ERROR
				fprintf(stderr, "ERROR. Something goes wrong with the communication protocol.\n");
			}
		}
	}
	
	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection\n");
	closesocket(s);
	fprintf(stdout, "OK.\n");
	
	/* Release resources (only for Windows) */
	SockCleanup();

	return 0;
}