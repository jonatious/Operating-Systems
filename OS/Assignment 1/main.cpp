// Obligatory includes 
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

#include <string.h>
#include <stdlib.h>

using namespace std;

// Constants
#define ALPHA 0
#define BRAVO 5

//---------------------- Added Code ---------------------//
#define MAXHOSTNAME 255
//Global Constants
int server;
int portNum = 1234;
char buffer[1024];

//Global functions

int string_to_int(char str[])
{
	int val = 0;
	for(int i = 0; i< strlen(str); i++)
		val = val*10 + (str[i] - '0');
	
	return val;
}

void create_buffer(char msg[], int resource)
{
	char temp[3];
	
	if(resource < 10)
	{
		temp[0] = resource + '0';
		temp[1] = '\0';
	}
	else
	{
		temp[0] = resource/10 + '0';
		temp[1] = resource%10 + '0';
		temp[2] = '\0';
	}
	
	memset(buffer, 0, 1024);

	strcat(buffer,msg);
	strcat(buffer,"$");
	strcat(buffer,temp);
}

void create_socket()
{
	sleep(3);
    struct sockaddr_in server_addr;

	char hostname[MAXHOSTNAME+1];
	struct hostent *hp;
	
	gethostname(hostname, MAXHOSTNAME); 	
	if ((hp= gethostbyname(hostname)) == NULL) { 
		cout << "Connection Refused.." << endl;
		exit(1); 
	}
	
	memset(&server_addr,0,sizeof(server_addr));
	memcpy((char *)&server_addr.sin_addr,hp->h_addr,hp->h_length);
	server_addr.sin_family= hp->h_addrtype;
	server_addr.sin_port= htons((u_short)portNum);
	
	server = socket(AF_INET, SOCK_STREAM, 0);

	if (server < 0) 
	{
	    cout << "\nError establishing socket..." << endl;
	    exit(1);
	}
	
	if (connect(server,(struct sockaddr *)&server_addr, sizeof server_addr) == 0)
	{
	    //cout << "=> Connection to the server port number: " << portNum << endl;
	}
	else
	{
		cout << "Connection failed.." << endl;
		exit(1);
	}
	
}

// Templates to be filled
int create_lock(int resource)
{
	char msg[] = "CREATE_LOCK";
	
	create_socket();
	create_buffer(msg, resource);
	
	send(server, buffer, 100, 0);
	
	memset(buffer, 0, 1024);
	recv(server, buffer, 1024, 0);
	
	if(*buffer == 'Y')
	{
		cout << "Successfully created lock" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else
	{
		cout << "Lock creation failed" << endl;
		close(server);
		cout << endl;
		return -1;
	}
}

int read_lock(int resource)
{
	char msg[] = "READ_LOCK";
	
	create_socket();
	create_buffer(msg, resource);
	
	send(server, buffer, 100, 0);
	
	memset(buffer, 0, 1024);
	recv(server, buffer, 1024, 0);
	
	if(*buffer == 'Y')
	{
		cout << "Successfully obtained READ lock" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else if(*buffer == 'W')
	{
		do
		{
			cout << "Waiting in Read lock queue" << endl;
			memset(buffer, 0, 1024);
			recv(server, buffer, 1024, 0);
		}while(*buffer != 'Y');
		
		cout << "Read lock obtained after wait in queue" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else
	{
		cout << "Read Lock failed" << endl;
		close(server);
		cout << endl;
		return -1;
	}
}

int write_lock(int resource)
{
	char msg[] = "WRITE_LOCK";
	
	create_socket();
	create_buffer(msg, resource);
	
	send(server, buffer, 100, 0);
	
	memset(buffer, 0, 1024);
	recv(server, buffer, 1024, 0);
	
	if(*buffer == 'Y')
	{
		cout << "Successfully obtained WRITE lock" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else if(*buffer == 'W')
	{
		do
		{
			cout << "Waiting in Write lock queue" << endl;
			memset(buffer, 0, 1024);
			recv(server, buffer, 1024, 0);
		}while(*buffer != 'Y');
		
		cout << "Write lock obtained after wait in queue" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else
	{
		cout << "Write Lock failed" << endl;
		close(server);
		cout << endl;
		return -1;
	}
}

int read_unlock(int resource)
{
	char msg[] = "READ_UNLOCK";
	
	create_socket();
	create_buffer(msg, resource);
	
	send(server, buffer, 100, 0);
	
	memset(buffer, 0, 1024);
	recv(server, buffer, 1024, 0);
	
	if(*buffer == 'Y')
	{
		cout << "READ lock released" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else
	{
		cout << "Read Lock release failed" << endl;
		close(server);
		cout << endl;
		return -1;
	}
}

int write_unlock(int resource)
{
	char msg[] = "WRITE_UNLOCK";
	
	create_socket();
	create_buffer(msg, resource);
	
	send(server, buffer, 100, 0);
	
	memset(buffer, 0, 1024);
	recv(server, buffer, 1024, 0);
	
	if(*buffer == 'Y')
	{
		cout << "WRITE lock released" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else
	{
		cout << "Write Lock release failed" << endl;
		close(server);
		cout << endl;
		return -1;
	}
}

int delete_lock(int resource)
{
	char msg[] = "DELETE_LOCK";
	
	create_socket();
	create_buffer(msg, resource);
	
	send(server, buffer, 100, 0);
	
	memset(buffer, 0, 1024);
	recv(server, buffer, 1024, 0);
	
	if(*buffer == 'Y')
	{
		cout << "Successfully deleted lock" << endl;
		close(server);
		cout << endl;
		return 0;
	}
	else
	{
		cout << "Lock deletion failed" << endl;
		close(server);
		cout << endl;
		return -1;
	}
}

int kill_server()
{
	char msg[] = "KILL_SERVER";
	
	create_socket();
	memset(buffer, 0, 1024);
	strcpy(buffer, msg);
	
	send(server, buffer, 100, 0);
	
	cout << "Kill Server Requested" << endl;
	close(server);
	cout << endl;
	return 0;
}

//---------------------- Added code till here ----------------------//

int main (int argc, char* argv[]) {
	int pid; // child's pid

//---------------------- Added code from here ----------------------//
	if(argc == 1)
	{
		cout << "Please enter port number as command line argument (Ex: ./client 1234). Exiting..." << endl;
		return 0;
	}
	
	portNum = string_to_int(argv[1]);
//---------------------- Added code till here ----------------------//

        // Before the fork
        cout << "Create lock ALPHA\n";
	create_lock(ALPHA);
        cout << "Create lock BRAVO\n";
	create_lock(BRAVO);
        cout << "Parent requests write permission on lock BRAVO\n";
	write_lock(BRAVO);
        cout << "Write permission on lock BRAVO was granted\n";
        cout << "Parent requests read permission on lock ALPHA\n";
	read_lock(ALPHA);
        cout << "Read permission on lock ALPHA was granted\n";
	sleep(1);
	
	// Fork a child
	if ((pid = fork()) == 0) {
		// Child process
	        cout << "Child requests read permission on lock ALPHA\n";
		read_lock(ALPHA); // This permission should be granted
        	cout << "Read permission on lock ALPHA was granted\n";
		sleep(1);
	        cout << "Child releases read permission on lock ALPHA\n";
		read_unlock(ALPHA);
		sleep(1);
        	cout << "Child requests write permission on lock BRAVO\n";
		write_lock(BRAVO); // Should wait until parent relases its lock
        	cout << "Write permission on lock BRAVO was granted\n";
		sleep(1);
	        cout << "Child releases write permission on lock BRAVO\n";
		write_unlock(BRAVO);
		cout << "Child terminates\n";
                _exit(0);
	} // Child

	// Back to parent
        cout << "Parent releases read permission on lock ALPHA\n";
	read_unlock(ALPHA);
        cout << "Parent requests write permission on lock ALPHA\n";
	write_lock(ALPHA); // Should wait until child removes its read lock
        cout << "Write permission on lock ALPHA was granted\n";
	sleep(1);
        cout << "Parent releasesweite permission on lock ALPHA\n";
	write_unlock(ALPHA);
	sleep(1);
        cout << "Parent releases write permission on lock BRAVO\n";
	write_unlock(BRAVO);

	// Child and parent join
        while (pid != wait(0));  // Busy wait
	delete_lock(ALPHA);
        delete_lock(BRAVO);
        // We assume that failed operations return a non-zero value
        if (write_lock(ALPHA) != 0) {
		cout << "Tried to access a deleted lock\n";
	}
	kill_server();
} // main
	
