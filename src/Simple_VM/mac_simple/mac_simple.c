//
// Created by Ujjwal Krishnamurthi on 7/26/20.
//

#include <stdio.h>
#include <stdbool.h>

bool running = true;
int IP = 0;
int SP = -1;

int stack[256];

typedef enum {
    PSH,
    ADD,
    POP,
    HLT
} InstructionSet;

const int program[] = {
        PSH, 5,
        PSH, 6,
        ADD,
        POP,
        HLT
};

int fetch() {
    return program[IP];
}

void eval(int instr) {
    switch (instr) {
        case HLT: {
            running = false;
            printf("done\n");
            break;
        }
        case PSH: {
            SP++;
            stack[SP] = program[++IP];
            break;
        }
        case POP: {
            int val_popped = stack[SP--];
            printf("popped %d\n", val_popped);
            break;
        }
        case ADD: {
            // first we pop the stack and store it as a
            int a = stack[SP--];

            // then we pop the top of the stack and store it as b
            int b = stack[SP--];

            // we then add the result and push it to the stack
            int result = b + a;
            SP++; // increment stack pointer **before**
            stack[SP] = result; // set the value to the top of the stack

            // all done!
            break;
        }
        default:
            break;
    }
}

int main() {
    while (running) {
        eval(fetch());
        IP++; // increment the ip every iteration
    }
}