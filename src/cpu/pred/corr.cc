#include <cstdio>

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "base/trace.hh"
#include "cpu/pred/corr.hh"
#include "debug/Fetch.hh"

CorrBP::CorrBP(const CorrBPParams *params ):BPredUnit(params),
InstShiftAmt(params->instShiftAmt)

{
// initialise history table and counter table
 for (uint i=0; i<historyBits;i++)
   {

    historytable[i]=0;
  }

   for (uint i=0; i<historyBits;i++)
   {

    countertable[i]=0;
  }

  local_pred=0;
  global_pred=0;
  fl.openlog();
  sprintf (msg , " constructor called ");
  fl.log(msg);
}

// reset function

void CorrBP::reset() {


   for (int i=0; i<historyBits;i++)
   {

    historytable[i]=0;
  }


   for (int i=0; i<historyBits;i++)
   {

    countertable[i]=0;
  }

  local_pred=0;
  global_pred=0;

}

inline
unsigned
CorrBP::calcLocHistIdx(Addr &branch_addr)
{
    // Get low order bits after removing instruction offset.
    return (branch_addr >> InstShiftAmt) & (historyBits-1);
}

inline
void
CorrBP::updateGlobalHistTaken(unsigned branch_lower_order)
{
   historytable[branch_lower_order]=(historytable[branch_lower_order] <<1) | 1;
   historytable[branch_lower_order]=(historytable[branch_lower_order] & (historyBits-1));


    DPRINTF(Fetch, "global branch taken\n");

}



inline
void
CorrBP::updateGlobalHistNotTaken(unsigned branch_lower_order)
{
   historytable[branch_lower_order]=(historytable[branch_lower_order] <<1) ;
   historytable[branch_lower_order]=(historytable[branch_lower_order] & (historyBits-1));



    DPRINTF(Fetch, "global branch not taken\n");

}



// lookup

bool CorrBP::lookup(ThreadID tid, Addr branch_addr, void * &bp_history){

bool taken=0;

branch_lower_order= calcLocHistIdx(branch_addr);

index=historytable[branch_lower_order];

count=countertable[index];

taken= (count >> 1);   // get the msb of the sat count


sprintf (msg , " addr= 0x%lu", branch_addr);
 fl.log(msg);

// Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->global_pred = taken;
    bp_history = (void *)history;
DPRINTF(Fetch, "branch lower order=%i\n", branch_lower_order);
DPRINTF(Fetch, "lookup complete\n");


return taken;
}



// update
void
CorrBP::update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed){


   branch_lower_order = calcLocHistIdx(branch_addr);

if (bp_history) {
        BPHistory *history = static_cast<BPHistory *>(bp_history);
// update local history
 if (taken) {
        DPRINTF(Fetch, "Branch updated as taken.\n");

        index=historytable[branch_lower_order];
        count=countertable[index];
        if (count<3)
        count++;
        countertable[index]=count;

        updateGlobalHistTaken(branch_lower_order);
        DPRINTF(Fetch, "global update.\n");



    } else {
        DPRINTF(Fetch, "Branch updated as not taken.\n");

       index=historytable[branch_lower_order];
        count=countertable[index];
        if ((count>0) & (count<=3))
        count--;
        countertable[index]=count;

        updateGlobalHistNotTaken(branch_lower_order);
        DPRINTF(Fetch, "global update.\n");
   }
  //pass it back to be recorded
    history->global_pred = taken;

    bp_history = static_cast<void *>(history);

}

sprintf (msg , " addr= 0x%lu", branch_addr);
 fl.log(msg);

DPRINTF(Fetch, "update complete\n");


}


void
CorrBP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
// Place holder for a function that is called to update predictor history when
// a BTB entry is invalid or not found.

branch_lower_order = calcLocHistIdx(branch_addr);

// get the stored predictor history
  BPHistory *history = static_cast<BPHistory *>(bp_history);

// update History to not taken

   index=historytable[branch_lower_order];
        count=countertable[index];
        if ((count>0) & (count<=3))
        count--;
        countertable[index]=count;

        updateGlobalHistNotTaken(branch_lower_order);

//pass it back to be recorded
    history->global_pred = 0;

    bp_history = static_cast<void *>(history);


DPRINTF(Fetch, "btbUpdate\n");

sprintf (msg , " addr= 0x%lu", branch_addr);
fl.log(msg);

}


void
CorrBP::uncondBranch(ThreadID tid,Addr branch_addr,void *&bp_history)
{

// Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->global_pred= 1;
    bp_history = static_cast<void *>(history);

    DPRINTF(Fetch, "uncondBranch\n");
    branch_lower_order = calcLocHistIdx(branch_addr);

        index=historytable[branch_lower_order];
        count=countertable[index];
        if (count<3)
        count++;
        countertable[index]=count;

        updateGlobalHistTaken(branch_lower_order);


  sprintf (msg , " uncond branch called ");
  fl.log(msg);

}



void
CorrBP::squash(ThreadID tid, void *bp_history){

DPRINTF(Fetch, "squash\n");
  //  updateGlobalHistNotTaken();

  BPHistory *history = static_cast<BPHistory *>(bp_history);

       // Delete this BPHistory now that we're done with it.
    delete history;

  sprintf(msg,"squash called ");
  fl.log(msg);

}

void
CorrBP::squash2(Addr &branch_addr){


     branch_lower_order = calcLocHistIdx(branch_addr);
     // decrement counters since branch was mispredicted
       index=historytable[branch_lower_order];
        count=countertable[index];
        if ((count>0) & (count<=3))
        count--;
        countertable[index]=count;

        updateGlobalHistNotTaken(branch_lower_order);


}

CorrBP*
CorrBPParams::create()
{
    return new CorrBP(this);
}
/*
CorrBP:: ~ CorrBP (void){


sprintf (msg , " destructor");

fl.log(msg);
fl.closelog();


}
*/

#ifdef DEBUG
int
CorrBP::BPHistory::newCount = 0;
#endif
