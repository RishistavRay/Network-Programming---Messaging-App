/*
** Server.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "helper.h"
#include "server.h"


#define BACKLOG 10	 // how many pending connections queue will hold

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* client_connect(void *new_client) {

  //create new message packet
  int numbytes;  
  MESSAGE_PACKET new_message;
  CLIENT* client = (CLIENT*)new_client;

  while (1) {

    if ((numbytes = recv(client->sockfd, &new_message, sizeof(MESSAGE_PACKET), 0)) == -1) {
    perror("recv");
    exit(1);
    }
    else if (numbytes == 0) {
      //logout(new_message.source);
      close(client->sockfd);
      return 0;
      }

    int cmd = new_message.type;

    if (cmd == LOGIN) {
      login(new_message.source,new_message.data, client->sockfd);
    }

   else if (cmd == EXIT) {
      logout(new_message.source);
    }

    else if (cmd == NEW_SESS) {
      new_session(new_message.data);
      join_session(new_message.source, new_message.data,client->sockfd,1);
    }
    else if (cmd == JOIN) {
      join_session(new_message.source, new_message.data,client->sockfd,0);
    }
    else if (cmd == LEAVE_SESS) {
      leave_session(new_message.source);
    }
    else if (cmd == QUERY) {
      list(client->sockfd);
    }
    else if (cmd == MESSAGE) {
      message(new_message.source, new_message.data);
    }
    else if (cmd == PRIVATE) {
      private_message(client->sockfd,new_message.source,new_message.data);
    }
    else if (cmd == BOOT) {
      boot(new_message.source,new_message.data, client->sockfd);
    }
    else if (cmd == PROMOTE) {
      promote(new_message.source,new_message.data, client->sockfd);
    }
  }

  return 0;

}

int login (unsigned char* clientid,unsigned char* password,int sockfd ) {


  MESSAGE_PACKET message_login;
  message_login.type = LO_NAK;
  char* data = "username or password incorrect";
  int numbytes;

    //check if password correct or user exist
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    if ((strcmp(password_array[i].clientid ,(char*) clientid)==0) && (strcmp(password_array[i].password,(char*) password)==0)) {
        message_login.type = LO_ACK;
      }
  }


  //check if already loggedin
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    if ((strcmp(client_array[i].clientid ,(char*) clientid)==0)) {
        data = "user already loggedin";
        message_login.type = LO_NAK;
      }
  }


//If succesful then 
if (message_login.type == LO_ACK) {
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    if (*client_array[i].clientid == '\0') {
        strcpy(client_array[i].clientid,(char*)clientid);
        client_array[i].sockfd = sockfd;
        break; 
      }
  }
  printf("Client %s has logged in \n",clientid);
}

  // send acknoledgement
  strcpy((char*) message_login.data,data);
  if ((numbytes = send((sockfd), &message_login, sizeof(message_login), 0)) == -1) {
  perror("client: sendto");
  exit(1);}  
  return 0;
}

int new_session (unsigned char* sessionid) {

  for(int i =0; i < SESSION_ARRAY_LENGTH; i++) {
    if (session_array[i] == 0) {
      session_array[i] = atoi((char*)sessionid) ;
      printf("Created new session %s \n",sessionid);
      break;
    }
  }
  return 0;
}

int join_session (unsigned char* clientid ,unsigned char* sessionid,int sockfd, int admin) {

    MESSAGE_PACKET message_session;
    message_session.type = JN_ACK;
    int numbytes;

    for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    if (strcmp(client_array[i].clientid ,(char*) clientid)==0) {
        //send negative
        if (client_array[i].sessionid != 0) {
          message_session.type = JN_NAK;
          break;
        }
        client_array[i].sessionid = atoi((char*)sessionid);
        if(admin) {client_array[i].admin = 1;} 
        else {client_array[i].admin = 0;}
        printf("%s joined session %s \n",clientid, sessionid);
        break; 
      }
    }


  if ((numbytes = send((sockfd), &message_session, sizeof(message_session), 0)) == -1) {
  perror("client: sendto");
  exit(1);} 

  return 0;
}

int leave_session (unsigned char* clientid) {

    for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    if (strcmp(client_array[i].clientid ,(char*) clientid)==0) {
        client_array[i].sessionid = 0;
        break; 
      }
    }
  printf("%s left session \n" , (char*) clientid);
  return 0;
}

int list (int sockfd) {

  MESSAGE_PACKET message_list;
  message_list.type = QU_ACK;
  int numbytes;

  if ((numbytes = send((sockfd), &message_list, sizeof(message_list), 0)) == -1) {
  perror("client: sendto");
  exit(1);}

  if ((numbytes = send((sockfd), &client_array, sizeof(client_array), 0)) == -1) {
  perror("client: sendto");
  exit(1);}

  printf("sent list \n");
  return 0;

}


int logout (unsigned char* clientid) {

    for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    if (strcmp(client_array[i].clientid ,(char*) clientid)==0) {
        memset(&client_array[i], 0, sizeof(CLIENT));

        break; 
      }
    }
  printf("loggedout %s \n",clientid);
  return 0;
}

int message(unsigned char* clientid, unsigned char* line) {

  MESSAGE_PACKET message_text;
  message_text.type = MESSAGE;
  message_text.size = strlen((char*)line);
  strncpy((char*) message_text.source,(char*)clientid,MAX_NAME);
  strncpy((char*) message_text.data,(char*) line,MAX_DATA);
  int numbytes;



  //find session id
  int sessionid;
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
  if (strcmp(client_array[i].clientid ,(char*) clientid)==0) {
        sessionid = client_array[i].sessionid;
        break; 
      }
  }
  // broadcast message to others with same session id
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
  if (client_array[i].sessionid == sessionid && strcmp(client_array[i].clientid ,(char*) clientid)!=0) {

    if ((numbytes = send((client_array[i].sockfd), &message_text, sizeof(message_text), 0)) == -1) {
    perror("client: sendto");
    exit(1);}

    }
  }

  printf("sent messages to session %d \n",sessionid);

  return 0;

}

int private_message(int sockfd, unsigned char* receipientid, unsigned char* line) {

  MESSAGE_PACKET message_private;
  message_private.type = P_ACK;
  strncpy((char*) message_private.data,(char*) line,MAX_DATA);
  int numbytes;


  //find clientid
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
  if (client_array[i].sockfd == sockfd) {
        strncpy((char*) message_private.source,(char*)client_array[i].clientid,MAX_NAME);
        break; 
      }
  }
  
  // broadcast message to others with same session id
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
  if (strcmp(client_array[i].clientid ,(char*) receipientid)==0) {
    if ((numbytes = send((client_array[i].sockfd), &message_private, sizeof(message_private), 0)) == -1) {
    perror("client: sendto");
    exit(1);}
    }

  }

  printf("private message sent to %s \n",receipientid);
  return 0;

}

int boot (unsigned char* clientid,unsigned char* name,int sockfd ) {


  MESSAGE_PACKET message_boot;
  message_boot.type = BOOT_NACK;
  int numbytes;

  //check if already loggedin
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    //check if admin
    if ((strcmp(client_array[i].clientid ,(char*) clientid)==0)) {
        if (client_array[i].admin == 1 && client_array[i].sessionid != 0)


        //check if another user
          for(int j =0; j < CLIENT_ARRAY_LENGTH; j++) {
            if ((strcmp(client_array[j].clientid ,(char*) name)==0) && client_array[j].sessionid == client_array[i].sessionid ) {
              client_array[j].sessionid = 0;
              message_boot.type = BOOT_ACK;



              //send warning
              // send acknoledgement

                MESSAGE_PACKET message_bootw;
                message_bootw.type = BOOT_WARN;
                int numbytesw;

              printf("boot request by %s succesful \n",clientid);

              if ((numbytesw = send((client_array[j].sockfd), &message_bootw, sizeof(message_bootw), 0)) == -1) {
              perror("client: sendto");
              exit(1);}  
              }
          }}
  }

//If succesful then 
if (message_boot.type == BOOT_NACK) {

  printf("boot request by %s failed \n",clientid);
}

  // send acknoledgement
  if ((numbytes = send((sockfd), &message_boot, sizeof(message_boot), 0)) == -1) {
  perror("client: sendto");
  exit(1);}  
  return 0;
}

int promote (unsigned char* clientid,unsigned char* name,int sockfd ) {


  MESSAGE_PACKET message_promote;
  message_promote.type = PROMOTE_NACK;
  int numbytes;

  //check if already loggedin
  for(int i =0; i < CLIENT_ARRAY_LENGTH; i++) {
    //check if admin
    if ((strcmp(client_array[i].clientid ,(char*) clientid)==0)) {
        if (client_array[i].admin == 1 && client_array[i].sessionid != 0)

        //check if another user
          for(int j =0; j < CLIENT_ARRAY_LENGTH; j++) {
            if ((strcmp(client_array[j].clientid ,(char*) name)==0) && client_array[j].sessionid == client_array[i].sessionid ) {
              client_array[j].admin = 1;
              client_array[i].admin = 0;
              message_promote.type = PROMOTE_ACK;



              //send warning
              // send acknoledgement

                MESSAGE_PACKET message_promotew;
                message_promotew.type = PROMOTE_WARN;
                int numbytesw;

              printf("promote request by %s succesful \n",clientid);

              if ((numbytesw = send((client_array[j].sockfd), &message_promotew, sizeof(message_promotew), 0)) == -1) {
              perror("client: sendto");
              exit(1);}  
              }
          }}
  }

//If succesful then 
if (message_promote.type == PROMOTE_NACK) {

  printf("promote request by %s failed \n",clientid);
}

  // send acknoledgement
  if ((numbytes = send((sockfd), &message_promote, sizeof(message_promote), 0)) == -1) {
  perror("client: sendto");
  exit(1);}  
  return 0;
}





int main(int argc, char *argv[])
{

  //initialize client array to null

  memset(client_array, 0, sizeof(client_array));

  //initialize password array

  strcpy(password_array[0].clientid,"roger");
  strcpy(password_array[0].password,"rabbit");
  strcpy(password_array[1].clientid,"donald");
  strcpy(password_array[1].password,"duck");
  strcpy(password_array[2].clientid,"mickey");
  strcpy(password_array[2].password,"mouse");

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
  (void) argc;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
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

    //set timeout
       
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}


		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

  //fcntl(sockfd, F_SETFL, O_NONBLOCK);

	printf("server: waiting for connections...\n");



	while(1) {  // main accept() loop


		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
      //pthread_yield();
			continue;
		}

    CLIENT *new_client = malloc(sizeof(CLIENT));

		sin_size = sizeof their_addr;

    new_client->sockfd = new_fd;

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

     pthread_t new_thread;
     pthread_create(&new_thread, NULL, client_connect, new_client);

    /*
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, "Hello, world!", 13, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	} */

  }

    //remove room for client array in memory

	return 0;
}
