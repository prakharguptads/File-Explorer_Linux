#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <iostream>
#include <bits/stdc++.h>
#include<pwd.h>
#include<grp.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <unistd.h>

#define move moveCursor(cursor, 0)

using namespace std;
static int ch_=-1;
size_t dirSize = 1024;
char currentDir[1024];
vector <dirent*> files;
stack <string> backs,forwards;
int cursor = 0;
string attribute2="",command="",attribute1="";
int max_i=10,firstindex=0,lastindex=firstindex+max_i;
static struct termios ins, ns;

void moveCursor(int x, int y) {
        cout << "\033[" << x << ";" << y << "H";
        fflush(stdout);
}

void resolvestatement(string statement)
{
        statement+=' ';
        string s="";
        int f=0;
        for(int i=0;i<statement.length();i++)
        {
                if(statement[i]!=' ')
                s+=statement[i];
                else{
                        if(f==0)
                        command=s;
                        else if(f==1)
                        attribute1=s;
                        else
                        attribute2=s;
                        s="";
                        f++;
                }
        }
}

void showCurrentDir(const char* dirname)
{
    //cout<<s;
    printf("\033c");
    //cout<<files.size();
    struct stat fs;
    DIR * dir = opendir(dirname);
	if(dir == NULL)
	return;
	struct dirent* entity;
	entity = readdir(dir);
        //max_i=10;
        int filess=files.size();
	for(int i=firstindex;i<min(lastindex,filess);i++)
	{
                entity=files[i];
        if (!lstat(entity->d_name, &fs))
        {
        mode_t val;
        val=(fs.st_mode & ~S_IFMT);
        printf((S_ISDIR(fs.st_mode)) ? "d" : "-");
        (val & S_IRUSR) ? printf("r") : printf("-");
        (val & S_IWUSR) ? printf("w") : printf("-");    
        (val & S_IXUSR) ? printf("x") : printf("-");
        (val & S_IRGRP) ? printf("r") : printf("-");
        (val & S_IWGRP) ? printf("w") : printf("-");
        (val & S_IXGRP) ? printf("x") : printf("-");
        (val & S_IROTH) ? printf("r") : printf("-");
        (val & S_IWOTH) ? printf("w") : printf("-");
        (val & S_IXOTH) ? printf("x") : printf("-");
            time_t t = fs.st_mtime;
            struct tm lt;
            localtime_r(&t, &lt);
            char timbuf[80];
            strftime(timbuf, sizeof(timbuf), "%d.%m.%Y %H:%M:%S",  &lt);
            float st_s= fs.st_size;
            int l=0;
            string pr="B";
            while(st_s>1024)
            {
                st_s/=1024;
                l++;
            }
            if(l==1)
            pr="KB";
            else if(l==2)
            pr="MB";
            else if(l==3)
            pr="GB";
            else if(l==4)
            pr="TB";
            cout<<"\t"<<fs.st_gid<<"\t"<<fs.st_uid<<"\t"<<timbuf<<"\t"<< st_s<<pr<<"\t";
        }
		cout<<entity->d_name<<endl;
		entity=readdir(dir);
	}
        moveCursor(12,0);
        cout << "\nMode: Normal Mode. Press : to switch to Command Mode\t"<<currentDir;
        moveCursor(0,0);
	closedir(dir);
}

bool sorc(dirent* a ,dirent* b)
{
        return ((a->d_name)<(b->d_name));
}

void SetCurrentDir(const char * dir) {
        DIR * dp;
        struct dirent * entry;
        firstindex=0;
        lastindex=10;
        dp = opendir(dir);
        if (dp == NULL) {
                fprintf(stderr, "No Such Directory!\n");
                return;
        }
        chdir(dir);
        getcwd(currentDir, dirSize);
        //backs.push(string(currentDir));
        files.clear();
        while ((entry = readdir(dp)) != NULL) {
                files.push_back(entry);
        }
        closedir(dp);
        // firstIndex = 0;
        cursor = 0;
        sort(files.begin(),files.end(),sorc);
        showCurrentDir(dir);
        //cout<<currentDir;
        return;
}

void Keyboard()
{
    tcgetattr(0, & ins);
        ns = ins;
        ns.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(0, TCSANOW, & ns);
        return;
}

int readelement() {
        char ch;
        if (ch_ != -1) {
                ch = ch_;
                ch_ = -1;
                return ch;
        }
        read(0, & ch, 1);
        return ch;
}

void Up() {
        if (cursor > 1) {
                cursor--;
                move;
                return;
        }
        else if (firstindex != 0) {
                firstindex--;
         lastindex--;
         showCurrentDir(currentDir);
                move;
        }
        return;
}

void Down() {
        int filess=files.size();
        if (cursor < min(filess,max_i)) {
            cursor++;
            move;
                return;
        }
        else if (lastindex != files.size() && cursor!=filess) {
         firstindex++;
         lastindex++;
         showCurrentDir(currentDir);
                move;
        }
        return;
}

void Open()
{
    struct stat statbuf;
    char * file = files[firstindex+cursor-1] -> d_name;
    lstat(file, & statbuf); 
      if (S_ISREG(statbuf.st_mode))
        {
           pid_t pid = fork();
                if (pid == 0) {
                execl("/usr/bin/xdg-open", "xdg-open", file ,NULL);
                exit(1);
                }
        } else {
            if (strcmp(file, "..") == 0 && strcmp(currentDir, "/home") == 0)
            return;
            if (strcmp(file, ".") == 0)
            return;
            while(forwards.size()!=0)
            forwards.pop();
            backs.push(string(currentDir));
            SetCurrentDir((string(currentDir) + '/' + string(file)).c_str());
        }
        return;
}

void forward()
{
        if(forwards.size()!=0)
        {
                backs.push(string(currentDir));
                SetCurrentDir((forwards.top()).c_str());
        }
        else
        return;
        forwards.pop();
}

void backward()
{
        //cout<<backs.size();
        if(backs.size()!=0)
        {
                forwards.push(string(currentDir));
                SetCurrentDir((backs.top()).c_str());
        }
        else
        return;
        backs.pop();
        return ;
}

void backspace()
{
        //cout<<"a"<<endl;
        while(forwards.size()!=0)
            forwards.pop();
            backs.push(string(currentDir));
        SetCurrentDir((string(currentDir)+"/..").c_str());
        return ;
}

// string read()
// {
//         char ch='$';
//         string input1="";
//                 //printf("$ ");
//                 while((ch = cin.get()) != ' ' && ch!=10){
//                         if(ch==127 and input1.length()>=1){
//                         input1.pop_back();
//                         printf("\b \b");
//                         }
//                         else if(ch==127){}
//                         else if(ch!=10)
//                         input1+=ch;
//                         cout<<ch;
//                 }
//                 return input1;
// }

// bool searchfile()
// {
//         for(int i=0;i<files.size();i++)
//         {
//                 if(files[i]->d_name!="." || files[i]->d_name!="..")
//                 searchfile(files[i])
//         }
//         return 0;
// }

void setMode()
{
        moveCursor(13, 0);
        cout << "Mode: Command Mode. Press ESC to switch to Normal Mode\n";
        return;
}

int commandMode()
{
        //tcsetattr(0, TCSANOW, & ins);
        setMode();
        char ch='$';
        while(1)
        {
                int f=1;
                string statement="";
                printf("$ ");
                while((ch = cin.get()) != 27 and ch != 10){
                        if(ch==127 and statement.length()>=1){
                        statement.pop_back();
                        printf("\b \b");
                        }
                        else if(ch==127){}
                        else if(ch!=10)
                        statement+=ch;
                        cout<<ch;
                }
                //cout<<attribute2;
                resolvestatement(statement);
                //cout<<statement;

                if(ch==27)
                break;
                if(ch==10)
                 {
                        //cout<<endl;
                        if(command=="search")
                        {
                                
                        }
                        else if(command=="create_dir")
                        {
                                string location,filename;
                                filename=attribute1;
                                location=attribute2;
                                mkdir((location + '/' + filename).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
                                }
                        else if(command=="create_file")
                        {
                                string location,filename;
                                filename=attribute1;
                                location=attribute2;
                                open((location + '/' + filename).c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                        }
                        else if(command=="goto")
                        {
                                char* p;
                                if(attribute1[0]=='~')
                                p = realpath((attribute1.substr(1)).c_str(),NULL);
                                else
                                p = realpath(attribute1.c_str(),NULL);
                                SetCurrentDir(p);
                                setMode();
                        }
                        else if(command=="delete")
                        printf("delete");
                        else if(command=="rename")
                        {
                                rename(attribute1.c_str(), attribute2.c_str());
                                SetCurrentDir(currentDir);
                                setMode();
                        }
                        else if(command=="copy")
                        printf("copy");
                        else if(command=="move")
                        printf("move");
                        else if(command=="quit")
                        return 1;
                        cout<<endl;
                }
        }
        move;
        SetCurrentDir(currentDir);
        return 0;
}

int main() { 
    std::cout << std::fixed;
    std::cout << std::setprecision(1);
    char dir[1024]="/home/dell";
    int dirsize=1024;
    printf("\033c");
    SetCurrentDir(dir);
    Keyboard();
    char ch ='$';
    while(ch!='q')
    {
        ch=readelement();
        if (ch == 65)
        Up();
        else if (ch == 66)
        Down();
        else if(ch ==10)
        Open();
        else if(ch == 'h')
        SetCurrentDir("/home/dell");
        else if(ch == 67)
        forward();
        else if(ch==68)
        backward();
        else if(ch == 127)
        backspace();
        else if(ch == ':')
        {
                if(commandMode())
                ch='q';
        }
    }
   tcsetattr(0, TCSANOW, & ins);
   printf("\033[H\033[J");
}