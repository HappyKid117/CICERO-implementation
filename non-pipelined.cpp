#include<bits/stdc++.h>
#include<pthread.h>
using namespace std;

/*
    000 - 0 - Match Any
    001 - 1 - Match (OP)
    010 - 2 - NoMatch (OP)
    011 - 3 - Split (OP)
    100 - 4 - JMP (OP)
    101 - 5 - Accept
    110 - 6 - Accept Partial
*/

bool stage0 = false;
bool stage1 = false;
bool stage2 = false;
bool stage3 = false;

tuple < array<int,2> > s0_s1;
tuple < array<int,2>, array<int,3> > s1_s2;
tuple < array<int,3>, array<int,2> > s2_s3; 
tuple < array<int,2> > s3_s0;

bool acceptParital = false;
bool accept = false;
bool reject = false;
bool endAll = false;

bool inputSelector = true;
bool fifoSelector = true;
bool addToFIFO = true;
int currentFIFO = 1;

queue<array<int,2>> FIFO1;
queue<array<int,2>> FIFO2;

array<int,2> output1; // New Pc, FIFO
array<int,2> output2; // New Pc, FIFO // used in stage 3
array<int,3> finalOutput; // Validity, New PC, FIFO
array<int,2> input1; // PC, FIFO
array<int,3> input2; // Instruction, PC, FIFO
int currentCharacter = 0;

string S = "acbed";

void* fetchInstruction(void* input){
    array<int,20> instructionMemory;
    instructionMemory[1] = 0b0110000000000100; // split 4
    instructionMemory[2] = 0b0000000000000000; // matchany
    instructionMemory[3] = 0b1000000000000001; // jump 1
    instructionMemory[4] = 0b0010000000000000; // match a
    instructionMemory[5] = 0b0010000000000001; // match b
    instructionMemory[6] = 0b0110000000001011; // split 11
    instructionMemory[7] = 0b0010000000000000; // match a
    instructionMemory[8] = 0b0010000000000001; // match b
    instructionMemory[9] = 0b1000000000001010; // jump 10
    instructionMemory[10] = 0b1100000000000000; // accept_partial
    instructionMemory[11] = 0b0010000000000001; // match b
    instructionMemory[12] = 0b0010000000000001; // match b
    instructionMemory[13] = 0b1000000000001010; // jump 10

    while(true){
        if(endAll) break;
        if(stage1){
            cout<<"> In stage 1\n";
            input2[0] = instructionMemory[input1[0]];
            input2[1] = input1[0];
            input2[2] = input1[1];
            stage1 = false;
            stage2 = true;
        }
    }
    // cout<<"> Fetch Instruction Broken\n";
    pthread_exit(NULL);
}

void* module1(void *input){
    while(true){
        if(endAll) break;
        if(stage2){
            cout<<"> In stage 2\n";
            int instruction = input2[0];
            int opcode = instruction >> 13;
            int OP     = instruction & 0b0001111111111111;
            cout<<"> PC = "<<input2[1]<<" opcode = "<<opcode<<" OP = "<<OP<<endl;
            bool valid = true;

            switch (opcode){
            case 0: // Match Any
                output1[0] = input2[1]+1;
                output1[1] = (input2[2]+1)%2;
                break;
            
            case 1: // Match (OP)
                if((S[currentCharacter]-'a') == OP){
                    output1[0] = input1[0]+1;
                    output1[1] = (input1[1]+1)%2;
                }else{
                    valid = false;
                }
                break;
            
            case 2: // NoMatch
                if(S[currentCharacter] != OP){
                    output1[0] = input1[0]+1;
                    output1[1] = (input1[1]+1)%2;
                }else{
                    valid = false;
                }
                break;
            
            case 3: // SPLIT
                output1[0] = input1[0]+1;
                output1[1] = input1[1];
                break;

            case 4: // JUMP
                output1[0] = OP;
                output1[1] = input1[1];
                break;

            case 6: // Accept Partial
                acceptParital = true;
                break;

            default:
                cout<<"Error: No such instruction "<<opcode<<endl;
                break;
            }

            finalOutput[0] = valid;
            finalOutput[1] = output1[0];
            finalOutput[2] = output1[1];
            addToFIFO = true;
            while(!addToFIFO);

            stage2 = false;
            stage3 = true;
        }
    }
    
    // cout<<"> Module 1 Broken\n";
    pthread_exit(NULL);
}

void* module2(void* input){
    while(true){
        if(endAll) break;
        if(stage3){
            
            cout<<"> In stage 3\n";
            int instruction = input2[0];
            int opcode = instruction >> 13;
            int OP     = instruction & 0b0001111111111111;

            output2[0] = OP;
            output2[1] = input2[2];
            
            if(opcode == 3){
                finalOutput[0] = 1;
                finalOutput[1] = output2[0];
                finalOutput[2] = output2[1];
                addToFIFO = true;
                while(!addToFIFO);
            }

            stage3 = false;
            stage0 = true;
        }
    }
    pthread_exit(NULL);
}

void* fifoAdder(void* input){
    while(true){
        if(endAll) break;
        if(addToFIFO){
            if(inputSelector){
                FIFO1.push({1,0});
                inputSelector = false;
            }else{
                if(finalOutput[0]){
                    finalOutput[0] = 0;
                    if(finalOutput[2]){
                        FIFO2.push({finalOutput[1], finalOutput[2]});
                    }else{
                        FIFO1.push({finalOutput[1], finalOutput[2]});
                    }
                }
            }
            addToFIFO = false;
        }
    }
    pthread_exit(NULL);
}

int main(){
    pthread_t threads[10];
    int instructionFetchRetVal = pthread_create(&threads[0], NULL, fetchInstruction, (void *) 0);
    int module1RetVal = pthread_create(&threads[1], NULL, module1, (void *) 0);
    int module2RetVal = pthread_create(&threads[2], NULL, module2, (void *) 0);
    int fifoAdderRetVal = pthread_create(&threads[3], NULL, fifoAdder, (void *) 0);

    cin>>S;
    stage0 = true;
    while(true){
        if(stage0){
            cout<<"> In stage 0\n";
            if(acceptParital){
                cout<<"MANAGER: The string "<<S<<" is accepted partially\n";
                endAll = true;
                break;
            }

            if(accept){
                cout<<"MANAGER: The string is accepted\n";
                endAll = true;
                break;
            }

            if(FIFO1.empty() and FIFO2.empty()){
                cout<<"Manager: Both FIFOs are empty, string rejected\n";
                cout<<"Manager: "<<S<<" is not accepted by the regex\n";
                reject = true;
                endAll = true;
                break;

            }else{
                if(currentFIFO==1){
                    if(FIFO1.empty()){
                        cout<<"MANAGER: FIFO1 is empty, switching to FIFO2\n";
                        currentFIFO = 2;
                        currentCharacter++;
                    }
                }else{
                    if(FIFO2.empty()){
                        cout<<"MANAGER: FIFO2 is empty, switching to FIFO1\n";
                        currentFIFO = 1;
                        currentCharacter++;
                    }
                }

                if(currentFIFO==1){
                    input1[0] = FIFO1.front()[0];
                    input1[1] = FIFO1.front()[1];
                    FIFO1.pop();
                }else{
                    input1[0] = FIFO2.front()[0];
                    input1[1] = FIFO2.front()[1];
                    FIFO2.pop();   
                }

                stage0 = false;
                stage1 = true;

            }

        }
    }

    for(int i=0; i<4; i++){
        int retVal = pthread_join(threads[i], NULL); // waits for the threads to exit

        if(retVal){
            printf("ERROR; return code from pthread_join() is %d\n", retVal);
            exit(-1);
        }
    }
}
