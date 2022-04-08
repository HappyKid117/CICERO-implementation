#include<bits/stdc++.h>
#include<pthread.h>
#include<unistd.h>
using namespace std;

// Caveats
/*
    increased FIFOS, char ID is vague
    selector line for final mux not specified
    Need elaboration for instruction selection logic
    When does the manager check for FIFOS being empty (an instruction may still be in transit)
*/

/*
    000 - 0 - Match Any
    001 - 1 - Match (OP)
    010 - 2 - NoMatch (OP)
    011 - 3 - Split (OP)
    100 - 4 - JMP (OP)
    101 - 5 - Accept
    110 - 6 - Accept Partial
*/

bool stage0 = false; // lock for stage0 - Manager stage
bool stage1 = false; // lock for stage1 - Instruction fetch stage
bool stage2 = false; // lock for stage2 - All instructions stage
bool stage3 = false; // lock for stage3 - Split instruction stage

bool acceptParital = false; // flag set when string accepted paritally
bool accept = false; // flag set when string accepted completely
bool reject = false; // flag set when string rejected
bool endAll = false; // flag set to end all threads
bool splitCheck = false; // flag set to notify there has been split instruction to the manager

bool inputSelector = true; // selector wire for the first mux
bool fifoSelector = true; // selector wire for the second mux
bool addToFIFO = true; // Maanger flag for adding to FIFO
int currentFIFO = 1; // Manager's data for keeping track of current FIFO

bool s0_valid = false; // valid bit indicating validity of the information
bool s1_valid = false; // valid bit indicating validity of the information
bool s2_valid = false; // valid bit indicating validity of the information
bool s3_valid = false; // valid bit indicating validity of the information

queue<array<int,2>> FIFO1; // Manager's FIFO 1
queue<array<int,2>> FIFO2; // Manager's FIFO 2


array<int,3> s0_finalOutput; // Validity, New PC, FIFO
array<int,2> s0_input1; // PC, FIFO

array<int,2> s1_input1; // PC, FIFO
array<int,3> s1_input2; // Instruction, PC, FIFO

array<int,2> s2_input1; // PC, FIFO
array<int,3> s2_input2; // Instruction, PC, FIFO
array<int,3> s2_output1; // New Pc, FIFO

array<int,3> s3_input2; // Instruction, PC, FIFO
array<int,3> s3_output1; // New Pc, FIFO
array<int,3> s3_output2; // New Pc, FIFO // used in stage 3
array<int,3> s3_finalOutput; // Validity, New PC, FIFO

int currentCharacter = 0; // Manager keeping track of current character

string S; // String stored in memory after user input

void* pipelineElements(void* input){ // Controls the flow of the pipeline

    int cycles = 0;
    bool half_cycle = true  ;
    if(half_cycle) cout<<"> Clock Cycle : C"<<cycles<<" | Current character = "<<currentCharacter<<endl; fflush(0);
    usleep(800000); // (in microseconds) can be any large number, it is only for us to feel the essence of pipeline in output, all instruction are very fast and do not need microseconds to execute
    while(!endAll){
        if(half_cycle) cycles++; // increase 1 cycle for every 2 half cycles
        if(half_cycle) cout<<"> Clock Cycle : C"<<cycles<<" | Current character = "<<currentCharacter<<endl;
        stage0 = false; // pause execution of stage 0
        stage1 = false; // pause execution of stage 1
        stage2 = false; // pause execution of stage 2
        stage3 = false; // pause execution of stage 2

        s3_input2 = s2_input2; // transfer contents of input2 wire from stage 2 to stage 3
        s3_output1 = s2_output1; // transfer contents of output1 wire from stage 2 to stage 3
        
        s2_input1 = s1_input1; // transfer contents of input1 wire from stage 1 to stage 2
        s2_input2 = s1_input2; // transfer contents of input2 wire from stage 1 to stage 2
        
        s1_input1 = s0_input1; // transfer contents of input1 wire from stage0 to stage1

        s0_finalOutput = s3_finalOutput; // transfer contents of finalOutput wire from stage3 to stage0

        s3_valid = s2_valid; // transfer validity of information from stage 2 to stage 3
        s2_valid = s1_valid; // transfer validity of information from stage 1 to stage 2
        s1_valid = s0_valid; // transfer validity of information from stage 0 to stage 1
        
        stage0 = true; // resume stage 0
        usleep(400000); // give the manager some time
        stage1 = true; // resume stage 1
        stage2 = true; // resume stage 2
        stage3 = true; // resume stage 3
        half_cycle = !half_cycle; // switch half cycle state
        usleep(40000); // half cycle time period
    }
    pthread_exit(NULL);
}

void* fetchInstruction(void* input){ // stage 0
    array<int,20> instructionMemory;

    instructionMemory[1] = 0b0010000000000000; // match a
    instructionMemory[2] = 0b0010000000000001; // match b
    instructionMemory[3] = 0b0110000000000111; // split 7
    instructionMemory[4] = 0b0010000000000000; // match a
    instructionMemory[5] = 0b0010000000000001; // match b
    instructionMemory[6] = 0b1000000000001010; // jump 10
    instructionMemory[7] = 0b0010000000000001; // match b
    instructionMemory[8] = 0b0010000000000001; // match b
    instructionMemory[9] = 0b1000000000001010; // jump 10
    instructionMemory[10] = 0b1100000000000000; // accept_partial

    while(true){
        if(endAll) break;
        if(stage1){
            s1_input2[0] = instructionMemory[s1_input1[0]]; // writing instruction into the wire
            s1_input2[1] = s1_input1[0]; // passing on PC
            s1_input2[2] = s1_input1[1]; // passing on char id
        }
    }
    // cout<<"> Fetch Instruction Broken\n";
    pthread_exit(NULL);
}

void* module1(void *input){
    while(true){
        if(endAll) break;
        if(stage2){
            int instruction = s2_input2[0]; // instruction
            int opcode = instruction >> 13; // getting opcode
            int OP     = instruction & 0b0001111111111111; // operand
            bool valid = false; // initial valid bit set to false, will turn true if condition satisfies

            switch (opcode){ // decoding opcode
            case 0: // Match Any
                s2_output1[1] = s2_input2[1]+1; // incrementing PC
                s2_output1[2] = (s2_input2[2]+1)%2; // modular arithmetic : 0+1 = 1, 1+1 = 0
                valid = true; // setting information as valid
                break;
            
            case 1: // Match (OP)
                if((S[currentCharacter]-'a') == OP){
                    valid = true;
                    s2_output1[1] = s2_input1[0]+1;
                    s2_output1[2] = (s2_input1[1]+1)%2;
                }
                break;
            
            case 2: // NoMatch
                if(S[currentCharacter] != OP){
                    s2_output1[1] = s2_input1[0]+1;
                    s2_output1[2] = (s2_input1[1]+1)%2;
                    valid = true;
                }
                break;
            
            case 3: // SPLIT
                s2_output1[1] = s2_input1[0]+1;
                s2_output1[2] = s2_input1[1];
                valid = true;
                break;

            case 4: // JUMP
                s2_output1[1] = OP;
                s2_output1[2] = s2_input1[1];
                valid = true;
                break;

            case 6: // Accept Partial
                valid = true;
                acceptParital = true;
                break;

            default:
                // cout<<"Error: No such instruction "<<opcode<<endl;
                break;
            }

            s2_output1[0] = valid and s2_valid; // the information is valid iff it is stage2 turn to be valid and the condition is satisfied
        }
    }
    
    pthread_exit(NULL);
}

void* module2(void* input){
    while(true){
        if(endAll) break;
        if(stage3){
            
            s3_finalOutput[0] = s3_output1[0]; // passing on stage2 output to final output
            s3_finalOutput[1] = s3_output1[1];
            s3_finalOutput[2] = s3_output1[2];
            
            // cout<<"> In stage 3\n";
            int instruction = s3_input2[0];
            int opcode = instruction >> 13;
            int OP     = instruction & 0b0001111111111111;

            s3_output2[1] = OP;
            s3_output2[2] = s3_input2[2];
            
            if(opcode == 3){ // if operation was split, we need to pass more information to the manager in the same stage, by notifying it through splitcheck
                s3_output2[0] = s3_valid;
                splitCheck = s3_valid;
            }
        }
    }
    pthread_exit(NULL);
}

int main(){
    pthread_t threads[10];
    
    cin>>S;
    
    int instructionFetchRetVal = pthread_create(&threads[0], NULL, fetchInstruction, (void *) 0); // thread for stage 1
    int module1RetVal = pthread_create(&threads[1], NULL, module1, (void *) 0); // thread for stage 2
    int module2RetVal = pthread_create(&threads[2], NULL, module2, (void *) 0); // thread for stage 3

    stage0 = true; // start stage 0
    stage1 = true; // start stage 1
    stage2 = true; // start stage 2
    stage3 = true; // start stage 3

    int count; // this is to keep track of how many cycles the FIFOS have been empty
    while(true){
        if(stage0){

            if(inputSelector){ // first input is given by the user pc = 1, current character index = 0
                FIFO1.push({1,0});
                inputSelector = false;
                cout<<"> Added PC = 1 to queue"<<endl; fflush(0);
                int pipelineElementsRetVal = pthread_create(&threads[3], NULL, pipelineElements, (void *) 0); // created pipeline control thread
            }else{
                if(s0_finalOutput[0]){ // check if information recieved from stage 3 is valid (not all of it recieved every cycle is valid)
                    s0_finalOutput[0] = 0; // set it to not valid because it has been read
                    if(s0_finalOutput[2]){ // check character ID and accordingly choose the FIFO
                        FIFO2.push({s0_finalOutput[1], s0_finalOutput[2]});
                    }else{
                        FIFO1.push({s0_finalOutput[1], s0_finalOutput[2]});
                    }
                    printf("> Added PC = %d to queue\n", s0_finalOutput[1]); fflush(0);
                }else{
                    cout<<"> Not added to queue"<<endl;
                }

                if(splitCheck){ // check if split operation has taken place, if so, get the information from stage 3
                    splitCheck = false;
                    s0_finalOutput[0] = s3_output2[0];
                    s0_finalOutput[1] = s3_output2[1];
                    s0_finalOutput[2] = s3_output2[2];
                    
                    s3_output2[0] = 0;
                    s0_finalOutput[0] = 0;
                    if(s0_finalOutput[2]){ // check character ID and accordingly choose the FIFO
                        FIFO2.push({s0_finalOutput[1], s0_finalOutput[2]});
                    }else{
                        FIFO1.push({s0_finalOutput[1], s0_finalOutput[2]});
                    }
                    printf("> Added PC = %d to queue through split\n", s0_finalOutput[1]);
                }
            }

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

            if((FIFO1.empty() and FIFO2.empty())){
                count--; // reduce count for every cycle it is empty. if it the FIFOS are empty for a while, then it means the string should be rejected
                if(count==0){
                    cout<<"Manager: Both FIFOs have been empty for a while, string rejected\n";
                    cout<<"Manager: "<<S<<" is not accepted by the regex\n";
                    reject = true;
                    endAll = true;
                    break;
                }
                s0_valid = false; // set s0_valid to false so as to propogate this throughout the pipeline
            }else{
                count = 10; // reset count because FIFOs were not empty
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
                    s0_input1[0] = FIFO1.front()[0];
                    s0_input1[1] = FIFO1.front()[1];
                    FIFO1.pop();
                }else{
                    s0_input1[0] = FIFO2.front()[0];
                    s0_input1[1] = FIFO2.front()[1];
                    FIFO2.pop();   
                }
                s0_valid = true; // set s0_valid to true becasue valid information has been passed into the pipeline
            }
            stage0 = false; // pause the manager till new input from stage3 arrives
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
