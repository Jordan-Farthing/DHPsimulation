# DHPsimulation

Any change that is made to a file will be included with a comment in the file in the following format <br />
"DHP FIX <br />
..." <br />

# List of files that have been changed from working simulator below

FETCH STAGE

payload.h <br />
-add a mux flax for CMOV and changing branch instr for decode stage and alu stage.
payload.cc <br />
-add argument flag for inherit prev instruction to map_to_actual depending on DHP region intrs if good or bad <br />
-if inherit prev is true, get prev good instr value and prev db_index to current index and return <br />

btb.h <br />
-add argument Branch_PC to btb lookup <br />
-add argument nomral to do normal btb lookup or sequential variable set
btb.cc <br />
-if pc is hammock pc or we dont want normal call, sequentially set variables and return <br />
-use normal variable for not being in IDLE state.

fetchunit.h <br />
-declare enumerate state_machine control variable <br />
-declare instr counter for instructions in each state <br />
-declare hammock info struct ideal table <br />
-in fetch2 if a mispredict, set state_machine to IDLE again because mispredict is a branch before predicate branch.

fetchunit.cc <br />
-initialize state_machine as IDLE <br />
-define hammock ideal table struct <br />
-call btb lookup with hammock PC argument only if state machine is IDLE <br />
-make switch control in fetch transfer bundle function to map_to_actual based on hammock state branch outcome <br />

decode.cc <br />
-use OP_JAL for branch instr  <br />
-use OP_OP_IMM_32 for CMOVE instr  <br />
-set all source registers to true and destination registers to true and values if mux instr, otherwise do normal  <br />

alu.cc <br />
-change OP_JAL execution for mux == 1 for a hammock branch instruction that wants a destination reg value to determine <br />
-which reg is true for CMOVE <br />
-change OP_OP_IMM_32 for CMOVE execution in ALU <br />
-if predicate value is not zero, bring A reg value into C, else bring B reg value into C <br />

riscv-base decode.h <br />
-provide set functions to inject instructions in Fetch State Machine <br />

RENAME STAGE 

payload.h <br />
-Record in fetch1 if instruction is valid predicate instr or not <br />
-Record in fetch1 if instruction is in correct region(true) or dead instruction(false) <br />
payload.cc <br />
-depending on case in transfer_fetch_bundle, set bool for valid predicate or correct_region <br />

retire.cc
-if a valid predicate at head of Active list and not in correct region, then dont commit to AMT just push tail of free list <br />
-otherwise do normal action <br />

renamer.h <br />
renamer.cc <br />
-using flag argument in commit function to commit differently for predicate false region. <br />
-change commit function based on correct being true or not

