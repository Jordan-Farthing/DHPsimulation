#include <iostream>
#include "renamer.h"
#include <assert.h>

renamer::renamer(uint64_t n_log_regs, uint64_t n_phys_regs, uint64_t n_branches) {
    assert (n_phys_regs>n_log_regs);
    assert ((1<=n_branches) && (n_branches<=64));
    log_regs=n_log_regs;
    phys_regs=n_phys_regs;
    RMT= new uint64_t[n_log_regs];
    for(uint64_t i = 0; i < n_log_regs; i++){
        RMT[i]=i;
    }
    AMT= new uint64_t[n_log_regs];
    for(uint64_t i = 0; i < n_log_regs; i++){
        AMT[i]=i;
    }
    Free_List.table=new uint64_t[n_phys_regs-n_log_regs];
    for(uint64_t i = 0; i < (n_phys_regs-n_log_regs); i++){
        Free_List.table[i]=n_log_regs+i;
    }
    Free_List.size=n_phys_regs-n_log_regs;
    Free_List.empty=0;
    Free_List.head=0;
    Free_List.tail=0;
    Active_List.table = new uint64_t*[(n_phys_regs-n_log_regs)];
    for(uint64_t i = 0; i < (n_phys_regs-n_log_regs); i++){
        Active_List.table[i]=new uint64_t[19];
    }
    Active_List.size=n_phys_regs-n_log_regs;
    Active_List.full=0;
    Active_List.head=0;
    Active_List.tail=0;
    Physical_Register_File=new uint64_t[n_phys_regs];
    //all registers are ready
    PRF_Ready_Bit_Array=new uint64_t[n_phys_regs];
    for(uint64_t i = 0; i < n_phys_regs; i++){
        PRF_Ready_Bit_Array[i]=1;
    }
    GBM=0;
    total_branches=n_branches;
    Checkpoints = new struct Branch_Checkpoint[n_branches];
    for(uint64_t i = 0; i < n_branches; i++){
        Checkpoints[i].Shadow_Map_Table=new uint64_t[n_log_regs];
        Checkpoints[i].FL_Head_Index=0;
        Checkpoints[i].Checkpoint_GBM=0;
    }
}

renamer::~renamer(){
}


bool renamer::stall_reg(uint64_t bundle_dst){
    uint64_t checker=Free_List.head;
    uint64_t i = 0;
    if (Free_List.empty==0){
        if(checker==Free_List.tail){
            i=1;
            checker=(checker+1)%Free_List.size;
        }
        while (i<bundle_dst){
            if(checker==Free_List.tail){
                return true;
            }
            checker=(checker+1)%Free_List.size;
            i++;
        }
        return false;
    }
    return true;
}

bool renamer::stall_branch(uint64_t bundle_branch){
    uint64_t j =1;
    uint64_t max_mask=(j<<(total_branches-j));
    uint64_t counter=0;
    uint64_t i = 1;
    while (i<=max_mask){
        if((i&GBM)==0){
            counter+=1;
        }
        if(counter>=bundle_branch){
            return false;
        }
        if(i<=max_mask){
            i=i<<1;
        }
        else{
            break;
        }
    }
    return true;
}


uint64_t renamer::get_branch_mask(){
    return GBM;
}

uint64_t renamer::rename_rsrc(uint64_t log_reg){
    return RMT[log_reg];
}

uint64_t renamer::rename_rdst(uint64_t log_reg){
    assert(Free_List.empty==0);
    RMT[log_reg]=Free_List.table[Free_List.head];
    Free_List.head=(Free_List.head+1)%Free_List.size;
    if(Free_List.head==Free_List.tail){
        Free_List.empty=1;
    }
    return RMT[log_reg];
}

uint64_t renamer::checkpoint(){
    uint64_t j =1;
    uint64_t i = 0;
    while (i<total_branches){
        if((j&GBM)==0){
            break;
        }
        j=j<<1;
        i++;
        assert(i!=total_branches);
    }
    GBM|=j;
    Checkpoints[i].Checkpoint_GBM=GBM;
    Checkpoints[i].FL_Head_Index=Free_List.head;
    Shadow_Copy(i);
    return i;
}

bool renamer::stall_dispatch(uint64_t bundle_inst){
    if (Active_List.full==1){
        return true;
    }
    uint64_t test_tail=Active_List.tail;
    int i =0;
    if(Active_List.head==test_tail){
        i=1;
        test_tail=(test_tail+1)%Active_List.size;
    }
    while (i<bundle_inst){
        if (Active_List.head==test_tail){
            return true;
        }
        test_tail=(test_tail+1)%Active_List.size;
        i++;
    }
    return false;
}

//DHP FIX
//add 2 new arguments DHP, DHP_id
uint64_t renamer::dispatch_inst(bool dest_valid,
                                uint64_t log_reg,
                                uint64_t phys_reg,
                                bool load,
                                bool store,
                                bool branch,
                                bool amo,
                                bool csr,
                                uint64_t PC,
                                bool DHP,
                                uint64_t DHP_id){
    assert(Active_List.full==0);
    if (dest_valid){
        Active_List.table[Active_List.tail][1]=1;
        Active_List.table[Active_List.tail][2]=log_reg;
        Active_List.table[Active_List.tail][3]=phys_reg;
    }
    else{
        Active_List.table[Active_List.tail][1]=0;
    }
    Active_List.table[Active_List.tail][4]=0;
    Active_List.table[Active_List.tail][5]=0;
    Active_List.table[Active_List.tail][6]=0;
    Active_List.table[Active_List.tail][7]=0;
    Active_List.table[Active_List.tail][8]=0;
    if (load){
        Active_List.table[Active_List.tail][9]=1;
    }
    else{
        Active_List.table[Active_List.tail][9]=0;
    }
    if (store){
        Active_List.table[Active_List.tail][10]=1;
    }
    else{
        Active_List.table[Active_List.tail][10]=0;
    }
    if (branch){
        Active_List.table[Active_List.tail][11]=1;
    }
    else{
        Active_List.table[Active_List.tail][11]=0;
    }
    if (amo){
        Active_List.table[Active_List.tail][12]=1;
    }
    else{
        Active_List.table[Active_List.tail][12]=0;
    }
    if (csr){
        Active_List.table[Active_List.tail][13]=1;
    }
    else{
        Active_List.table[Active_List.tail][13]=0;
    }
    //DHP FIX
    //set fields in active list for DHP, DHP_id
    if (DHP){
        Active_List.table[Active_List.tail][15]=1;
    }
    else{
        Active_List.table[Active_List.tail][15]=0;
    }
    Active_List.table[Active_List.tail][16]=DHP_id;
    //DHP FIX
    //set deactivated to false or 0 in dispatch
    Active_List.table[Active_List.tail][17]=0;
    Active_List.table[Active_List.tail][14]=PC;


    uint64_t return_tail=Active_List.tail;
    Active_List.tail=(Active_List.tail+1)%Active_List.size;
    if(Active_List.tail==Active_List.head){
        Active_List.full=1;
    }
    return return_tail;
}

bool renamer::is_ready(uint64_t phys_reg){
    return (PRF_Ready_Bit_Array[phys_reg]==1);
}

void renamer::clear_ready(uint64_t phys_reg){
    PRF_Ready_Bit_Array[phys_reg]=0;
}

void renamer::set_ready(uint64_t phys_reg){
    PRF_Ready_Bit_Array[phys_reg]=1;
}

uint64_t renamer::read(uint64_t phys_reg){
    return Physical_Register_File[phys_reg];
}

void renamer::write(uint64_t phys_reg, uint64_t value){
    Physical_Register_File[phys_reg]=value;
}

void renamer::set_complete(uint64_t AL_index){
    Active_List.table[AL_index][4]=1;
}

void renamer::resolve(uint64_t AL_index,
             uint64_t branch_ID,
             bool correct){
    //bitshift 1 by branch ID number before doing bit operation.
    uint64_t bit_mask;
    uint64_t y=1;
    bit_mask=y<<branch_ID;
    uint64_t i=0;
    while (i<total_branches){
        Checkpoints[i].Checkpoint_GBM&=(~bit_mask);
        i++;
    }
    if(correct){
        GBM&=(~bit_mask);
        return;
    }
    else{
        GBM=Checkpoints[branch_ID].Checkpoint_GBM;
        GBM&=(~bit_mask);
        i=0;
        while(i<log_regs){
            RMT[i]=Checkpoints[branch_ID].Shadow_Map_Table[i];
            i++;
        }
        if (Active_List.full==1){
            assert(Active_List.tail==Active_List.head);
            if(Active_List.tail==((AL_index+1)%Active_List.size))
                Active_List.full=1;
            else{
                Active_List.full=0;
            }
        }
        Active_List.tail=(AL_index+1)%Active_List.size;

        if(Checkpoints[branch_ID].FL_Head_Index!=Free_List.head){
            Free_List.empty=0;
        }
        Free_List.head=Checkpoints[branch_ID].FL_Head_Index;
    }
}

bool renamer::precommit(bool &completed,
               bool &exception, bool &load_viol, bool &br_misp, bool &val_misp,
               bool &load, bool &store, bool &branch, bool &amo, bool &csr,
               uint64_t &PC){
    if((Active_List.head==Active_List.tail)&&(Active_List.full==0)){
        return false;
    }
    else{
        completed=(Active_List.table[Active_List.head][4]==1);
        exception=(Active_List.table[Active_List.head][5]==1);
        load_viol=(Active_List.table[Active_List.head][6]==1);
        br_misp=(Active_List.table[Active_List.head][7]==1);
        val_misp=(Active_List.table[Active_List.head][8]==1);
        load=(Active_List.table[Active_List.head][9]==1);
        store=(Active_List.table[Active_List.head][10]==1);
        branch=(Active_List.table[Active_List.head][11]==1);
        amo=(Active_List.table[Active_List.head][12]==1);
        csr=(Active_List.table[Active_List.head][13]==1);
        PC=Active_List.table[Active_List.head][14];
        return true;
    }
}

//DHP FIX
//using flag for predicate commit
bool renamer::commit(){
    assert(!((Active_List.head==Active_List.tail)&&(Active_List.full==0)));
    assert(Active_List.table[Active_List.head][4]==1);
    assert(Active_List.table[Active_List.head][5]==0);
    assert(Active_List.table[Active_List.head][6]==0);
    //DHP FIX
    //if head is not a DHP instr and deactivated, then commit normally.
    if (!(Active_List.table[Active_List.head][15] && Active_List.table[Active_List.head][17])) {
        if (Active_List.table[Active_List.head][1]) {
            //need to copy AMT pReg to Free List Tail since we have dest register pReg to replace it in AMT.
            //assert free list is not full
            Free_List.table[Free_List.tail] = AMT[Active_List.table[Active_List.head][2]];
            Free_List.tail = (Free_List.tail + 1) % Free_List.size;
            Free_List.empty = 0;
            //only thing different is to add dst reg to AMT
            AMT[Active_List.table[Active_List.head][2]] = Active_List.table[Active_List.head][3];
            Active_List.head = (Active_List.head + 1) % Active_List.size;
        } else {
            Active_List.head = (Active_List.head + 1) % Active_List.size;
        }
        Active_List.full = 0;
        return true;
    }
    else{
        if (Active_List.table[Active_List.head][1]) {
            //assert free list is not full
            Free_List.tail = (Free_List.tail + 1) % Free_List.size;
            Free_List.empty = 0;
            //only thing different is to add dst reg to AMT
            Active_List.head = (Active_List.head + 1) % Active_List.size;
        }
        else{
            Active_List.head = (Active_List.head + 1) % Active_List.size;
        }
        Active_List.full = 0;
        return false;
    }
}

void renamer::squash(){
    Active_List.tail=Active_List.head;
    Free_List.head=Free_List.tail;
    Free_List.empty=0;
    Active_List.full=0;
    uint64_t i =0;
    while(i<log_regs){
        RMT[i]=AMT[i];
        i++;
    }
    for(i = 0; i < phys_regs; i++){
        PRF_Ready_Bit_Array[i]=1;
    }
    GBM=0;
}

void renamer::set_exception(uint64_t AL_index){
    Active_List.table[AL_index][5]=1;
}
void renamer::set_load_violation(uint64_t AL_index){
    Active_List.table[AL_index][6]=1;
}
void renamer::set_branch_misprediction(uint64_t AL_index){
    Active_List.table[AL_index][7]=1;
}
void renamer::set_value_misprediction(uint64_t AL_index){
    Active_List.table[AL_index][8]=1;
}
bool renamer::get_exception(uint64_t AL_index){
    return Active_List.table[AL_index][5]==1;
}

void renamer::Shadow_Copy(uint64_t index){
    uint64_t i = 0;
    while (i<log_regs){
        Checkpoints[index].Shadow_Map_Table[i]=RMT[i];
        i++;
    }
}

//DHP FIX
//PC that is deactivated when taken is 1a5b4
//else all instructions are activated
void renamer::deactivate(uint64_t DHP_id){
    uint64_t fake_head=Active_List.head;
    while(fake_head!=Active_List.tail) {
        if(Active_List.table[fake_head][15]==1)
            if(Active_List.table[fake_head][16]==DHP_id){
                    Active_List.table[fake_head][17]=1;
                    return;
                }
        fake_head = (fake_head + 1) % Active_List.size;
    }
}
