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
int cursor = 1;
vector <string> attributes;
string attribute2="",command="",attribute1="";
int max_i=10,firstindex=0,lastindex=firstindex+max_i,nrows,ncols;
static struct termios ins, ns;
string searchingdir;
struct winsize w;

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
                if(statement[i]==92)
                {
                        s+=' ';
                        i++;
                }
                else if(statement[i]!=' ')
                s+=statement[i];
                else{
                        if(f==0 && s!="")
                        {
                                command=s;
                                attributes.push_back(s);
                                f++;
                        }
                        else if(f==1 && s!="")
                        {
                                attribute1=s;
                                attributes.push_back(s);
                                f++;
                        }
                        else if(s!="")
                        {
                                attribute2=s;
                                attributes.push_back(s);
                        }
                        s="";
                        
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
        cout<<i+1<<"\t";
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
            if(ncols>95)
            cout<<"\t"<<fs.st_gid<<"\t"<<fs.st_uid<<"\t"<<timbuf<<"\t"<< st_s<<pr;
        }
                if(string(entity->d_name).length()>20)
                cout<<"\t"<<string(entity->d_name).substr(0,17)<<"..."<<endl;
                else
		cout<<"\t"<<entity->d_name<<endl;
		entity=readdir(dir);
	}
        moveCursor(nrows-5,0);
        cout<<"Total Files and Directories: "<<filess<<endl;
        cout << "\n--------------Normal Mode-------------- \nPress : to switch to Command Mode\t"<<currentDir;
        moveCursor(1,1);
	closedir(dir);
}

bool sorc(dirent* a ,dirent* b)
{
        return strcasecmp((a->d_name),(b->d_name))<0;
}

void SetCurrentDir(const char * dir) {
        DIR * dp;
        struct dirent * entry;
        firstindex=0;
        lastindex=max_i;
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
        //cout<<files[0]->d_name<<endl;
        //cout<<currentDir;
        return;
}

void isignal(int signal)
{
        ioctl(STDOUT_FILENO, TIOCGWINSZ, & w);
        nrows=w.ws_row;
        ncols=w.ws_col;
        if(nrows<20)
        max_i=nrows-7;
        else
        max_i=10;
        SetCurrentDir(currentDir);
}

void Keyboard()
{
    tcgetattr(0, & ins);
        ns = ins;
        //ns.c_iflag &= ~(ICRNL | IXON);
        ns.c_lflag &= ~(ECHO | ICANON | ISIG);
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
         if(nrows<25)
         lastindex=firstindex+max_i;
         else
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

bool searchfile(string filename)
{
        DIR * d;
        struct dirent* file;
        struct stat buf;
        d=opendir(searchingdir.c_str());
        while((file=readdir(d))!=NULL)
        {
                stat((searchingdir+"/"+file->d_name).c_str(),&buf);
                if(S_ISREG(buf.st_mode))
                {
                        if(string(file->d_name)==filename)
                        return 1;
                }
                else{
                        if(string(file->d_name)!= "." && string(file->d_name)!=".." && string(file->d_name).substr(0,1)!=".")
                        {
                                if(string(file->d_name)==filename)
                                return 1;
                                string temp=searchingdir;
                                searchingdir=searchingdir+"/"+file->d_name;
                                if(searchfile(filename))
                                return 1;
                                searchingdir=temp;
                        }
                }
        }
        closedir(d);
        return 0;
}

void setMode()
{
        moveCursor(nrows-3, 0);
        cout << "--------------Command Mode-------------- \nPress ESC to switch to Normal Mode\n";
        return;
}

void copyFile(string filename, string destination) {
        char block[4096];
        int in , out, nread,n;
        in = open(filename.c_str(), O_RDONLY);
        out = open((destination).c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
        while (1) {
        nread = read(in, block, 4096);
        if (nread == -1) {
            printf("Can't Read.\n");
            return;
        }
        n = nread;
        if (n == 0) break;
        nread = write(out, block, n);
        if (nread == -1) {
            printf("Can't Write.\n");
            return;
        }
    }
        close(in);
        close(out);
}

void copydir(string s1,string s2)
{
        DIR * d;
        struct dirent* file;
        struct stat buf;
        d=opendir(s1.c_str());
        while((file=readdir(d))!=NULL)
        {
                stat((s1+"/"+file->d_name).c_str(),&buf);
                if(S_ISREG(buf.st_mode))
                {
                        //cout<<s2+"/"+string(file->d_name)<<endl;
                        copyFile(s1+"/"+string(file->d_name), s2+"/"+string(file->d_name));
                }
                else{
                        if(string(file->d_name)!="." && string(file->d_name)!=".."){
                                mkdir((s2 + "/" + string(file->d_name)).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
                                copydir(s1+ "/" + string(file->d_name), s2 + "/" + string(file->d_name));
                        }
                }
        }
        chdir("..");
        closedir(d);
        return;
}

char* ExtractPath(string attribute2)
{
        char* p;
        if(attribute2[0]=='~')
        p = realpath(("/home/dell"+(attribute2.substr(1))).c_str(),NULL);
        else
        p = realpath(attribute2.c_str(),NULL);
        return p;
}

int deletefile(string attribute1)
{
        return remove(attribute1.c_str());
}

int deletedir(string deletingfile)
{
        DIR * d;
        struct dirent* file;
        struct stat buf;
        if((d=opendir(deletingfile.c_str()))==NULL)
        {
                cout<<"Can't Open Directory";
                return -1;
        }
        while((file=readdir(d))!=NULL)
        {
                stat((deletingfile+"/"+string(file->d_name)).c_str(),&buf);
                if(S_ISREG(buf.st_mode))
                {
                        deletefile(deletingfile+"/"+string(file->d_name));
                }
                else{
                        if(string(file->d_name)!="." && string(file->d_name)!=".."){
                                deletedir(deletingfile+"/"+string(file->d_name));
                                rmdir(file->d_name);
                        }
                }
        }
        closedir(d);
        if(deletingfile!="/home/dell")
        rmdir(deletingfile.c_str());
        return 0;
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
                attributes.clear();
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
                                searchingdir=currentDir;
                                SetCurrentDir(currentDir);
                                setMode();
                                if(searchfile(attribute1))
                                cout<<"True";
                                else
                                cout<<"False";
                        }
                        else if(command=="create_dir")
                        {
                                string location,filename;
                                filename=attribute1;
                                location=attribute2;
                                SetCurrentDir(currentDir);
                                setMode();
                                if(ExtractPath(attribute2)==NULL)
                                cout<<"Invalid Path";
                                mkdir((location + '/' + filename).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
                                }
                        else if(command=="create_file")
                        {
                                string location,filename;
                                filename=attribute1;
                                location=attribute2;
                                SetCurrentDir(currentDir);
                                setMode();
                                if(ExtractPath(attribute2)==NULL)
                                cout<<"Invalid Path";
                                open((location + '/' + filename).c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                        }
                        else if(command=="goto")
                        {
                                char* p=ExtractPath(attribute1);
                                if(p==NULL)
                                {
                                        SetCurrentDir(currentDir);
                                        setMode();
                                        cout<<"Invalid Path";
                                }
                                else{
                                SetCurrentDir(p);
                                setMode();
                                }
                        }
                        else if(command=="delete_file")
                        {
                                int k=deletefile(attribute1);
                                SetCurrentDir(currentDir);
                                setMode();
                                if(k==-1)
                                cout<<"Unsuccessful";
                        }
                        else if(command=="delete_dir")
                        {
                                int k=deletedir(attribute1);
                                SetCurrentDir(currentDir);
                                setMode();
                                if(k==-1)
                                cout<<"Unsuccessful";
                        }
                        else if(command=="rename")
                        {
                                int k=rename(attribute1.c_str(), attribute2.c_str());
                                SetCurrentDir(currentDir);
                                setMode();
                                if(k==-1)
                                cout<<"Unsuccessful";
                        }
                        else if(command=="copy")
                        {
                                int path_error=0;
                                //cout<<attributes.size();
                                for(int i=1;i<attributes.size()-1;i++)
                                {
                                        //cout<<attributes[i]<<endl;
                                        struct stat buf;
                                        stat(attributes[i].c_str(),&buf);
                                        if(S_ISDIR(buf.st_mode))
                                        {
                                                char* location=ExtractPath(attributes[i]);
                                                //cout<<location;
                                                string filename="";
                                                for(int j=attributes[i].size()-1;j>=0;j--)
                                                {
                                                        if(attributes[i][j]!='/')
                                                        filename=attributes[i][j]+filename;
                                                        else
                                                        break;
                                                }
                                                char* p=ExtractPath(attribute2);
                                                if(p==NULL || location==NULL)
                                                        path_error=-1;
                                                else{
                                                mkdir((string(p) + "/" + filename).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
                                                copydir(location, string(p) + "/" + filename);
                                                }
                                        }
                                        else 
                                        {
                                                char* location=ExtractPath(attributes[i]);
                                                //cout<<location;
                                                string filename="";
                                                for(int j=attributes[i].size()-1;j>=0;j--)
                                                {
                                                        if(attributes[i][j]!='/')
                                                        filename=attributes[i][j]+filename;
                                                        else
                                                        break;
                                                }
                                                char* p=ExtractPath(attribute2);
                                                if(p==NULL || location==NULL)
                                                        path_error=-1;
                                                else{
                                                copyFile(location, string(p) + "/" + filename);
                                                }
                                        }
                                }
                                SetCurrentDir(currentDir);
                                setMode();
                                if(path_error==-1)
                                cout<<"Invalid Path";
                        }
                        else if(command=="move")
                        {
                                cout<<attributes.size()<<endl;
                                int k=0;
                                for(int j=1;j<attributes.size()-1;j++)
                                {
                                        string filename="";
                                        for(int i=attributes[j].size()-1;i>=0;i--)
                                        {
                                                if(attributes[j][i]!='/')
                                                filename=attributes[j][i]+filename;
                                                else
                                                break;
                                        }
                                        k=rename(attributes[j].c_str(), (attribute2+"/"+filename).c_str());
                                }
                                SetCurrentDir(currentDir);
                                setMode();
                                if(k==-1)
                                cout<<"Unsuccessful";
                        }
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
    ioctl(STDOUT_FILENO, TIOCGWINSZ, & w);
        nrows=w.ws_row;
        ncols=w.ws_col;
        if(nrows<20)
        max_i=nrows-7;
        else
        max_i=10;
    signal(SIGWINCH, isignal);
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
