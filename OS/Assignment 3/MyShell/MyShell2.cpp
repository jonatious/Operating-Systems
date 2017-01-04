#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

string envVar[50];
string commands[50];
pid_t back_pids[50];
string back_commands[50];
static int back_count;

void parseEnv(char* inp)
{
    string input(inp);
    string delimiter = ":";
    size_t pos = 0;
    int i=0;

    while((pos=input.find(delimiter)) != string::npos){
        envVar[i++] = input.substr(0,pos);
        input.erase(0,pos+delimiter.length());
    }
}

void addBackgroundProcess(pid_t pid, string command)
{
	back_pids[back_count] = pid;
	back_commands[back_count] = command;
	back_count++;
}

void printBackgroundProcesses()
{
	//cout << "count = " << back_count << endl;
	if(back_count > 0)
	{
		cout << "Process ID\tCommand Line" << endl;
		for(int i = 0; i < back_count; i++)
			cout << back_pids[i] << "\t\t" << back_commands[i] << endl;
	}
	else
		cout << "No background processes are running" << endl;
}

bool isBackgroundProcess(pid_t pid)
{
    bool flag = false;
    for(int i = 0; i < back_count; i++)
        if(back_pids[i] == pid)
        {
            flag = true;
            break;
        }

    return flag;
}

void killBackgroundProcesses()
{
    for(int i = 0; i < back_count; i++)
        kill(back_pids[i], SIGKILL);
}

void removeBackgroundProcess(pid_t pid)
{
	int i = 0;
	while(pid != back_pids[i++]);

	for(i--;i < back_count - 1; i++)
	{
		back_pids[i] = back_pids[i+1];
		back_commands[i] = back_commands[i+1];
	}
	back_pids[i] = 0;
	back_commands[i] = "";
	back_count--;
}

int getLength(string array[])
{
    int i = 0;
    while(array[i] != "")
        i++;
    return i;
}

int index(string val)
{
	int length = getLength(commands);

	for(int i = 0; i < length; i++)
		if(commands[i] == val)
			return i;

	return -1;
}

void clearCommands()
{
	for(int i=0; i < 50; i++)
		commands[i] = "";
}

void printArray(char* array[])
{
	int i =0;
	while(array[i] != NULL)
	{
		cout << "value " << i+1 << ": " << array[i] << endl;
		i++;
	}
}

string findPath(string command)
{
	if(access((command).c_str(), F_OK) == 0)
		return command;
    for(int i = 0; i < getLength(envVar); i++)
    {
        if(access((envVar[i] + "/" + command).c_str(), F_OK) == 0)
            return envVar[i] + "/" + command;
    }
    return "";
}

string nextDelimiter(string input)
{
	string delmtr = " ";
	string nextDelmtr = "";
	size_t pos2 = 0, pos1 = 0;

	pos1 = input.find(delmtr);
	nextDelmtr = delmtr;

	delmtr = "<";
	pos2 = input.find(delmtr);
	if(pos1 > pos2)
	{
		pos1 = pos2;
		nextDelmtr = delmtr;
	}

	delmtr = ">";
	pos2 = input.find(delmtr);
	if(pos1 > pos2)
	{
		pos1 = pos2;
		nextDelmtr = delmtr;
	}

	delmtr = "|";
	pos2 = input.find(delmtr);
	if(pos1 > pos2)
	{
		pos1 = pos2;
		nextDelmtr = delmtr;
	}

	delmtr = "&";
	pos2 = input.find(delmtr);
	if(pos1 > pos2)
	{
		pos1 = pos2;
		nextDelmtr = delmtr;
	}

	delmtr = "\t";
	pos2 = input.find(delmtr);
	if(pos1 > pos2)
	{
		pos1 = pos2;
		nextDelmtr = delmtr;
	}

	return nextDelmtr;
}

string getCommand(string input_line){
    string delimiter;
    string com;
    size_t pos = 0;

    //command
    input_line.erase(input_line.begin(), std::find_if(input_line.begin(), input_line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    delimiter = nextDelimiter(input_line);
    pos=input_line.find(delimiter);
    com = input_line.substr(0,pos);

    return com;
}

void parseInput(string input_line)
{
    string delimiter;
    size_t pos = 0;
    int i = 0;

    //command
    input_line.erase(input_line.begin(), std::find_if(input_line.begin(), input_line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    delimiter = nextDelimiter(input_line);

    while((pos = input_line.find(delimiter)) != string::npos){

    	if(delimiter != " " && delimiter != "" && pos == 0)
	    {
	    	commands[i++] = delimiter;
	    	input_line.erase(0,pos+delimiter.length());
	    	input_line.erase(input_line.begin(), std::find_if(input_line.begin(), input_line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	    	delimiter = nextDelimiter(input_line);
	    	continue;
	    }

	    commands[i++] = input_line.substr(0,pos);
	    input_line.erase(0,pos+delimiter.length());
	    input_line.erase(input_line.begin(), std::find_if(input_line.begin(), input_line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

	    //special characters
	    if(delimiter != " " && delimiter != "" && delimiter != "\t")
	    {
	    	commands[i++] = delimiter;
	    }
	    delimiter = nextDelimiter(input_line);
	}
	if(input_line != "")
		commands[i] = input_line;
}
  
int main()
{
    string input_line;
    parseEnv(getenv("PATH"));
    back_count = 0;

    while(1)
    {
        cout << "MyShell: ";
        getline(cin, input_line);
        
        string command = getCommand(input_line);
        clearCommands();
        parseInput(input_line);

        //Built in commands
        if(strcmp(command.c_str(), "exit") == 0)
        {
            killBackgroundProcesses();
        	break;
        }
        if(strcmp(command.c_str(), "cd") == 0)
        {
        	chdir(commands[1].c_str());
        	continue;
        }
        if(strcmp(command.c_str(), "") == 0)
        {
        	continue;
        }
        if(strcmp(command.c_str(), "processes") == 0)
        {
        	//remove completed background processes
        	pid_t pid = fork();
        	if (pid == 0)
        	{
        		_exit(0);
        	}

        	pid_t apid;
        	while ((apid = wait(0)) != pid) {
        			if(isBackgroundProcess(apid)) {
        				removeBackgroundProcess(apid);
     				}
			}

        	printBackgroundProcesses();
        	continue;
        }

        const char* path = findPath(command).c_str();

        //generate argv
        int length = getLength(commands);
        bool isInputRedirect = false, isOutputRedirect = false, isInteractive = true, isPiped = false;
        string file, pipe_commands;
        char* argv[length+1];

        int i;

        for(i = 0; i < length; i++)
        {
        	const char* temp = commands[i].c_str();
        	//cout << "arg " << i+1 << ": " << temp << endl;

        	if(commands[i] == ">" || commands[i] == "<" || commands[i] == "&" || commands[i] == "|")
        	{;
        		int ind;
        		if(index("&") != -1)
        			isInteractive = false;
        		if(index("|") != -1)
        		{
        			ind = index("|");
        			isPiped = true;

        			for(int j = ind+1; j < length; j++)
        				pipe_commands = pipe_commands + " " + commands[j];
        		}
        		if(commands[i] == "<")
        		{
        			isInputRedirect = true;
        			file = commands[i+1];
        		}
        		else if(commands[i] == ">")
        		{
        			isOutputRedirect = true;
        			file = commands[i+1];
        		}
        		break;
        	}

        	argv[i] = strdup(temp);
        }
        argv[i] = NULL;
        //cout << "count = " << i << endl;

        pid_t pid = fork();
        path = findPath(command).c_str();

        if(pid == 0)//child process
        {
        	//input redirection
        	if(isInputRedirect)
        	{
        		int fd = open(file.c_str(), O_RDONLY);
        		close(0);
        		dup(fd);
        		close(fd);
        	}

        	//output redirection
        	if(isOutputRedirect)
        	{
        		int fd = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        		close(1);
        		dup(fd);
        		close(fd);
        	}

        	if(isPiped)
        	{
        		int pd[2];
        		pipe(pd);

        		pid_t pipe_child = fork();
        		
        		if(pipe_child == 0)
        		{
        			close(0);
        			dup(pd[0]);
        			close(pd[0]);
	        		close(pd[1]);

	        		clearCommands();
	        		parseInput(pipe_commands);

	        		const char* pipe_path = findPath(commands[0]).c_str();
	        		int len = getLength(commands);

	        		bool pipe_opRedirect = false;
	        		string pipe_file;
	        		char* pipe_argv[len+1];
	        		int j;

	        		for(j=0;j < len; j++)
	        		{
	        			if(commands[j] == "&")
	        				break;
		        		if(commands[j] == ">")
		        		{
		        			pipe_opRedirect = true;
		        			pipe_file = commands[j+1];
		        			break;
		        		}
		        		const char* temp = commands[j].c_str();
		        		pipe_argv[j] = strdup(temp);
	        		}
	        		pipe_argv[j] = NULL;

	        		//pipe output redirection
		        	if(pipe_opRedirect)
		        	{
		        		//cout << "op pipe redirect" << endl;
		        		int fd = open(pipe_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		        		close(1);
		        		dup(fd);
		        		close(fd);
		        	}

	        		const char* temp = strdup(pipe_path);

	        		//cout << "pipe path= " << temp << " arg1 = " << pipe_argv[0] << endl;
	        		execv(temp, pipe_argv);
        			perror("execv");
        			_exit(1);
        		}
        		else
        		{
	        		close(1);
	        		dup(pd[1]);
	        		close(pd[1]);
	        		close(pd[0]);
	        		//cout << "pipe parent path= " << path << " arg1 = " << argv[0] << endl;
        			const char* temp = strdup(path);
		        	execv(temp, argv);
		        	perror("execv");

		        	_exit(1);// failure
        		}
        	}

        	//cout << "reg path= " << path << " arg1 = " << argv[0] << endl;

        	const char* temp = strdup(path);
        	execv(temp, argv);
        	perror("execv");
        	_exit(1);// failure
        }
        else
        {
        	if(isInteractive)
        	{
        		pid_t apid;
        		while ((apid = wait(0)) != pid) {
        			if(isBackgroundProcess(apid)) {
        				removeBackgroundProcess(apid);
     				}
				}
        	}
        	else
        	{
        		addBackgroundProcess(pid, input_line);
        	}
        }
        
    }

    return 0;
}