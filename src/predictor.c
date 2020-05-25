//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
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
  gshare_init_predictor();
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
  gshare_train_predictor(pc , outcome);
}

