#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>   
#include <unistd.h> 
#include <dirent.h> 
#include <fstream>
#include <signal.h>
#include "../headers/helpdiseaseaggregator.h"

using namespace std;

volatile sig_atomic_t died_pid=-1;
volatile sig_atomic_t sigintorquit = false;

int main(int argc,char* argv[])
{
    int i=0,j=0,k=0,q=0,countries=0,numWorkers,buffersize,pid,num_of_forks,PCfd,CPfd,last_bytes=0,chunk_times=0,cases=0;
    string input_dir,myText,all_sum_stats="",message ="",f="";
    Worker_Info** info_array;
    char space[1] = {' '},last_char[1] = {'\0'},new_line[1] = {'\n'},num_of_dirs[20];
    struct dirent *entry;
    DIR *dir;

    if(argc!=7)
    {
        cout << "Incorrect Input!Please try the above format:" <<  endl << "./diseaseAggregator –w numWorkers -b bufferSize -i input_dir " << endl;
        return -1;
    }
    for(i=1;i<=5;i+=2)
    {            
            if(strcmp(argv[i],"-w")==0) numWorkers=atoi(argv[i+1]);
            else if(strcmp(argv[i],"-i")==0) input_dir = argv[i+1];
            else if(strcmp(argv[i],"-b")==0) buffersize = atoi(argv[i+1]);
            else return -1;
    }

    //Check for positive values of parameters.
    if(buffersize<=0)
    {
        cout << "BufferSize must be a positive number!" << endl;
        return -1;
    }
    if(numWorkers<=0)
    {
        cout << "numWorkers must be a positive number!" << endl;
        return -1;
    }

    //Convert input_dir from string to char*.
    char* path = new char[input_dir.length()+1];
    strcpy(path,input_dir.c_str());

    //Calculate num of countries.
    dir = opendir(path);
    if(dir==0) 
    {
        cout << "The parameter input_dir that you gave doesn't exist.Try again!" << endl;
        return -1;
    }
    while ((entry = readdir(dir)) != NULL)  
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0)   
            countries++;
    closedir(dir);

    //Store countries from input_dir to countries_array.
    char** countries_array = new char*[countries];
    dir = opendir(path);
    while ((entry = readdir(dir)) != NULL) 
    {
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0)
        {
            countries_array[k] = new char[strlen(entry->d_name)+1];
            strcpy(countries_array[k],entry->d_name);
            k++;
        }
    }
    closedir(dir);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Start to do some real work for father//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    //Case: Some Workers will manipulate more than one dirs.
    if(countries >= numWorkers)
    {
        //Calculate how many forks are going to happen and then allocate memory for info_array
        num_of_forks = numWorkers;
        info_array = new Worker_Info*[num_of_forks];

        //Calculate some values so as to split dirs uniformly.
        int dirs_per_worker = countries/numWorkers;
        int mod = countries%numWorkers;

        //Case: Some Workers are going to have more dirs than others.
        if(mod!=0)
        {
            for(i=0;i<numWorkers;i++)
            {
                //Allocate memory for each Worker_Info struct.
                info_array[i] = new Worker_Info;

                //First mod Workers will have more dirs_per_worker+mod directories.
                if(i<mod)
                {
                    //Allocate memory for directories of Worker.
                    info_array[i]->dirs = new char* [dirs_per_worker+1];
                    info_array[i]->num_of_dirs = dirs_per_worker+1;
                    for(int q=0;q<(dirs_per_worker+1);q++)
                    {
                        info_array[i]->dirs[q] = new char[strlen(countries_array[j])+1];
                        strcpy(info_array[i]->dirs[q],countries_array[j]);
                        j++;
                    }
                }
                //The rest Workers will have dirs_per_worker directories.
                else
                {
                    //Allocate memory for directories of Worker.
                    info_array[i]->dirs = new char* [dirs_per_worker];
                    info_array[i]->num_of_dirs = dirs_per_worker;
                    for(int q=0;q<(dirs_per_worker);q++)
                    {
                        info_array[i]->dirs[q] = new char[strlen(countries_array[j])+1];
                        strcpy(info_array[i]->dirs[q],countries_array[j]);
                        j++;
                    }
                }
            }
        }
        //Case: All Workers will have the same number of dirs.
        else
        {
            for(i=0;i<numWorkers;i++)
            {
                //Allocate memory for each Worker_Info struct.
                info_array[i] = new Worker_Info;

                //Allocate memory for directories of Worker.
                info_array[i]->dirs = new char* [dirs_per_worker];
                info_array[i]->num_of_dirs = dirs_per_worker;
                for(int q=0;q<(dirs_per_worker);q++)
                {
                    info_array[i]->dirs[q] = new char[strlen(countries_array[j])+1];
                    strcpy(info_array[i]->dirs[q],countries_array[j]);
                    j++;
                }
            }
        }  
    }
    else
    {    
        num_of_forks = countries;
        info_array = new Worker_Info*[num_of_forks];
        
        for(i=0;i<num_of_forks;i++)
        {
            //Allocate memory for each Active Worker_Info struct.
            info_array[i] = new Worker_Info;

            //Allocate memory for directory of Active Worker.
            info_array[i]->dirs = new char*[1];
            info_array[i]->num_of_dirs = 1;
            info_array[i]->dirs[0] = new char[strlen(countries_array[j])+1];
            strcpy(info_array[i]->dirs[0],countries_array[j]);
            j++;  
        }
    }
    
    //Store names of pair of pipes of each Worker into struct Worker_Info.
    char PCfifo[20];
    char CPfifo[20];
    for(i=0;i<(num_of_forks);i++)
    {
        sprintf(PCfifo,"pipes/PCfifo%d",i);
        sprintf(CPfifo,"pipes/CPfifo%d",i);
        strcpy(info_array[i]->PCfifo,PCfifo);
        strcpy(info_array[i]->CPfifo,CPfifo);
        if (mkfifo(PCfifo, 0666) < 0) perror("Cannot create named pipe!");
        if (mkfifo(CPfifo, 0666) < 0) perror("Cannot create named pipe!");
    }
    
    //Create num_of_forks Workers with fork and each Worker run worker.cpp.
    for(i=0;i<numWorkers; i++)
    { 
        pid = fork();

        //Fork failed
        if(pid==-1)
        { 
            perror("Failed to fork");
            return 1;
        }
        //Workers can continue.
        else if(pid==0)
        {
            //Active Workers. 
            if(i<num_of_forks)
            {    
                char p[40];
                char buff[10];
                sprintf(buff, "%d", buffersize);
                sprintf(p,"./executable/worker");
                char* arr[] = { p, info_array[i]->PCfifo, buff, NULL};
                execv("./executable/worker", arr);
            }
            //Inactive Workers.
            //Case: numWorkers>countries.
            else
            {
                char p[40];
                sprintf(p,"./executable/inactive_worker");
                char* arr[] = {p, NULL};
                execv("./executable/inactive_worker", arr);
            } 
        }
        //Father stores pids of his active children.
        else    
            if(i<num_of_forks)  info_array[i]->pid=pid;
    }

    cout << "Father " << getpid() << endl;

    //Father passes some significant information to each Worker through named pipes.
    for(i=0;i<(num_of_forks);i++)
    {
        //Father opens named pipe so as to write.
        if((PCfd = open(info_array[i]->PCfifo, O_WRONLY ))<0)   perror("Father cannot open PCfifo!");
        
        //Write number of dirs of Worker!
        sprintf(num_of_dirs,"%d",info_array[i]->num_of_dirs);
        write(PCfd,num_of_dirs,strlen(num_of_dirs));
        write(PCfd,new_line,1);

        //Father informs Worker which dirs is going manipulate.
        for(q=0;q<info_array[i]->num_of_dirs;q++)
        {
            //Calculate how many splits are we going to do for each country and last_bytes of country.
            chunk_times = strlen(info_array[i]->dirs[q])/buffersize;
            last_bytes = strlen(info_array[i]->dirs[q])%buffersize;

            //Case: Send chunk_times same-bytes messages.
            if(last_bytes==0)
            {
                for(k=0;k<chunk_times;k++)
                    write(PCfd,&info_array[i]->dirs[q][k*buffersize],buffersize);
            }
            //Case: Send chunk_times-1 same-bytes messages and last message with last_bytes.
            else
            {
                for(k=0;k<=chunk_times;k++)
                {
                    if(k==chunk_times)  
                        write(PCfd,&info_array[i]->dirs[q][k*buffersize],last_bytes);
                    else    
                        write(PCfd,&info_array[i]->dirs[q][k*buffersize],buffersize); 
                }
            }
            //Write a space between countries.
            if(q!=(info_array[i]->num_of_dirs-1))   write(PCfd,space,1);
        }
        //Write character which will indicate that we have next info and specifically the name of input_dir!
        write(PCfd,new_line,1);
        write(PCfd,path,strlen(path));

        //Write character which will indicate that we have next info and specifically the name of CP_fifo!
        write(PCfd,new_line,1);
        write(PCfd,info_array[i]->CPfifo,strlen(info_array[i]->CPfifo));
        
        //Write character which will indicate that message is over!
        write(PCfd,last_char,1);
        //Father close named pipe so as to Worker understand that he can read now...
        close(PCfd);
    }

    // //Father is ready to get Summary Statistics from every Worker so as to print them to stdout.
    for(i=0;i<(num_of_forks);i++)   all_sum_stats.append(Father_Read(info_array[i]->CPfifo,buffersize));

    cout << all_sum_stats;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////Father is ready to make requests to Workers.../////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
    int SUCCESS=0,FAIL=0;
    
    //Sigaction for SIGCHLD signal...
    struct sigaction act;
	memset (&act, 0, sizeof(act)); 
	act.sa_sigaction = sigchildhandler;
 	act.sa_flags = SA_RESTART | SA_SIGINFO;
	if(sigaction(SIGCHLD, &act, NULL) < 0) 
	{
		perror("Father SIGCHLD ERROR!");
		return 1;
	}
    
    //Sigaction for SIGINT signal...
    struct sigaction act1;
	memset (&act1, 0, sizeof(act1)); 
	act1.sa_sigaction = sigintorquithandler;
 	act1.sa_flags = SA_SIGINFO;
	if(sigaction(SIGINT, &act1, NULL) < 0) 
	{
		perror("Father SIGINT ERROR!");
		return 1;
	}

    //Sigaction for SIGQUIT signal...
    struct sigaction act2;
	memset (&act2, 0, sizeof(act2)); 
	act2.sa_sigaction = sigintorquithandler;
 	act2.sa_flags = SA_SIGINFO;
    if(sigaction(SIGQUIT, &act2, NULL) < 0) 
	{
		perror("Father SIGQUIT ERROR!");
		return 1;
	}

    string input;
    while(1) 
    {       
        //Get the request from user...
        cout << endl;
        getline(cin,input);
        stringstream rt(input);
        string words[9];
        i=0;
        //Get word by word user's request. 
        while(rt >> words[i]) i++;   

        //Case: One of Workers has died so first replace it and then answer to request.
        if(died_pid!=-1)
        {
            //Firstly find the info about the dead child.
            for(q=0;q<num_of_forks;q++)
                if(info_array[q]->pid==died_pid)    
                    break;
            //Create a new Worker and he's going to run ./worker...
            pid = fork();
            if(pid==0)
            {
                char p[40];
                char buff[10];
                sprintf(buff, "%d", buffersize);
                sprintf(p,"./executable/worker");
                char* arr[] = { p, info_array[q]->PCfifo, buff, NULL};
                execv("./executable/worker", arr);
            }        

            //Father replaces the dead Worker with a new one.  
            FatherReplaceDiedChild(info_array[q],buffersize,&path);

            //Update pid with new Worker's pid...
            info_array[q]->pid = pid;

            //In the end turn it off again...
            died_pid=-1;
        }

        //Case: End of program.
        if(words[0]=="/rightexit" && i==1) 
        {
            cout << "exiting" << endl;
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,input,buffersize);
            break;
        }
        else if(words[0]=="/exit" && i==1) 
        {
            for(i=0;i<(num_of_forks);i++)   kill(info_array[i]->pid,SIGKILL);
            
            SUCCESS++;
            ofstream myfile;
            string process = "log_files/log_file.";
            process.append(to_string(getpid()));
            myfile.open(process);
            for(i=0;i<countries;i++)
            {
                myfile << countries_array[i];
                myfile << "\n";
            }
            myfile << "Total ";
            myfile << to_string((SUCCESS+FAIL));
            myfile << "\n";
            myfile << "SUCCESS ";
            myfile << to_string((SUCCESS));
            myfile << "\n";
            myfile << "FAIL ";
            myfile << to_string((FAIL));
            myfile << "\n";
            myfile.close();

            break;
        }
        else if(words[0]=="/listCountries" && i==1)
        {
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,input,buffersize);
            for(i=0;i<(num_of_forks);i++)   message.append(Father_Read(info_array[i]->CPfifo,buffersize));
            cout << message;
            message="";
            SUCCESS++;
        }
        else if(words[0]=="/diseaseFrequency" && (i==4 || i==5))
        {
            //If we don't have restriction about country then call all Workers so as to collect the right info.
            if(i==4)
            {
                int errors=0;
                for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,input,buffersize);
                for(i=0;i<(num_of_forks);i++)   
                {
                    f = Father_Read(info_array[i]->CPfifo,buffersize);
                    if(f!="ERROR")  cases+=stoi(f);
                    else    errors++;
                }
                if(errors!=num_of_forks)  
                {
                    cout << cases;
                    SUCCESS++;
                }
                else
                {
                    cerr << "ERROR";
                    FAIL++;
                }
                cases=0;
            }   
            //If we have restriction about country then call only the Worker who manipulates the specific country.
            else
            { 
                bool flag=false;
                for(i=0;i<(num_of_forks);i++)   
                {
                    for(j=0;j<info_array[i]->num_of_dirs;j++)
                    {
                        string ft = info_array[i]->dirs[j];
                        
                        if(ft==words[4])
                        {
                            Father_Write(info_array[i]->PCfifo,input,buffersize);
                            ft = Father_Read(info_array[i]->CPfifo,buffersize);
                            if(ft!="ERROR") 
                            {
                                cout << stoi(ft);
                                SUCCESS++;
                            }
                            else    
                            {
                                cerr << "ERROR";
                                FAIL++;
                            }
                            flag=true;
                            break;
                        }
                    }
                    if(flag)    break;
                }
                if(!flag)   
                {
                    cerr << "ERROR";
                    FAIL++;
                }
            }
        }
        else if(words[0]=="/searchPatientRecord" && i==2)
        {
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,input,buffersize);
            
            int errors=0;
            for(i=0;i<(num_of_forks);i++)   
            {
                f = Father_Read(info_array[i]->CPfifo,buffersize);
                if(f!="ERROR")  
                {
                    cout << f;
                    SUCCESS++;
                }
                else    errors++;
            }
            if(errors==num_of_forks)    
            {
                cerr << "ERROR";
                FAIL++;
            }
        }
        else if(words[0]=="/topk-AgeRanges" && i==6)
        {
            bool flag=false;
            for(i=0;i<(num_of_forks);i++)   
            {
                for(j=0;j<info_array[i]->num_of_dirs;j++)
                {
                    string ft = info_array[i]->dirs[j];
                    
                    if(ft==words[2])
                    {
                        Father_Write(info_array[i]->PCfifo,input,buffersize);
                        string q = Father_Read(info_array[i]->CPfifo,buffersize);
                        if(q!="ERROR")  
                        {
                            cout << q;
                            SUCCESS++;
                        }
                        else    
                        {
                            cerr << "ERROR";    
                            FAIL++;
                        }
                        flag=true;
                        break;
                    }
                }
                if(flag)    break;
            }    
            //Case: Didn't found the specific country.
            if(!flag)   
            {
                cerr << "ERROR";
                FAIL++;;
            }
        }
        else if(words[0]=="/numPatientAdmissions" && (i==4 || i==5))
        {
            //If we don't have restriction about country then call all Workers so as to collect the right info.
            if(i==4)
            {
                string admissions;
                int errors=0;
                for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,input,buffersize);
                for(i=0;i<(num_of_forks);i++)   
                {
                    f = Father_Read(info_array[i]->CPfifo,buffersize);
                    if(f!="ERROR")  admissions.append(f);
                    else    errors++;
                }
                if(errors!=num_of_forks)  
                {
                    cout << admissions;
                    SUCCESS++;
                }
                else    
                {
                    cerr << "ERROR";
                    FAIL++;
                }
            }   
            //If we have restriction about country then call only the Worker who manipulates the specific country.
            else
            { 
                bool flag=false;
                for(i=0;i<(num_of_forks);i++)   
                {
                    for(j=0;j<info_array[i]->num_of_dirs;j++)
                    {
                        string ft = info_array[i]->dirs[j];
                        
                        if(ft==words[4])
                        {
                            Father_Write(info_array[i]->PCfifo,input,buffersize);
                            ft = Father_Read(info_array[i]->CPfifo,buffersize);
                            if(ft!="ERROR")
                            {
                                cout << ft;
                                SUCCESS++;
                            }
                            else
                            {
                                cerr << "ERROR";
                                FAIL++;
                            }
                            flag=true;
                            break;
                        }
                    }
                    if(flag)    break;
                }
                //Case: Didn't found the specific country.
                if(!flag)
                {
                    cerr << "ERROR";
                    FAIL++;
                }
            }            
        }
        else if(words[0]=="/numPatientDischarges" && (i==4 || i==5))
        {
            //If we don't have restriction about country then call all Workers so as to collect the right info.
            if(i==4)
            {
                string discharges;
                int errors=0;
                for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,input,buffersize);
                for(i=0;i<(num_of_forks);i++)   
                {
                    f = Father_Read(info_array[i]->CPfifo,buffersize);
                    if(f!="ERROR")  discharges.append(f);
                    else    errors++;
                }
                if(errors!=num_of_forks)  
                {
                    cout << discharges;
                    SUCCESS++;
                }
                else    
                {
                    cerr << "ERROR";
                    FAIL++;
                }
            }   
            //If we have restriction about country then call only the Worker who manipulates the specific country.
            else
            { 
                bool flag=false;
                for(i=0;i<(num_of_forks);i++)   
                {
                    for(j=0;j<info_array[i]->num_of_dirs;j++)
                    {
                        string ft = info_array[i]->dirs[j];
                        
                        if(ft==words[4])
                        {
                            Father_Write(info_array[i]->PCfifo,input,buffersize);
                            ft = Father_Read(info_array[i]->CPfifo,buffersize);
                            if(ft!="ERROR") 
                            {
                                cout << ft;
                                SUCCESS++;
                            }
                            else    
                            {
                                cerr << "ERROR";
                                FAIL++;
                            }
                            flag=true;
                            break;
                        }
                    }
                    if(flag)    break;
                }
                //Case: Didn't found the specific country.
                if(!flag)   
                {
                    cerr << "ERROR";
                    FAIL++;
                }
            }            
        }
        else if(words[0]=="/print" && i==1)
        {
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,words[0],buffersize);
        } 
        else if(words[0]=="/printwithtree" && i==1)
        {
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,words[0],buffersize);
        }
        else if(words[0]=="/print_length" && i==1)
        {
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,words[0],buffersize);
        }  
        else if(words[0]=="/print_records" && i==1)
        {
            for(i=0;i<(num_of_forks);i++)   Father_Write(info_array[i]->PCfifo,words[0],buffersize);
        }
        else if(words[0]=="/fatherpid" && i==1)
        {
            cout << getpid();
        }
        else if(sigintorquit)
	{
	    cout << "Father kills all Workers and he's done..." << endl;
	}
	else    
        {
            cout << "Try again user! ";
        }

        //Case: Father got SIGINT or SIGQUIT... 
        if(sigintorquit)
        {
            for(i=0;i<(num_of_forks);i++)   kill(info_array[i]->pid,SIGKILL);
            ofstream myfile;
            string process = "log_files/log_file.";
            process.append(to_string(getpid()));
            myfile.open(process);
            for(i=0;i<countries;i++)
            {
                myfile << countries_array[i];
                myfile << "\n";
            }
            myfile << "Total ";
            myfile << to_string((SUCCESS+FAIL));
            myfile << "\n";
            myfile << "SUCCESS ";
            myfile << to_string((SUCCESS));
            myfile << "\n";
            myfile << "FAIL ";
            myfile << to_string((FAIL));
            myfile << "\n";
            myfile.close();

            sigintorquit=false;
            break;
        }
    }
    

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////End of program,after is deallocation of memory//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Father is waiting for Workers...
    pid_t wpid;
    int status;
    while((wpid = wait(&status)) > 0);

    //Delete all named pipes.
    for(i=0;i<num_of_forks;i++)
    {
        if(remove(info_array[i]->PCfifo) != 0) perror("Cannot remove named pipe!\n");
        if(remove(info_array[i]->CPfifo) != 0) perror("Cannot remove named pipe!\n");
    }

    //Deallocation of memory...
    for(i=0;i<num_of_forks;i++)
    {
        for(k=0;k<info_array[i]->num_of_dirs;k++)
        {
            //For each country name.
            delete[] info_array[i]->dirs[k];
        }
        //For each array of countries(dirs).
        delete[] info_array[i]->dirs;
        //For each Worker_Info.
        delete info_array[i];
    }
    delete[] info_array;
    for(i=0;i<countries;i++)    delete[] countries_array[i];
    delete[] countries_array;
    delete[] path;

    return 0;
}
