//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

#define MAX(X, Y)((X)>(Y)?(X):(Y))
#define MIN(X, Y)((X)<(Y)?(X):(Y))

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

uint32_t gh = 0;
uint32_t ghMask = 0;
uint32_t lhMask = 0;
uint32_t pcMask = 0;

// BHT predictor for global and local use, each entry is a 2-bit BHT
uint8_t *gBHT;
uint8_t *lBHT;

// local history table, each entry is the history of given branch
uint32_t *lHT;

// tournament prediction multiplexer
uint8_t *predmux;

// helper function
uint8_t pred_2bh(uint8_t);
uint8_t pred(uint8_t*, uint32_t);
void update_hist(uint32_t*, uint32_t, uint8_t);
void update_2bh(uint8_t*, uint8_t);
void init_pc();
void init_global();
void init_local();
void train_global(uint32_t, uint8_t);
void train_local(uint32_t, uint8_t);

// gshare
void init_gshare();
uint32_t gxor(uint32_t);
uint8_t pred_gshare(uint32_t);
void train_gshare(uint32_t, uint8_t);

// tournament
void init_tournament();
uint8_t pred_tournament(uint32_t);
void train_tournament(uint32_t, uint8_t);


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  switch (bpType) {
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tournament();
      break;
    case CUSTOM:
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return pred_gshare(pc);
    case TOURNAMENT:
      return pred_tournament(pc);
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  switch (bpType)
  {
  case GSHARE:
    train_gshare(pc, outcome);
    break;
  case TOURNAMENT:
    train_tournament(pc, outcome);
    break;
  case CUSTOM:
  default:
    break;
  }
}

//------------------------------------//
//          helper Functions          //
//------------------------------------//

// update the state of 2-bit history entry, will increase counter if direction
// is 1, or decrease counter if direction is 0
void
update_2bh(uint8_t *bh, uint8_t direction)
{
  switch (direction)
  {
  case 1:
    *bh = *bh == 3 ? *bh : *bh + 1;
    break;
  case 0:
    *bh = *bh == 0 ? *bh : *bh - 1;
  default:
    break;
  }
}

// append the history to the LSB
void
update_hist(uint32_t *hist, uint32_t mask, uint8_t outcome)
{
  *hist = ((*hist) << 1) | outcome;
  *hist &= mask;
}

// initialize variables associated with pc
void
init_pc()
{
  // initialize pc mask
  for (int i = 0; i < pcIndexBits; ++i) {
    pcMask <<= 1;
    pcMask |= 1;
  }
}

// initialize variables associated with global predictor
void
init_global()
{
  // initialize global history mask
  for (int i = 0; i < ghistoryBits; ++i) {
    ghMask <<= 1;
    ghMask |= 1;
  }

  // initialize global BHT
  gBHT = (uint8_t*)malloc((1<<ghistoryBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(gBHT, 1, (1<<ghistoryBits) * sizeof(uint8_t));

}

// initialize variables associated with local predictor
void
init_local()
{
  // initialize local history mask
  for (int i = 0; i < lhistoryBits; ++i) {
    lhMask <<= 1;
    lhMask |= 1;
  }

  // initialize local BHT
  lBHT = (uint8_t*)malloc((1<<lhistoryBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(lBHT, 1, (1<<lhistoryBits) * sizeof(uint8_t));
}

// make prediction based on a 2-bit BHT entry
uint8_t
pred_2bh(uint8_t bh)
{
  return bh > 1 ? (uint8_t)TAKEN : (uint8_t)NOTTAKEN;
}

// make prediction given index and branch history table
uint8_t
pred(uint8_t *BHT, uint32_t index)
{
  return pred_2bh(*(BHT + index));
}

// train global state given the corresponding index and outcome
//
// 1. update global history register
// 2. update correspond BHT counter
void
train_global(uint32_t idx, uint8_t outcome)
{
  // update global history
  update_2bh(gBHT + idx, outcome == TAKEN);
  update_hist(&gh, ghMask, outcome);
}

// train global state given the corresponding index and outcome
//
// 1. update global history register
// 2. update correspond BHT counter
void
train_local(uint32_t idx, uint8_t outcome)
{
  // update local history
  update_2bh(lBHT + *(lHT + idx), outcome == TAKEN);
  update_hist(lHT + idx, lhMask, outcome);
}

//------------------------------------//
//           Gshare Logics            //
//------------------------------------//

// initiliaze gshare
void
init_gshare()
{
  init_global();
}

// XOR function for gshare indexing
uint32_t
gxor(uint32_t pc)
{
  return (pc & ghMask) ^ gh;
}

// make prediction using gshare
uint8_t
pred_gshare(uint32_t pc)
{
  return pred(gBHT, gxor(pc));
}

// train gshare predictor
void
train_gshare(uint32_t pc, uint8_t outcome)
{
  train_global(gxor(pc), outcome);
}

//------------------------------------//
//         Tournament Logics          //
//------------------------------------//

// initiliaze tournament
void
init_tournament()
{
  init_pc();
  init_local();
  init_global();

  // initialize local history table
  lHT = (uint32_t*)malloc((1<<pcIndexBits) * sizeof(uint32_t));
  // initialize all history to NOT TAKEN
  memset(lHT, 0, (1<<pcIndexBits) * sizeof(uint32_t));

  // initialize local BHT
  predmux = (uint8_t*)malloc((1<<pcIndexBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(predmux, 1, (1<<pcIndexBits) * sizeof(uint8_t));
}

// make prediction using Alpha 21264 tournament
uint8_t
pred_tournament(uint32_t pc)
{
  uint32_t addr = pc & pcMask;

  if (*(predmux+addr) <= 1)
    return pred(lBHT, *(lHT+addr));
  else
    return pred(gBHT, gh);
}

// make prediction using gshare
void
train_tournament(uint32_t pc, uint8_t outcome)
{
  uint32_t addr = pc & pcMask;

  // update global state
  train_global(gh, outcome);

  // update local state
  train_local(addr, outcome);

  // update prediction mux
  uint8_t gpred = pred(lBHT, *(lHT+addr));
  uint8_t lpred = pred(gBHT, gh);

  if (gpred != outcome && lpred == outcome)
    update_2bh(predmux + addr, 0);
  else if (gpred == outcome && lpred != outcome)
    update_2bh(predmux + addr, 1);
}