/*
	Author: Jonatious Joseph Jawahar
	PSID: 1416103
	Method: Priority given to write locks before read locks
*/

#include <cstdlib>
#include <errno.h> 
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>

//global constants
#define MAXHOSTNAME 255
#define SUCCESS "Y"
#define FAILURE "N"
#define WAIT "W"

//global variables
int portNum;
int bufsize = 1024;
char msg[1024];
int resource;

int locks[32];
int lock_count;

int read_locks[32];
int read_locks_count[32];
int write_locks[32];

int read_lock_queue[32][50];
int read_queue_len[32];
int write_lock_queue[32][50];
int write_queue_len[32];

using namespace std;

void parse_msg(char message[])
{
	memset(msg, 0, bufsize);
	int i;
	for (i=0; i< strlen(message); i++)
	{
		if(message[i] != '$')
			msg[i] = message[i];
		else
			break;
	}
	
	if(strlen(message) == i+2)
		resource = message[i+1] - '0';
	else
		resource = (message[i+1] - '0')*10 + (message[i+2] - '0');
}

int find_lock(int resource)
{	
	for(int i=0; i< lock_count; i++)
		if(locks[i] == resource)
			return i;
	
	return -1;
}

void handle_operation(int id)
{
	int loc = find_lock(resource);
	
	if(strcmp(msg, "CREATE_LOCK") != 0 && loc == -1)
	{
		send(id, FAILURE, 1, 0);
		cout << "Lock doesn't exist." << endl;
		return;
	}
	
	if(strcmp(msg, "CREATE_LOCK") == 0)
	{
		if(loc > -1)
		{
			send(id, FAILURE, 1, 0);
			cout << "Lock already exists." << endl;
			return;
		}
		
		locks[lock_count] = resource;
		lock_count++;
		send(id, SUCCESS, 1, 0);
		
		cout << "Lock created." << endl;
		return;
	}
	
	if(strcmp(msg, "DELETE_LOCK") == 0)
	{
		locks[loc] = 0;
		
		for(int i=loc; i< lock_count - 1; i++)
		{
			locks[i] = locks[i+1];
		}
		
		locks[lock_count-1] = 0;
		lock_count--;
		send(id, SUCCESS, 1, 0);
		
		cout << "Lock deleted." << endl;
		return;
	}
	
	if(strcmp(msg, "READ_LOCK") == 0)
	{
		if(write_locks[loc] == 0)
		{
			send(id, SUCCESS, 1, 0);
			read_locks[loc] = 1;
			read_locks_count[loc]++;
			cout << "Read lock granted." << endl; 
		}
		else
		{
			send(id, WAIT, 1, 0);
			read_lock_queue[loc][read_queue_len[loc]] = id;
			read_queue_len[loc]++;
			cout << "Resource is busy. Added to read queue." << endl;
		}
		
		return;
	}
	
	if(strcmp(msg, "WRITE_LOCK") == 0)
	{
		if(write_locks[loc] == 1 || read_locks[loc] == 1)
		{
			send(id, WAIT, 1, 0);
			
			write_lock_queue[loc][write_queue_len[loc]] = id;
			
			write_queue_len[loc]++;
			cout << "Resource is busy. Added to write queue." << endl;
		}
		else
		{
			send(id, SUCCESS, 1, 0);
			write_locks[loc] = 1;
			cout << "Write lock granted." << endl;
		}
		
		return;
	}
	
	if(strcmp(msg, "READ_UNLOCK") == 0)
	{
		
		if(read_locks[loc] == 0)
		{
			send(id, FAILURE, 1, 0);
			cout << "Read lock not locked" << endl;
			return;
		}
		
		if(write_locks[loc] == 1)
		{
			send(id, FAILURE, 1, 0);
			cout << "Write lock is locked" << endl;
			return;
		}
		
		send(id, SUCCESS, 1, 0);
		read_locks_count[loc]--;
		
		if(read_locks_count[loc] == 0)
			read_locks[loc] = 0;
		
		cout << "Read Unlock Successful." << endl;
		
		if(write_queue_len[loc] > 0 && read_locks_count[loc] == 0)
		{
			send(write_lock_queue[loc][0], SUCCESS, 1, 0);
				
			for(int i = 0; i < write_queue_len[loc] - 1; i++)
				write_lock_queue[loc][i] = write_lock_queue[loc][i+1];
			
			write_queue_len[loc]--;
			write_locks[loc] = 1;
			
			cout << "Write locks granted to next process in write queue" << endl;
		}
		
		return;
	}
	
	if(strcmp(msg, "WRITE_UNLOCK") == 0)
	{
		if(write_locks[loc] == 0)
		{
			send(id, FAILURE, 1, 0);
			cout << "Write lock not locked" << endl;
			return;
		}
		
		if(read_locks[loc] == 1)
		{
			send(id, FAILURE, 1, 0);
			cout << "Read lock is locked" << endl;
			return;
		}
		
		send(id, SUCCESS, 1, 0);
		write_locks[loc] = 0;
		
		cout << "Write Unlock Successful." << endl;
		
		if(write_queue_len[loc] > 0)
		{
			send(write_lock_queue[loc][0], SUCCESS, 1, 0);
		
			for(int i = 0; i < write_queue_len[loc] - 1; i++)
				write_lock_queue[loc][i] = write_lock_queue[loc][i+1];
			
			write_queue_len[loc]--;
			write_locks[loc] = 1;
			
			cout << "Write locks granted to next process in write queue" << endl;
		}
		else if(read_queue_len[loc] > 0)
		{
			for(int i = 0; i < read_queue_len[loc]; i++)
			{
				read_locks_count[loc]++;
				send(read_lock_queue[loc][i], SUCCESS, 1, 0);
				read_lock_queue[loc][i] = 0;
			}
			read_queue_len[loc] = 0;
			read_locks[loc] = 1;
			
			cout << "Read locks granted to all processes in read queue" << endl;
		}
		
		return;
	}
}

int string_to_int(char str[])
{
	int val = 0;
	for(int i = 0; i< strlen(str); i++)
		val = val*10 + (str[i] - '0');
	
	return val;
}

int main(int argc, char* argv[])
{
	lock_count = 0;
    int client, server;
    char buffer[bufsize];
    struct sockaddr_in server_addr;
	char myname[MAXHOSTNAME+1];
	
    struct hostent *hp;
	
    memset(&server_addr, 0, sizeof(struct sockaddr_in)); 
    gethostname(myname, MAXHOSTNAME);
	
    hp = gethostbyname(myname);
	
    if (hp == NULL) 
		return(-1);

	if(argc == 1)
	{
		cout << "Please enter port number as command line argument (Ex: ./server 1234). Exiting..." << endl;
		return 0;
	}
	
	portNum = string_to_int(argv[1]);
		
    server_addr.sin_family= hp->h_addrtype; 
    server_addr.sin_port= htons(portNum); 
	
    socklen_t size;

    client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0) 
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }

    cout << "\n=> Socket server has been created on port = " << portNum << endl;

    if ((bind(client, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
    {
        cout << "=> Error binding connection, the socket has already been established..." << endl;
        return -1;
    }

    size = sizeof server_addr;

    listen(client, 100);

	while(true)
	{
		cout << "Waiting for Connection..." << endl;
		server = accept(client,(struct sockaddr *)&server_addr,&size);
		
		cout << "connection Accepted" << endl;
		if (server < 0) 
		{
			cout << "=> Error on accepting..." << endl;
			continue;
		}
		
		recv(server, buffer, bufsize, 0);
		
		if(strcmp(buffer, "KILL_SERVER") == 0)
		{
			cout << "Kill server message received" << endl;
			break;
		}
		parse_msg(buffer);
		cout << "Operation = " << msg << ", Resource = " << resource << endl;
		
		handle_operation(server);
		
		cout << endl;
		
	}
	
	cout << "Terminating server..." << endl;

    close(client);
    return 0;
}
