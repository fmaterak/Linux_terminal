#include <cstdio>
#include <iostream>
#include <errno.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


int main(){
    int master= getpt();
    if(master == -1){
        perror("getpt");
        return 1;
    }

    char slave_path[256]{};
    if (ptsname_r(master, slave_path, sizeof(slave_path)) != 0){
        perror("ptsname_r");
        close(master);
        return 2;
    }

    if(grantpt(master) != 0){
        perror("grantpt");
        close(master);
        return 3;
    }
    if(unlockpt(master) != 0){
        perror("unlockpt");
        close(master);
        return 4;
    }

    int slave = open(slave_path, O_RDWR);
    if(slave < 0){
        perror("open");
        close(master);
        return 2;
    }

    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        return 5;
    }
    if(pid == 0){
        close(0);
        close(1);
        close(2);
        close(3);


        dup2(slave, 0);
        dup2(slave, 1);
        dup2(slave, 2);

        close(slave);

        if(execl("/bin/bash", "/bin/bash", nullptr) ==-1){
            return 6;
        }

    }
    close(slave);
    waitpid(pid, nullptr, 0);
    std::cout<<"End"<<std::endl;
} //main()
