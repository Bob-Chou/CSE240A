//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
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

// perceptron predictor parameter table
uint8_t lastpred;
int score;
int* params;
void perceptron_init_predictor();
uint8_t perceptron_make_prediction(uint32_t);
void perceptron_train_predictor(uint32_t, uint8_t);
//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint32_t gshare_ghistory;
uint32_t gshare_ghistory_limit;
struct Node {
  uint32_t index;
  uint8_t  state;
  struct Node *next;
};
struct Node dummy;
struct Node* dummyPointer = &dummy;
struct Node* tail = &dummy;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

void
gshare_init_predictor() {
  gshare_ghistory = 0;
  gshare_ghistory_limit = pow(2 , ghistoryBits);
}

void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    case GSHARE:
      gshare_init_predictor();
      break;
    case TOURNAMENT:
      tournament_init_predictor();
      break;
    case CUSTOM:
      perceptron_init_predictor();
      break;
    default:
      break;
  }
}



// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t gshare_get_prediction(uint32_t index) {
  struct Node* walk = dummyPointer->next;
  u_int8_t ret = 0;
  while (walk != NULL) {
    // printf("    walk->index = %d , walk->state = %d \n" , walk->index , walk->state);
    if (walk->index == index) {
      // printf("    find record %d\n" , walk->state);
      return walk->state;}
    walk = walk->next;
  }
  // printf("    No record \n");
  return ret;
}

uint8_t
gshare_make_prediction(uint32_t pc) {
  // gshare_ghistory is $ghistoryBits$ bits
  uint32_t index = (pc%gshare_ghistory_limit) ^ gshare_ghistory;
  // printf("****function make prediction pc = %d , index = %d \n" , pc , index);
  uint8_t predict = gshare_get_prediction(index);
  // printf("  return predict = %d \n" , predict);
  if (predict >= 2) predict = 1;
  else predict = 0;
  return predict;
  // return NOTTAKEN;
}

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
      return gshare_make_prediction(pc);
    case TOURNAMENT:
      return tournament_make_prediction(pc);
    case CUSTOM:
      return perceptron_make_prediction(pc);
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
void gshare_train_predictor(uint32_t pc , uint8_t outcome) {
  uint32_t index = (pc % gshare_ghistory_limit) ^ gshare_ghistory;
  // printf ("****function train predictor with pc = %u , outcome = %u , gshare = %u , index = %u\n" , pc , outcome , gshare_ghistory ,  index);
  struct Node* walk = dummyPointer->next;

  uint8_t inside = 0;
  while (walk != NULL) {
    // printf("    walk->index = %d , walk->state = %d \n" , walk->index , walk->state);
    // update table
    if (walk->index == index) {
      inside = 1;
      uint8_t res = walk->state;
      // printf ("       update walk->state %u outcome = %u " , res , outcome);
      if (outcome == 1) {
        if (res != 3) res += 1;
      } else {
        if (res != 0) res -= 1;
      }
      // printf (" %d \n" , res);
      walk->state = res;
      break;
    }
    walk = walk->next;
  }

  if (inside == 0) {
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    new_node->index = index;
    new_node->state = 1 - (outcome == 0 ? 1 : -1);
    new_node->next = NULL;
    tail->next = new_node;
    tail = new_node;
    // printf("  new node ~~ with index = %d , state = %d \n" , new_node->index , new_node->state);
  }

  gshare_ghistory = gshare_ghistory << 1;
  gshare_ghistory += outcome == 1 ? 1 : 0;
  gshare_ghistory %= gshare_ghistory_limit;
}

void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case GSHARE:
      gshare_train_predictor(pc , outcome);
      break;
    case TOURNAMENT:
      tournament_train_predictor(pc, outcome);
      break;
    case CUSTOM:
      perceptron_train_predictor(pc, outcome);
      break;
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
  chooser = (uint8_t*)malloc((1<<ghistoryBits) * sizeof(uint8_t));
  // initialize all BHT to weakly not-taken
  memset(chooser, 1, (1<<ghistoryBits) * sizeof(uint8_t));
}

// make prediction using Alpha 21264 tournament
uint8_t
tournament_make_prediction(uint32_t pc)
{
  uint32_t addr = pc & pcMask;

  if (*(chooser+gh) > 1)
    return *(lBHT + *(lHT+addr)) > 1 ? TAKEN : NOTTAKEN;
  else
    return *(gBHT + gh) > 1 ? TAKEN : NOTTAKEN;
}

// make prediction using tournament
void
tournament_train_predictor(uint32_t pc, uint8_t outcome)
{
  uint32_t addr = pc & pcMask;

  // update prediction mux
  uint8_t lpred = *(lBHT + *(lHT+addr)) > 1 ? TAKEN : NOTTAKEN;
  uint8_t gpred = *(gBHT + gh) > 1 ? TAKEN : NOTTAKEN;

  uint8_t *mux = chooser + gh;
  if (gpred != outcome && lpred == outcome)
    *mux = (*mux == 3) ? *mux : *mux + 1;
  else if (gpred == outcome && lpred != outcome)
    *mux = (*mux == 0) ? *mux : *mux - 1;

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
}

//------------------------------------//
//         Perceptron Logics          //
//------------------------------------//
// initiliaze perceptron
void
perceptron_init_predictor()
{
  pcIndexBits = 10;
  ghistoryBits = 9;
  // initialize pc mask
  for (int i = 0; i < pcIndexBits; ++i) {
    pcMask <<= 1;
    pcMask |= 1;
  }

  // initialize global history mask
  for (int i = 0; i < ghistoryBits; ++i) {
    ghMask <<= 1;
    ghMask |= 1;
  }

  // initialize parameter table
  params = (int*)malloc((1<<pcIndexBits) * (ghistoryBits + 1) * sizeof(int));
  memset(params, 0, (1<<pcIndexBits) * (ghistoryBits + 1) * sizeof(int));
}

// make prediction using perceptron predictor
uint8_t
perceptron_make_prediction(uint32_t pc)
{
  uint32_t addr = pc & pcMask;
  int *param = params + ((addr ^ gh) & pcMask) * (ghistoryBits + 1);

  score = param[0];
  uint32_t ghist = gh;
  for (int i = 0; i < ghistoryBits; ++i) {
    int h = (ghist & 1) == 0 ? -1 : 1;
    score += h * param[ghistoryBits - i];
    ghist >>= 1;
  }

  lastpred = score < 0 ? NOTTAKEN : TAKEN;
  return lastpred;
}

// make prediction using  perceptron predictor
void
perceptron_train_predictor(uint32_t pc, uint8_t outcome)
{
  uint32_t addr = pc & pcMask;
  int *param = params + ((addr ^ gh) & pcMask) * (ghistoryBits + 1);

  uint32_t ghist = gh;
  int weight_min = -(1<<5) - 1;
  int weight_max = (1<<5) - 1;
  if (lastpred != outcome || (score > weight_min && score < weight_max)) {
    int grad = outcome == TAKEN ? 1 : -1;
    param[0] += grad;
    for (int i = 0; i < ghistoryBits; ++i) {
      int h = (ghist & 1) == 0 ? -1 : 1;
      param[ghistoryBits - i] += h * grad;
      ghist >>= 1;
    }
  }

  // update global history
  gh = ((gh << 1) | outcome) & ghMask;
}

