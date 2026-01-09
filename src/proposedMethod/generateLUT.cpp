#include "common.h"
#include "generateLUT.h"
#include "utils.h"

#include <cmath>
#include <deque>
#include <iostream>

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_oneInput_propsed(int bit_num, double Lutxmax, double Lutxmin,
                             CryptoContext<DCRTPoly> &context) {
  const int slot_count = context->GetEncodingParams()->GetBatchSize();
  vector<double> initInput, initOutput;
  // step1: create input table
  int lutSize = pow(2, bit_num);
  // need lutSize-2 data points in input table
  // interval ensuring lutSize samples span [Lutxmin, Lutxmax] with both
  // endpoints included
  double interval = (Lutxmax - Lutxmin) / (lutSize - 3);
  cout << "interval:" << interval << endl;
  for (double i = Lutxmin; i < Lutxmax; i += interval) {
    if (int(initInput.size()) == lutSize - 3)
      break;
    initInput.push_back(i);
  }
  initInput.push_back(Lutxmax);
  cout << "initInput: " << initInput << endl;
  // step2: create output table
  for (double i = 0; i < initInput.size(); i++) {
    double temps_AM = initInput[i]; // precision
    initOutput.push_back(temps_AM);
  }
  cout << "initOutput:" << initOutput << endl;

  // step3: mid point
  for (double j = 0; j < initInput.size() - 1; j++) {
    initInput[j] = (initInput[j] + initInput[j + 1]) / 2;
    initInput[j] = (initInput[j] - Lutxmin) / (Lutxmax - Lutxmin);
  }
  initInput[initInput.size() - 1] =
      (initInput[initInput.size() - 1] - Lutxmin) / (Lutxmax - Lutxmin);
  // step4: append value
  initInput.push_back(1);
  initInput.push_back(0);
  initOutput.push_back(initOutput[initOutput.size() - 1]);
  initOutput.push_back(0);
  cout << "Precomputed initInput: " << initInput << endl;
  cout << "Precomputed initOutput: " << initOutput << endl;

  // step5: consturct for partial sum
  int k = slot_count / initInput.size();

  vector<vector<double>> lut_x(1, vector<double>(slot_count, 0.0)),
      lut_y(1, vector<double>(slot_count, 0.0));
  for (size_t i = 0; i < initInput.size(); i++) {
    lut_x[0][k * i] = initInput[i];
    lut_y[0][k * i] = initOutput[i];
  }
  return {lut_x, lut_y};
}

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_twoInput(int bit_num, double Lutxmax, double Lutxmin,
                     CryptoContext<DCRTPoly> &context) {
  int inputNum = 2;
  const int slot_count = context->GetEncodingParams()->GetBatchSize();
  vector<double> initInput;
  // step1: create input table
  int lutSize = pow(2, bit_num);
  // need lutSize-2 data points in input table
  // interval ensuring lutSize samples span [Lutxmin, Lutxmax] with both
  // endpoints included
  double interval = (Lutxmax - Lutxmin) / (lutSize - 3);
  cout << "interval:" << interval << endl;
  for (double i = Lutxmin; i < Lutxmax; i += interval) {
    if (int(initInput.size()) == lutSize - 3)
      break;
    initInput.push_back(i);
  }
  initInput.push_back(Lutxmax);
  cout << "initInput: " << initInput << endl;

  // Generate output
  // step2: create output table
  vector<vector<double>> initOutput(lutSize, vector<double>(lutSize, 0.0));
  for (double i = 0; i < initInput.size(); i++) {
    for (double j = 0; j < initInput.size(); j++) {
      initOutput[i][j] = initInput[i] + initInput[j];
    }
    // step4: append value
    initOutput[i][initInput.size()] = initOutput[i][initInput.size() - 1];
  }
  // step4: append value
  initOutput[initInput.size()] = initOutput[initInput.size() - 1];

  // step3: mid point
  for (double j = 0; j < initInput.size() - 1; j++) {
    initInput[j] = (initInput[j] + initInput[j + 1]) / 2;
    initInput[j] = (initInput[j] - Lutxmin) / (Lutxmax - Lutxmin);
  }
  initInput[initInput.size() - 1] =
      (initInput[initInput.size() - 1] - Lutxmin) / (Lutxmax - Lutxmin);

  // step4: append value
  initInput.push_back(1);
  initInput.push_back(0);

  cout << "Precomputed initInput: " << initInput << endl;
  cout << "Precomputed initOutput: " << initOutput << endl;

  // step5: consturct for partial sum
  vector<vector<double>> lut_x, lut_y;
  int intervalSlotNum = slot_count / pow(2, bit_num);
  for (int i = 0; i < lutSize; i++) {
    vector<double> tmp_initOutput(slot_count, 0.0);
    for (int j = 0; j < lutSize; j++) {
      tmp_initOutput[intervalSlotNum * j] = initOutput[i][j];
    }
    lut_y.push_back(tmp_initOutput);
  }
  vector<double> tmp_initInput(slot_count, 0.0);
  for (int i = 0; i < lutSize; i++) {
    tmp_initInput[intervalSlotNum * i] = initInput[i];
  }
  for (int i = 0; i < inputNum; i++) {
    lut_x.push_back(tmp_initInput);
  }

  return {lut_x, lut_y};
}

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_twoInput_propsed(int bit_num, double Lutxmax, double Lutxmin,
                             CryptoContext<DCRTPoly> &context) {
  int inputNum = 2;
  const int slot_count = context->GetEncodingParams()->GetBatchSize();
  vector<double> initInput;
  // step1: create input table
  int lutSize = pow(2, bit_num);
  // need lutSize-2 data points in input table
  // interval ensuring lutSize samples span [Lutxmin, Lutxmax] with both
  // endpoints included
  double interval = (Lutxmax - Lutxmin) / (lutSize - 3);
  cout << "interval:" << interval << endl;
  for (double i = Lutxmin; i < Lutxmax; i += interval) {
    if (int(initInput.size()) == lutSize - 3)
      break;
    initInput.push_back(i);
  }
  initInput.push_back(Lutxmax);
  cout << "initInput: " << initInput << endl;

  // Generate output
  // step2: create output table
  vector<vector<double>> initOutput(lutSize, vector<double>(lutSize, 0.0));
  for (double i = 0; i < initInput.size(); i++) {
    for (double j = 0; j < initInput.size(); j++) {
      initOutput[i][j] = initInput[i] + initInput[j];
    }
    // step4: append value
    initOutput[i][initInput.size()] = initOutput[i][initInput.size() - 1];
  }
  // step4: append value
  initOutput[initInput.size()] = initOutput[initInput.size() - 1];

  // step3: mid point
  for (double j = 0; j < initInput.size() - 1; j++) {
    initInput[j] = (initInput[j] + initInput[j + 1]) / 2;
    initInput[j] = (initInput[j] - Lutxmin) / (Lutxmax - Lutxmin);
  }
  initInput[initInput.size() - 1] =
      (initInput[initInput.size() - 1] - Lutxmin) / (Lutxmax - Lutxmin);

  // step4: append value
  initInput.push_back(1);
  initInput.push_back(0);

  cout << "Precomputed initInput: " << initInput << endl;
  cout << "Precomputed initOutput: " << initOutput << endl;

  // step5: rearrange stategy
  vector<vector<double>> rearrangedOutput(pow(lutSize, inputNum - 1),
                                          vector<double>(lutSize, 0.0));
  for (int k = 0; k < pow(lutSize, inputNum - 1); k++) {
    for (int j = 0; j < lutSize; j++) {
      rearrangedOutput[k][j] =
          initOutput[((k /
                       static_cast<size_t>(std::pow(lutSize, inputNum - 2))) %
                          static_cast<size_t>(std::pow(lutSize, inputNum - 1)) +
                      j) %
                     lutSize][j];
    }
  }
  cout << "rearrangedOutput: " << rearrangedOutput << endl;

  // step5: consturct for partial sum
  vector<vector<double>> lut_x, lut_y;
  int intervalSlotNum = slot_count / pow(2, bit_num);
  for (int i = 0; i < pow(lutSize, inputNum - 1); i++) {
    vector<double> tmp_Output(slot_count, 0.0);
    for (int j = 0; j < lutSize; j++) {
      tmp_Output[intervalSlotNum * j] = rearrangedOutput[i][j];
    }
    lut_y.push_back(tmp_Output);
  }
  vector<double> tmp_initInput(slot_count, 0.0);
  for (int i = 0; i < lutSize; i++) {
    tmp_initInput[intervalSlotNum * i] = initInput[i];
  }
  for (int i = 0; i < inputNum; i++) {
    lut_x.push_back(tmp_initInput);
  }

  return {lut_x, lut_y};
}

pair<vector<vector<double>>, vector<vector<double>>>
generateLUT_threeInput_propsed(int bit_num, double Lutxmax, double Lutxmin,
                               CryptoContext<DCRTPoly> &context) {

  int inputNum = 3;
  const int slot_count = context->GetEncodingParams()->GetBatchSize();
  vector<double> initInput;
  // step1: create input table
  int lutSize = pow(2, bit_num);
  // need lutSize-2 data points in input table
  // interval ensuring lutSize samples span [Lutxmin, Lutxmax] with both
  // endpoints included
  double interval = (Lutxmax - Lutxmin) / (lutSize - 3);
  cout << "interval:" << interval << endl;
  for (double i = Lutxmin; i < Lutxmax; i += interval) {
    if (int(initInput.size()) == lutSize - 3)
      break;
    initInput.push_back(i);
  }
  initInput.push_back(Lutxmax);
  cout << "initInput: " << initInput << endl;

  // Generate output
  // step2: create output table
  vector<vector<vector<double>>> initOutput(
      lutSize, vector<vector<double>>(lutSize, vector<double>(lutSize, 0.0)));
  for (double i = 0; i < initInput.size(); i++) {
    for (double j = 0; j < initInput.size(); j++) {
      for (double k = 0; k < initInput.size(); k++) {
        // initOutput[i][j]=initInput[i]+initInput[j];
        initOutput[i][j][k] = initInput[i] + initInput[j] + initInput[k];
      }
      initOutput[i][j][initInput.size()] =
          initOutput[i][j][initInput.size() - 1];
    }
    // step4: append value
    initOutput[i][initInput.size()] = initOutput[i][initInput.size() - 1];
  }
  // step4: append value
  initOutput[initInput.size()] = initOutput[initInput.size() - 1];

  // step3: mid point
  for (double j = 0; j < initInput.size() - 1; j++) {
    initInput[j] = (initInput[j] + initInput[j + 1]) / 2;
    initInput[j] = (initInput[j] - Lutxmin) / (Lutxmax - Lutxmin);
  }
  initInput[initInput.size() - 1] =
      (initInput[initInput.size() - 1] - Lutxmin) / (Lutxmax - Lutxmin);

  // step4: append value
  initInput.push_back(1);
  initInput.push_back(0);

  cout << "Precomputed initInput: " << initInput << endl;
  cout << "Precomputed initOutput: " << initOutput << endl;

  // step5: rearrange stategy
  vector<vector<double>> rearrangedOutput(pow(lutSize, inputNum - 1),
                                          vector<double>(lutSize, 0.0));
  for (int k = 0; k < pow(lutSize, inputNum - 1); k++) {
    for (int j = 0; j < lutSize; j++) {
      rearrangedOutput[k][j] = initOutput
          [((k / static_cast<size_t>(std::pow(lutSize, inputNum - 2))) %
                static_cast<size_t>(std::pow(lutSize, inputNum - 1)) +
            j) %
           lutSize]
          [((k / static_cast<size_t>(std::pow(lutSize, inputNum - 3))) %
                static_cast<size_t>(std::pow(lutSize, inputNum - 2)) +
            j) %
           lutSize][j];
    }
  }
  cout << "rearrangedOutput: " << rearrangedOutput << endl;

  // step5: consturct for partial sum
  vector<vector<double>> lut_x, lut_y;
  int intervalSlotNum = slot_count / pow(2, bit_num);
  for (int i = 0; i < pow(lutSize, inputNum - 1); i++) {
    vector<double> tmp_Output(slot_count, 0.0);
    for (int j = 0; j < lutSize; j++) {
      tmp_Output[intervalSlotNum * j] = rearrangedOutput[i][j];
    }
    lut_y.push_back(tmp_Output);
  }
  vector<double> tmp_initInput(slot_count, 0.0);
  for (int i = 0; i < lutSize; i++) {
    tmp_initInput[intervalSlotNum * i] = initInput[i];
  }
  for (int i = 0; i < inputNum; i++) {
    lut_x.push_back(tmp_initInput);
  }

  return {lut_x, lut_y};
}