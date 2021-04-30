# DHPsimulation
to run this 
721sim --disambig=0,0,0 --perf=1,1,1,1 -t --fq=64 --cp=32 --al=256 --lsq=128 --iq=64 --iqnp=4 --fw=4 --dw=4 --iw=8 --rw=4 pk if.riscv

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

fetchunit.cc <br />
-initialize state_machine as IDLE <br />
-define hammock ideal table struct <br />
-call btb lookup with hammock PC argument only if state machine is IDLE <br />
-make switch control in fetch transfer bundle function to map_to_actual based on hammock state branch outcome <br />
-in fetch2 if a mispredict, set state_machine to IDLE again because mispredict is a branch before predicate branch. <br />

decode.cc <br />
-use OP_OP_32 for branch instr  <br />
-use OP_OP_IMM_32 for CMOVE instr  <br />
-set all source registers to true and destination registers to true and values if mux instr, otherwise do normal  <br />

alu.cc <br />
-change OP_OP_32 execution for mux == 1 for a hammock branch instruction that wants a destination reg value to determine <br />
-which reg is true for CMOVE <br />
-change OP_OP_IMM_32 for CMOVE execution in ALU <br />
-if predicate value rs3(D) is nonzero, bring rs1(A) reg value into C, else bring rs2(B) reg value into C <br />

riscv-base decode.h <br />
-provide set functions to inject instructions in Fetch State Machine <br />

RENAME STAGE <br />

pipeline.cc
-added 1 log reg to renamer

/////////////////////// <br />
(Doesnt Matter Anymore) <br />
payload.h <br />
-Record in fetch1 if instruction is valid predicate instr or not <br />
-Record in fetch1 if instruction is in correct region(true) or dead instruction(false) <br />
-depending on case in transfer_fetch_bundle, set bool for valid predicate or correct_region <br />
/////////////////////// <br />

------------ <br />
payload.h <br />
payload.cc <br />
-bool DHP and DHP_id recorded in payload. Instruction is a DHP CD instruction or not and the pReg ID that guards it <br />

fetchunit.cc
-dont set DHP to true for branch or CMOVE, default set to false and set to true if in CD region (not IDLE or CMOV) <br />

rename.cc <br />
-use rename_rsrc to ask what pREG of r64 is and give it to the payload buffer <br />

dispatch.cc
-2 new arguments, pass in bool DHP and uint64_t DHP id to REN->dispatch_inst <br />
-instruction will only set DHP to true if its in DHP CD region <br />

renamer.h
renamer.cc <br />
-3 new fields to active list, DHP (1 or 0), DHP_id value, deactivated (1 or 0) <br />
-add 2 new fields to dispatch instruction DHP, DHP_id in renamer.h renamer.cc <br />
-set fields for active list DHP and DHP_id inside dispatch function set deactivate field to false initially <br />
-add deactivate function for writeback when instruction is DHP and taken,  <br />
deactivate instruction that uses its DHP_id and the PC to deactivate. <br />
-commit function looks for DHP and deactivated to be true, and commits differently.

writeback.cc
- if instruction that executed is branch predicate, and its destination value is true; <br />
- call deactivate with its destination tag to deactivate instructions in active list that depend on this tag and <br />
- are a valid DHP instruction <br />



