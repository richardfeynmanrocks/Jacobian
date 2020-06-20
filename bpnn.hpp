#include "/Users/davidfreifeld/Downloads/eigen-3.3.7/Eigen/Dense"

extern "C" {
  #include "../mapreduce/mapreduce.h"
}

#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <random>
#include <algorithm>

class Layer {
public:
  Eigen::MatrixXd* contents;
  Eigen::MatrixXd* weights;
  Eigen::MatrixXd* bias;
  Eigen::MatrixXd* dZ;

  Layer(float* vals, int rows, int columns);
  Layer(int rows, int columns);
  void initWeights(Layer next);
};

class Network {
public:
  char* fpath;

  std::vector<Layer> layers;
  int length;

  float learning_rate;
  int batch_size;
  int batches;
  Eigen::MatrixXd* labels;

  Network(char* path, int inputs, int hidden, int outputs, int neurons, int batch_sz, float rate);
  void update_layer(float* vals, int datalen, int index);

  Eigen::MatrixXd activate(Eigen::MatrixXd matrix);
  Eigen::MatrixXd activate_deriv(Eigen::MatrixXd matrix);
  void feedforward();
  void list_net();

  float cost();
  float accuracy();
  void backpropagate();
  int next_batch(char* path);
  float test(char* path);
};

void demo(int total_epochs);
int prep_file(char* path);