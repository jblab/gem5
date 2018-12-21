#include "base/intmath.hh"
#include "base/trace.hh"
#include "cpu/pred/perceptron.hh"
#include "debug/Fetch.hh"
#include <numeric>
#include <string>

PerceptronBP::PerceptronBP(uint32_t size, uint32_t theta)
{
    if (!size) {
      fatal("PerceptronBP must have size > 0");
    }
    this->W.resize(size);
    this->size = size;
    this->theta = theta;

    // fills W with 0's from [begin, end)
    std::fill(this->W.begin(), this->W.end(), 0);
}
/*
int32_t
PerceptronBP::getPrediction(std::vector<int8_t>& X)
{
    // TODO - this function may require X as an input, depending on how the history register is handled
    int32_t innerProdManual = 0;
    int32_t innerProdStd =  std::inner_product(X.begin(), X.end(), this->W.begin(), 0);

    for(int i=0;i < X.size();i++){
      innerProdManual += X[i] * this->W[i];
    }

    assert(innerProdStd == innerProdManual);
    assert(X.size() == this->W.size());
    return innerProdStd;
  
}
*/

int32_t
PerceptronBP::getPrediction(std::vector<int8_t>& X)
{
    std::vector<uint16_t> W_top_indices = this->topIndices(this->W);
    std::vector<int32_t> new_W;
    std::vector<int8_t> new_X;
    for(int i=0; i<W_top_indices.size(); i++){
        new_W.push_back(W[W_top_indices[i]]);
        new_X.push_back(X[W_top_indices[i]]);
    }
    assert(new_X.size() == new_W.size());
    uint32_t innerProdStd =  std::inner_product(new_X.begin(), new_X.end(), new_W.begin(), 0);
    return innerProdStd;
}

std::vector<uint16_t>
topIndices(std::vector<int32_t>& W){
  int tmp = 0;
  uint16_t highest_inx = 0;
  uint16_t next_highest_inx = 1;
  uint16_t arr_size = W.size();
  std::vector<uint16_t> output;
  for(int i=0; i<2; ++i){
    for(int j=i+1; j<arr_size; ++j){
      if(W[i]<W[j]){
        tmp = W[i];
        W[i] = W[j];
        W[j] = tmp;
        if(i==0)
            highest_inx = j;
        if(i==1)
            next_highest_inx = j;
      }
    }
  }
  output.push_back(highest_inx);
  output.push_back(next_highest_inx);
  return output;
  //for(int i=0; i<arr_size; i++)
  //  cout << arr[i] << endl;
}

void
PerceptronBP::reset()
{
    this->W.clear();
    this->W.resize(this->size);

    // fills W with 0's from [begin, end)
    std::fill(this->W.begin(), this->W.end(), 0);
    DPRINTF(Fetch, "Reset PerceptronBP");
}


void 
PerceptronBP::train(int8_t branch_outcome, int32_t perceptron_output, int32_t theta, std::vector<int8_t> &X)
{
    std::string s = "W: ";
    DPRINTF(Fetch, "Perceptron train entered\n");
    if (this->changeToPlusMinusOne(perceptron_output) != branch_outcome || abs(perceptron_output)<theta) {//incorrect perceptron prediction. Upgrade the perceptron predictor
        for(int i=0; i< this->W.size(); i++) {
            // wx+b
            W[i]= W[i]+ branch_outcome * X[i]; //Increase or decrease weight vectors
            if(abs(W[i]) > theta){
              if (W[i] < 0){
                W[i] = theta * -1;
              }
              else{
                W[i] = theta;
              }
            }
            s.append(std::to_string((long long int)W[i]));
            s.append(", ");
        }
    }
    DPRINTF(Fetch, "%s\n", s);
}

inline int8_t
PerceptronBP::changeToPlusMinusOne(int32_t input)
{
  return (input >= 0) ? 1 : -1;
}