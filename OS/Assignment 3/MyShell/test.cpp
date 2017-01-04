#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string envVar[50];

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

int getLength(string array[])
{
    int i = 0;
    while(array[i] != "")
        i++;
    return i;
}

string findPath(string command)
{
    for(int i = 0; i < getLength(envVar); i++)
    {
        if(access((envVar[i] + "/" + command).c_str(), F_OK) == 0)
            return envVar[i] + "/" + command;
    }
    return "";
}

int main()
{
    /*parseEnv(getenv("PATH"));

    char *argv[] = { "ls", NULL };

    execv(findPath("ls").c_str(), argv);
    cout << "Starting..." << endl;
    sleep(20);*/
    sleep(10);
    cout << "ending" << endl;

    return 0;
}