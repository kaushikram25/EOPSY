#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define WITH_SYMBOLS

int INTERRUPTED = 0;

struct children { 
    int PID;
    struct children* next; 
    int exitStatus;
};

void handle_sigterm(int sig)
{
    printf("Process[%d]: terminated with signal %d\n", getpid(), sig);
}

void handle_sigint(int sig)
{
    INTERRUPTED = 1;
    printf("Process[%d]: terminated with signal %d\n", getpid(), sig);
}

void clearChildren(struct children* head) {
    while(head != NULL) {
        struct children* next = head->next;
        printf("Parent[%d]: sending SIGTERM signal to Child[%d]\n", getpid(), head->PID);
        kill(head->PID, SIGTERM);
        head = next;
    }
}

void printChildren(struct children* head) {
    struct children *tmp = head;
    while(tmp) {
        printf("Child PID: %d\n", tmp->PID);
        tmp = tmp->next;
    }
}

int main(int argc, char* argv[])
{
    #ifdef WITH_SYMBOLS
    // ignore all signals
    int i;
    for(i = 0; i < 32; i++) {
        signal(i, SIG_IGN);
    }
    // handle only SIGCHLD and SIGINT signals
    signal(SIGCHLD, SIG_DFL);
    signal(SIGINT, handle_sigint);
    #endif

    struct children* head = NULL;
    int CURR_NUM_CHILD = 0;
    int NUM_CHILD = 0;
    if(!argv[1]) {
        printf("error: not enough arguments\n");
        return 1;
    }
    NUM_CHILD = atoi(argv[1]);
    printf("no child processes requested: %d\n", NUM_CHILD);
    printf("Root PID: %d\n", getpid());
    #ifdef WITH_SYMBOLS
    while(NUM_CHILD > CURR_NUM_CHILD++ && !INTERRUPTED) {
    #else
    while(NUM_CHILD > CURR_NUM_CHILD++) {
    #endif
        if(head == NULL) {
            head = malloc(sizeof(struct children));
            head->next = NULL;
            head->PID = fork();
        } else {
            struct children* prev_head = head;
            head = malloc(sizeof(struct children));
            head->next = prev_head;
            head->PID = fork();
        }
        if(!head->PID) {
            break;
        }
        if(head->PID < 0) {
            clearChildren(head);
            printf("error: could not create process\n");
        }
        sleep(1);
    }
    #ifdef WITH_SYMBOLS
    if(INTERRUPTED) {
        clearChildren(head);
        printf("Parent[%d]: PROCESS INTERRUPTED!\n", getpid());
    }
    #endif
    if(head && head->PID) {
        int not_finished_count = 1;
        int loop_count = 0;
        while(not_finished_count) {
            struct children * tmp = head;
            not_finished_count = 0;
            usleep(500*1000); //0.5s
            printf("Loop iteration: %d\n", loop_count++);
            while(tmp) {
                int status;
                int waitStatus = waitpid(tmp->PID, &status, WNOHANG);
                if(waitStatus == 0) {
                    not_finished_count++;
                    printf("not finished yet!\n");
                } else {
                    tmp->exitStatus = WEXITSTATUS(status);
                    printf("Child[%d]: Exit status %d\n", tmp->PID, tmp->exitStatus);
                }
                tmp = tmp->next;
            }
        }
        printf("Parent[%d]: There are no more child processes\n", getpid());
    #ifdef WITH_SYMBOLS
    } else if(!INTERRUPTED) {
        for(i = 0; i < 32; i++) {
            signal(i, SIG_IGN);
        }
        signal(SIGTERM, handle_sigterm);
    #else
    } else {
    #endif
        printf("Child[%d]: created, parent: %d\n", getpid(), getppid());
        sleep(10);
        printf("Child[%d]: completed execution\n", getpid());
    }
    #ifdef WITH_SYMBOLS
    for(i = 0; i < 32; i++) {
        signal(i, SIG_DFL);
    }
    #endif
    return 0;
}