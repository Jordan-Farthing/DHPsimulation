# DHPsimulation

Any change that is made to a file will be included with a comment in the file in the following format <br />
"DHP FIX <br />
..." <br />

# List of files that have been changed from working simulator below

FETCH STAGE

payload.h <br />
payload.cc <br />
-add argument flag for inherit prev instruction to map_to_actual depending on DHP region intrs if good or bad <br />
-if inherit prev is true, get prev good instr value and prev db_index to current index and return <br />

btb.h <br />
-add argument Branch_PC to btb lookup <br />
btb.cc <br />
-if pc is hammock pc, then terminate btb <br />

fetchunit.h <br />
-declare enumerate state_machine control variable <br />
-declare instr counter for instructions in each state <br />
-declare hammock info struct ideal table <br />

fetchunit.cc <br />
-initialize state_machine as IDLE <br />
-define hammock ideal table struct <br />
-call btb lookup with hammock PC argument only if state machine is IDLE <br />
-make switch control in fetch transfer bundle function to map_to_actual based on hammock state branch outcome <br />

RENAME STAGE 

payload.h <br />
-Record in fetch1 if instruction is valid predicate instr or not <br />
-Record in fetch1 if instruction is in correct region(true) or dead instruction(false) <br />
payload.cc <br />
-depending on case in transfer_fetch_bundle, set bool for valid predicate or correct_region <br />
