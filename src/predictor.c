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

//
//TODO: Add your own Branch Predictor data structures here
//
uint32_t gh = 0;
uint32_t ghMask = 0;

// gshare indexed 2-bit BHT
uint8_t *gshareTable;

// helper function
uint8_t pred_gshare(uint32_t);
uint8_t pred_2bh(uint8_t);
uint32_t gxor(uint32_t);
void set_hist(uint32_t*, uint8_t);
void set_2bh(uint8_t*, uint8_t);

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

  // initialize global history mask
  for (int i = 0; i < ghistoryBits; ++i) {
    ghMask <<= 1;
    ghMask |= 1;
  }

  // initialize global history table
  gshareTable = (uint8_t*)malloc((1<<ghistoryBits) * sizeof(uint8_t));
  // initialize all state to weakly not-taken
  memset(gshareTable, 1, (1<<ghistoryBits) * sizeof(uint8_t));
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
      return pred_gshare(pc);
    case TOURNAMENT:
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
  uint32_t idx = gxor(pc);
  set_2bh(gshareTable + idx, outcome);
  set_hist(&gh, outcome);
}

// XOR function for gshare indexing
uint32_t
gxor(uint32_t pc)
{
  return ((pc & ghMask) ^ (gh & ghMask));
}

// make prediction using gshare
uint8_t
pred_gshare(uint32_t pc)
{
  return pred_2bh(*(gshareTable + gxor(pc)));
}

// make prediction given a 2-bit history state
uint8_t
pred_2bh(uint8_t bh)
{
  return bh > 1 ? (uint8_t)TAKEN : (uint8_t)NOTTAKEN;
}

// update the state of 2-bit history entry
void
set_2bh(uint8_t *bh, uint8_t outcome)
{
  switch (outcome)
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

// append the history
void
set_hist(uint32_t *hist, uint8_t outcome)
{
  *hist = ((*hist) << 1) | outcome;
}