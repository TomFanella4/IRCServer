
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "IRCServer.h"

int QueueLength = 5;

int IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void IRCServer::runServer(int port) {
	int masterSocket = open_server_socket(port);

	initialize();
	
	while ( 1 ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int main( int argc, char ** argv ) {
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void IRCServer::processRequest( int fd ) {
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");

	char * token;

	token = strtok(commandLine, " ");
	const char * command = token;
	
	token = strtok(NULL, " ");
	const char * user = token;
	
	token = strtok(NULL, " ");
	const char * password = token;
	
	token = strtok(NULL, " ");	
	const char * args = token;

	if (args == NULL)
		args = "";

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LIST-ROOMS")) {
		listRooms(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

void IRCServer::initialize() {
	// Open password file
	currentUser = 0;
	maxUsers = 20;
	users = (User*) malloc(sizeof(User) * maxUsers);	
	
	passwordFile = fopen(PASSWORD_FILE, "a+");

	// Initialize users in room
	char currentLine[50];

	while (fgets(currentLine, 50, passwordFile) != NULL) {
		char * token;
		token = strtok(currentLine, " ");
		users[currentUser].username = strdup(token);
		
		token = strtok(NULL, " ");	
		users[currentUser].password = strdup(token);
		
		currentUser++;
	}
	
	fclose(passwordFile);
	
	// Initalize message list
	currentRoom = 0;
	maxRooms = 10;
	rooms = (Room*) malloc(sizeof(Room) * maxRooms);
}

bool IRCServer::checkPassword(int fd, const char * user, const char * password) {
	char currentLine[50];
	passwordFile = fopen(PASSWORD_FILE, "a+");

	while (fgets(currentLine, 50, passwordFile) != NULL) {
		if (strstr(currentLine, user) != NULL && strstr(currentLine, password) != NULL)
			return true;
	}
	fclose(passwordFile);

	return false;
}

void IRCServer::addUser(int fd, const char * user, const char * password, const char * args) {
	// Check if user is in password file
	if (checkPassword(fd, user, password)) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	
	// User not found in file
	// Add user to file
	passwordFile = fopen(PASSWORD_FILE, "a+");

	// Checks if user list needs to be increased
	if (currentUser == maxUsers) {
		maxUsers*=2;
		users = (User*)realloc(users, sizeof(User) * maxUsers);
	}

	// Assigns username and password
	users[currentUser].username = strdup(user);
	users[currentUser].password = strdup(password);
	currentUser++;

	// Puts username and password in password file
	fprintf(passwordFile, "%s %s\n", user, password);
	fflush(passwordFile);
	fclose(passwordFile);

	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));

	return;
}

void IRCServer::createRoom(int fd, const char * user, const char * password, const char * args) {
	
	// Check username and password
	if (!checkPassword(fd, user, password)) {	
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	for (int i = 0; i < currentRoom; i++) {
		if (!strcmp(args, rooms[i].name)) {
			const char * msg =  "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	
	// Create room and assign defaults
	rooms[currentRoom].name = strdup(args);
	rooms[currentRoom].currentMessage = 0;
	rooms[currentRoom].currentUsinr = 0;
	rooms[currentRoom].maxUsinr = 20;
	rooms[currentRoom].usinr = (User*) malloc(sizeof(User) * rooms[currentRoom].maxUsinr);
	
	currentRoom++;

	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));

	return;
}

void IRCServer::listRooms(int fd, const char * user, const char * password, const char * args) {
	
	// Check username and password
	if (!checkPassword(fd, user, password)) {	
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	for (int i = 0; i < currentRoom; i++) {
		const char * namecpy;
		namecpy = strcat(strdup(rooms[i].name), "\n");
		write(fd, namecpy, strlen(namecpy));
	}

	write(fd, "\r\n", strlen("\r\n"));
	
	return;
}

void IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args) {
	
	// Check username and password
	if (!checkPassword(fd, user, password)) {	
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	int roomNum;

	for (int i = 0; i < currentRoom; i++) {
		if (!strcmp(args, rooms[i].name)) {
			roomNum = i;
			break;
		}
	}

	if (rooms[roomNum].currentUsinr == rooms[roomNum].maxUsinr) {
		rooms[roomNum].maxUsinr *= 2;
		rooms[roomNum].usinr = (User*) realloc(rooms[roomNum].usinr, 
								sizeof(rooms[roomNum].usinr) * rooms[roomNum].maxUsinr);
	}

	rooms[roomNum].usinr[rooms[roomNum].currentUsinr].username = strdup(user);
	rooms[roomNum].usinr[rooms[roomNum].currentUsinr].password = strdup(password);

	rooms[roomNum].currentUsinr++;
	
	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));

	return;
}

void IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args) {

	// Check username and password
	if (!checkPassword(fd, user, password)) {	
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	int roomNum;

	for (int i = 0; i < currentRoom; i++) {
		if (!strcmp(args, rooms[i].name)) {
			roomNum = i;
			break;
		}
	}

	for (int i = 0; i < rooms[roomNum].currentUsinr; i++) {
		if (strcmp(rooms[roomNum].usinr[i].username, strdup(user)) == 0 &&	
			strcmp(rooms[roomNum].usinr[i].password, strdup(password)) == 0) {
			
			for (int j = i; j < rooms[roomNum].currentUsinr - 1; j++) {
				rooms[roomNum].usinr[j].username = rooms[roomNum].usinr[j + 1].username;
				rooms[roomNum].usinr[j].password = rooms[roomNum].usinr[j + 1].password;
			}

			rooms[roomNum].usinr[rooms[roomNum].currentUsinr - 1].username = "";
			rooms[roomNum].usinr[rooms[roomNum].currentUsinr - 1].password = "";
			
			rooms[roomNum].currentUsinr--;

			const char * msg =  "OK\r\n";
			write(fd, msg, strlen(msg));

			return;
		}
	}

	const char * msg =  "DENIED\r\n";
	write(fd, msg, strlen(msg));

	return;	
}

void IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args) {

}

void IRCServer::getMessages(int fd, const char * user, const char * password, const char * args) {

}

void IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args) {
	
	// Check username and password
	if (!checkPassword(fd, user, password)) {	
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	int roomNum;

	for (int i = 0; i < currentRoom; i++) {
		if (!strcmp(args, rooms[i].name)) {
			roomNum = i;
			break;
		}
	}

	for (int i = 0; i < rooms[roomNum].currentUsinr; i++) {
		const char * namecpy;
		namecpy = strcat(strdup(rooms[roomNum].usinr[i].username), "\n");
		write(fd, namecpy, sizeof(namecpy));
	}
	
	write(fd, "\r\n", strlen("\r\n"));

	return;
}

void IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args) {
	
	const char * msg;

	if (!checkPassword(fd, user, password)) {	
		msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}

	//printf("CU: %d, First: %s\n", currentUser, users[0].username);

	for (int i = 0; i < currentUser; i++) {
		const char * namecpy;
		namecpy = strcat(strdup(users[i].username), "\n");
		write(fd, namecpy, strlen(namecpy));
	}
	
	write(fd, "\r\n", strlen("\r\n"));

	return;
}

