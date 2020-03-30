#include <iostream>
#include <algorithm> 
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <iomanip>.


#define green 32
#define blue 34
#define red 31
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77    
using namespace std;

//trimming whitespace of ends  of a string
string trimWhiteSpace(string s)
{
    //trimming white space beginning of string
	auto start = s.begin();
	while (start != s.end() && isspace(*start)) {
		start++;
	}
    //trimming white space at the end of string
	auto end = s.end();
	do {
		end--;
	} while (distance(start, end) > 0 && isspace(*end));
     
	return string(start, end + 1);
}

//trim white spaces of a vector of strings (arguments)
void trimArguments(vector<string> &arguments){
    for(int i = 0; i < arguments.size(); ++i){
        arguments[i] = trimWhiteSpace(arguments[i]);
    }
}
//checks if a substrig exist in a given string
bool isInString(string source, string delimeter){
    size_t found = source.find(delimeter);
    return found!=string::npos;
}

//printing all Background process ids running
void printPIDs(vector<pid_t> pids){
    for(pid_t pd: pids){
        cout << "Process id: " << pd << endl;
    }
}

//splitting arguments by a delimeter, handling arguments in single and double quotes
vector<string> split(string commandLine, string delimeter = " ")
{
    vector<string> arguments;
    bool hasQuotedArgumnets = false;   
    int count = 0;
    while (commandLine.size())
    {
        for (int i = 0; i < commandLine.size(); i++)
        {
            ///Check if index is in quotes
            if ((commandLine.at(i) == '"' || commandLine.at(i) == '\'') && !hasQuotedArgumnets)
            {
                hasQuotedArgumnets = true;
            }
            ///checking for second pair of quotes not found yet 
            else if ((commandLine.at(i) == '"' || commandLine.at(i) == '\'') && hasQuotedArgumnets)
            {
                hasQuotedArgumnets = false;
            }
            if (hasQuotedArgumnets && commandLine.at(i) == delimeter[0])
            {
                /*If separator is found within quotes, add 128 to make it unreadable*/
                commandLine.at(i) += 128;
            }
        }
        ///////////////////////////////////
        //pushing back all arguments
        size_t found = commandLine.find(delimeter);
        if (found == string::npos)
        {
            string lastpart = trimWhiteSpace(commandLine);
            if (lastpart.size() > 0)
            {
                arguments.push_back(lastpart);
            }
            break;
        }
        string segment = trimWhiteSpace(commandLine.substr(0, found));
                                                                    
        commandLine = commandLine.substr(found + 1);
        if (segment.size() != 0)
            arguments.push_back(segment);
    }
    //restoring unreadable charactesrs 
    for (int i = 0; i < arguments.size(); i++)
    {
        size_t found = arguments[i].find_first_of(delimeter[0] + 128);
        while (found != string::npos)
        {
            //restore token
            arguments[i][found] = delimeter[0];
            //find another instance of unreadable character
            found = arguments[i].find_first_of(delimeter[0] + 128, found + 1);
        }
    }
    //removing single and double quotes from arguments 
    for(int i = 0; i <  arguments.size();++i){
        string argument = arguments[i];
        if((argument[0] == '\'' && argument[argument.size()-1] == '\'')  ||  (argument[0] == '\"' && argument[argument.size()-1] == '\"')){
            arguments[i].pop_back();
            arguments[i].erase(arguments[i].begin());
        }
    }
    return arguments;
}

//converting a vector of arguments to correct format for execvp
char** executableArguments(string command){
    vector<string> arguments = split(command," ");
    // for(string arg: arguments){
    //     cerr << arg << endl;
    //     cerr << arg.size() << endl;
    // }
    char** args = new char * [arguments.size()+1];
    for(int i = 0; i < arguments.size(); ++i){
        //args[i] = (char *)arguments[i].c_str();
        args[i] = new char[arguments[i].size() + 1];
        strcpy(args[i], arguments[i].c_str());
    }
    args[arguments.size()] = NULL;
    //cout << "Howdy" << endl;
    return args;
}

//outputting into a certain color 
string colorFormat(string str, int color){
    return "\033[1;" + to_string(color) + "m" +  str + "\033[0m";
}

//outputs format for shell depending on its directory location
void shellDirFormat(){
        char Buffer[1024];
        ///////////////////////////////////////////////////////////////////////////
        // outputting linux format of user data and current directory
        getcwd(Buffer,sizeof(Buffer));
        string dir(Buffer);
        gethostname(Buffer,sizeof(Buffer));
        string hostName(Buffer);
        getlogin_r(Buffer,sizeof(Buffer));
        string user(Buffer);
        cout << colorFormat(user+"@" +hostName,red) << ":" << colorFormat(dir,blue) + "$";
        ///////////////////////////////////////////////////////////////////////////
}
//executes line or command
void executLine(string Line){
    char** arguments = executableArguments(Line);
    execvp(arguments[0], arguments);
}
//waits for all processes in a vector
void waitForBGProcesses(vector<pid_t> &bgProcesses){
    for(int i = 0; i < bgProcesses.size(); ++i){
        pid_t bgProcess = bgProcesses[i];
        //waiting for background processes not blocking program and removing pid if is done 
        if(waitpid(bgProcess,0,WNOHANG) > 0){
            //process terminated, erasing process from vector
            bgProcesses.erase(bgProcesses.begin() + i);
            //reseting index to previous position to no skip any process
        }
    }
}
int executeShell(){
    vector<pid_t> backGroundProcesses;
    cout  << "----------------------------------Edgar's Shell---------------------------------" << endl;
    while(true){
        bool isBGProcess = false;
        //outputting right linux format
        shellDirFormat();
        
        //getting passed line to the shell
        string line = "";
        getline(cin, line);
        
        //waiting for all current background processes and removing the ones that are done
        waitForBGProcesses(backGroundProcesses);
        
        //saving standard input and standard output
        int SVStdin = dup(0);
        int SVStdout = dup(1);
        
        if(line == "jobs"){
            printPIDs(backGroundProcesses);
        }
        //exiting from shell
        else if(line == "exit"){
            cout << "----------------------------Exiting from Edgar's Shell----------------------------" << endl;
            return -1;
        }
        //changing directory
        else if(line.substr(0,2) == "cd"){
            string dirName = split(line," ")[1];
            chdir(dirName.c_str()); 
        }
        else{
            //checking for a background process
            if(line.back() == '&'){
                //removing character to have a valid command
                line.pop_back();
                isBGProcess = true;
            }
            vector<string> pipedCommands =  split(line,"|");

            for(int i = 0;  i < pipedCommands.size(); ++i){
                string pipedCommand = pipedCommands[i];
                int fd[2];
                pipe(fd);
                int pid = fork();
                if(!pid){
                    //redirecting intput
                    if (i < pipedCommands.size() - 1){
                        //cout << "Here" << endl;

                        dup2 (fd[1], 1); 
                        //redirecting input to a file
                    }
                    //redirecting output to a file
                    if(i == 0  && isInString(pipedCommand,"<") && isInString(pipedCommand,">")){
                        cout << "Command = " << pipedCommand<< endl;
                        vector<string> outCommand =  split(pipedCommand,">");
                        // for(string cmd: outCommand){
                        //     cout << cmd << endl;
                        // }
                        vector<string> inCommand = split(outCommand[0],"<");
                        // for(string cmd: inCommand){
                        //     cout << cmd << endl;
                        // }
                        string inFileName = inCommand[1];
                        string outFileName = outCommand[1];
                        cerr << "Here " << endl;
                        pipedCommand = inCommand[0];
                        int inFD = open(inFileName.c_str(),O_CREAT | O_RDONLY);
                        dup2(inFD,0);
                        int outFD = open(outFileName.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        //redirecting output
                        dup2(outFD,1);
                    }else{
                        if(i  == (pipedCommands.size()-1) && isInString(pipedCommand,">") && split(pipedCommand,">").size() > 1){
                            //cerr << "output redirection statement" << endl;
                            //redirecting output to a file if is last command and contains out2put redirection symbol
                            vector<string> outRedirectARGS  = split(pipedCommand,">");
                            string fileName= outRedirectARGS[1];
                            pipedCommand = outRedirectARGS[0];
                            //cerr << "fileName" << endl;
                            int fileOutputDescriptor = open(fileName.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                            dup2(fileOutputDescriptor,1);
                        }
                        if(i  == 0 && isInString(pipedCommand,"<") && split(pipedCommand,"<").size() > 1){
                            vector<string> inRedirectARGS  = split(pipedCommand,"<");
                            string inFileName= inRedirectARGS[1];
                            pipedCommand = inRedirectARGS[0];
                            int fdIn = open(inFileName.c_str(),O_CREAT | O_RDONLY);
                            dup2(fdIn,0);
                        }
                    }
                    close(fd[1]);
                    executLine(pipedCommand);
                }
                //parent process
                else{
                    //adding new background process
                    //cout  << "Adding bg process: " << isBGProcess << endl;
                    if(isBGProcess){
                        cerr << "Adding Background process: " << pid << endl;
                        backGroundProcesses.push_back(pid);
                    }else{
                        //very last process
                        if(i == pipedCommands.size()-1){
                            waitpid(pid,0,0);
                        }
                    }
                    //redirecting standard input
                    dup2(fd[0], 0);
                    //Closing standard output
                    close(fd[1]);
                }
            }
        }//
        //putting bak standard input and output
        dup2(SVStdin,0);
        dup2(SVStdout,1);
    }//end of shell iteration
}


int main(){
    executeShell();
    //formatArguments("awk '/init/{print $1}'");
    //executableArguments("awk '/init/{print $1}'");
    //newSplit("Hello 'create'");
}