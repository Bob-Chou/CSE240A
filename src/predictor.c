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
//            Tournament              //
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
uint8_t *chooser;

void tournament_init_predictor();
uint8_t tournament_make_prediction(uint32_t);
void tournament_train_predictor(uint32_t, uint8_t);

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    case GSHARE:
      break;
    case TOURNAMENT:
      tournament_init_predictor();
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
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
    case TOURNAMENT:
      return tournament_make_prediction(pc);
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
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case GSHARE:
    case TOURNAMENT:
      tournament_train_predictor(pc, outcome);
    case CUSTOM:
    default:
      break;
  }
}


//------------------------------------//
//         Tournament Logics          //
//------------------------------------//

// initiliaze tournament
void
tournament_init_predictor()
{
  // initialize pc mask
  for (int i = 0; i < pcIndexBits; ++i) {
    pcMask <<= 1;
    pcMask |= 1;
  }

  // initialize local history mask
  for (int i = 0; i < lhistoryBits; ++i) {
    lhMask <<= 1;
    lhMask |= 1;
  }
  // initialize local BHT
  lBHT = (uint8_t*)malloc((1<<lhistoryBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(lBHT, 1, (1<<lhistoryBits) * sizeof(uint8_t));

  // initialize global history mask
  for (int i = 0; i < ghistoryBits; ++i) {
    ghMask <<= 1;
    ghMask |= 1;
  }
  // initialize global BHT
  gBHT = (uint8_t*)malloc((1<<ghistoryBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(gBHT, 1, (1<<ghistoryBits) * sizeof(uint8_t));

  // initialize local history table
  lHT = (uint32_t*)malloc((1<<pcIndexBits) * sizeof(uint32_t));
  // initialize all history to NOT TAKEN
  memset(lHT, 0, (1<<pcIndexBits) * sizeof(uint32_t));

  // initialize local BHT
  chooser = (uint8_t*)malloc((1<<pcIndexBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(chooser, 1, (1<<pcIndexBits) * sizeof(uint8_t));
}

// make prediction using Alpha 21264 tournament
uint8_t
tournament_make_prediction(uint32_t pc)
{
  uint32_t addr = pc & pcMask;

  if (*(chooser+addr) <= 1)
    return *(lBHT + *(lHT+addr)) > 1 ? TAKEN : NOTTAKEN;
  else
    return *(gBHT + gh) > 1 ? TAKEN : NOTTAKEN;
}

// make prediction using gshare
void
tournament_train_predictor(uint32_t pc, uint8_t outcome)
{
  uint32_t addr = pc & pcMask;

  // update prediction mux
  uint8_t gpred = *(lBHT + *(lHT+addr)) > 1 ? TAKEN : NOTTAKEN;
  uint8_t lpred = *(gBHT + gh) > 1 ? TAKEN : NOTTAKEN;

  // update global history state
  uint8_t *gBHR = gBHT + gh;
  if (outcome == TAKEN)
    *gBHR = (*gBHR == 3) ? *gBHR : *gBHR + 1;
  else
    *gBHR = (*gBHR == 0) ? *gBHR : *gBHR - 1;
  // update global history
  gh = ((gh << 1) | outcome) & ghMask;

  // update local state
  uint8_t *lBHR = lBHT + *(lHT + addr);
  if (outcome == TAKEN)
    *lBHR = (*lBHR == 3) ? *lBHR : *lBHR + 1;
  else
    *lBHR = (*lBHR == 0) ? *lBHR : *lBHR - 1;
  // update local history
  *(lHT + addr) = ((*(lHT + addr) << 1) | outcome) & lhMask;

  uint8_t *mux = chooser + addr;
  if (gpred != outcome && lpred == outcome)
    *mux = (*mux == 0) ? *mux : *mux - 1;
  else if (gpred == outcome && lpred != outcome)
    *mux = (*mux == 3) ? *mux : *mux + 1;
}