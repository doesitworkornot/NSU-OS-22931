﻿#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char* argv[])
{
     int status;
     if (!fork())
     {
         execvp(argv[1], &argv[1]);
         perror(argv[1]);
         exit(127);
     }
     wait(&status);
     printf("exit status: %d\n", WEXITSTATUS(status));
     return 0;
}