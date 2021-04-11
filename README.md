# DHPsimulation

Any change that is made to a file will be included with a comment in the file in the following format <br />
"DHP FIX <br />
..." <br />

# List of files that have been changed from working simulator below

FETCH STAGE

payload.h
payload.cc
-add argument flag for inherit prev instruction to map_to_actual depending on DHP region intrs if good or bad
-if inherit prev is true, get prev good instr value and prev db_index to current index and return

btb.h
-add argument Branch_PC to btb lookup
btb.cc
-if pc is hammock pc, then terminate btb

fetchunit.h
-declare enumerate state_machine control variable
-declare instr counter for instructions in each state
-declare hammock info struct ideal table

fetchunit.cc
-initialize state_machine as IDLE
-define hammock ideal table struct
-call btb lookup with hammock PC argument only if state machine is IDLE
-make switch control in fetch transfer bundle function to map_to_actual based on hammock state branch outcome

RENAME STAGE

payload.h
-Record in fetch1 if instruction is valid predicate instr or not
-Record in fetch1 if instruction is in correct region(true) or dead instruction(false)
payload.cc
-depending on case in transfer_fetch_bundle, set bool for valid predicate or correct_region