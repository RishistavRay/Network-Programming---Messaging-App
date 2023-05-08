
/*
** listener.c -- a datagram sockets "server" 
** code snippits taken from Beej's guide
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>



#define MAXBUFLEN 100
#define PACKETLENGTH 1000

typedef struct {
      unsigned int total_frag;
      unsigned int frag_no;
      unsigned int size;
      char* filename;
      char filedata[PACKETLENGTH+500];
 } PACKET ;



 // get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
//Function to recieve pack
void recievePacket(int sockfd,struct sockaddr_storage *a_addr, socklen_t addr_len, FILE *file,PACKET *packet1, int* packetnumber, bool* done) {


    struct sockaddr_storage their_addr = (*a_addr);
	int numbytes;


    char buf[1200];
    memset(buf, 0, sizeof buf);
    char s[INET6_ADDRSTRLEN];
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);

    //printf("test30");
	printf("server: waiting to recvfrom...\n");
	addr_len = sizeof their_addr;

	if ((numbytes = recvfrom(sockfd, buf, 1100, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}





	//new 

 //break up packet and convert to int
  PACKET packet = (*packet1);
	packet1->total_frag = atoi(strtok(buf, ":"));
    packet1->frag_no	 = atoi(strtok(NULL, ":"));
	packet1->size = atoi(strtok(NULL, ":"));
	packet1->filename = strtok(NULL, ":");
	int length;
	length = strlen(packet1->filename)+1;
	//printf("test2");
	//char* pointer = strtok(NULL, ":");
	//printf("test3");
	//printf("%d",sizeof(char)*packet1->size);

	





	memcpy(packet1->filedata,packet1->filename + length , sizeof(char)*packet1->size);


	  double r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
	//printf("%f",r/RAND_MAX);
  // Packet drop code



		if (r/RAND_MAX > .5) {

		}
		else {
			printf("server: packet dropped \n"); //packet drop simulation
			return;
                }

		//guard if recieved duplicate
		//printf("%d \n",*packetnumber);
		if (*packetnumber != packet1->frag_no) {return;}

	



	printf("server: got packet %d from %s\n",packet1->frag_no,
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	//printf("server: packet is %d bytes long\n", numbytes);
	//buf[numbytes] = '\0';


	

	//printf("server: packet contains \"%d\"\n", packet.frag_no);  

	// New file
        
        char foldername[100] = "recFolder"; // set the folder name
        
        mkdir(foldername, 0777); // create the folder with read, write, and execute permissions for all users
        
        char filepath[100] = "/homes/r/rayrishi/NetBeansProjects/361lab1";
        
        sprintf(filepath, "%s/%s", foldername, packet1->filename); // construct the full file path
        
        file = fopen(filepath ,"a");
        
        fwrite(packet1->filedata,sizeof(char),packet1->size,file);
        
        fclose(file);
        
  file = fopen(packet1->filename ,"a");
	fwrite(packet1->filedata,sizeof(char),packet1->size,file);
	fclose(file);

	//Send Ack
	int j;
	char buf2[20];
	j = sprintf(buf2, "%d", packet1->frag_no);

	numbytes = sendto(sockfd, &buf2, sizeof buf2, 0, (struct sockaddr *)&their_addr, addr_len);
	printf("server: sent ACK packet %d \n", packet1->frag_no);

	*packetnumber =  *packetnumber + 1;

	if (packet1->frag_no == packet1->total_frag) {(*done) = 1;


}

	//printf("%d \n",*done);



	return;

}


int main(int argc, char *argv[])
{
  srand(time(NULL));   // Initialization, should only be called once.
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("server: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("server: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("server: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("server: packet contains \"%s\"\n", buf);
        
        
        //new code
        
        //check if command is ftp. if true, then respond yes, else respond no
        if(strcmp(buf,"ftp") == 0) {
            if ((numbytes = sendto(sockfd, "yes", strlen("yes"), 0,
                     (struct sockaddr *)&their_addr, addr_len)) == -1) {
            perror("server: sendto");
            exit(1);}
        }

        else {
           if ((numbytes = sendto(sockfd, "no", strlen("no"), 0,
                 (struct sockaddr *)&their_addr, addr_len)) == -1) {
            perror("server: sendto");
            exit(1);}
        }
        
        printf("server: sent %d bytes to %s\n", numbytes,  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));

 // LAB 2 Code

	FILE *file;
	int packetnumber = 1;
	bool done = 0;
	

	while(1) {

   PACKET packet;
	 packet.frag_no = 0;
	 packet.total_frag = -1;
   recievePacket(sockfd,&their_addr,addr_len,file,&packet,&packetnumber,&done);

	 if (done == 1) {break;}


	}
	


	

	close(sockfd);
	printf("file transfer complete \n");

	return 0;
}