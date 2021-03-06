//
//  checks.cpp
//  Jacobian
//
//  Created by David Freifeld
//

#include "./src/utils.hpp"
#include "./src/bpnn.hpp"
 
#define ZERO_THRESHOLD 5*pow(10, -5)

// Simple example network to be used in each check.
Network default_net()
{
    Network net ("./data_banknote_authentication.txt", 16, 0.0155, 0.03, L2, 0, 0.9);
    net.add_layer(4, "linear", linear, linear_deriv);
    net.add_layer(5, "lecun_tanh", lecun_tanh, lecun_tanh_deriv);
    net.add_layer(2, "linear", linear, linear_deriv);
    net.init_optimizer("momentum", 0);
    net.initialize();
    net.silenced = true;
    return net;
}

// Proper cloning of networks for providing a reference point.
Network explicit_copy(Network src)
{
    Network dst ("./data_banknote_authentication.txt", 16, 0.0155, 0.03, L2, 0, 0.9);
    dst = src;
    for (int i = 0; i < src.layers.size(); i++) {
        dst.layers[i] = src.layers[i];
    }
    return dst;
}

// Regularization should increase the cost.
void regularization_check(int& sanity_passed, int& total_checks)
{
    Network net = default_net();
    std::cout << "\u001b[4m\u001b[1mSANITY CHECKS:\u001b[0m\n";
    // Check if regularization strength increases loss (as it should).
    std::cout << "Regularization check...";

    net.list_net();
    Network copy1 = explicit_copy(net);
    Network copy2 = explicit_copy(net);
    copy1.next_batch(copy1.data);
    copy1.feedforward();
  
    copy2.next_batch(copy2.data);
    copy2.feedforward();
    net.list_net();
    if (copy1.cost() > copy2.cost()) {
        std::cout << " \u001b[32mPassed!\n\u001b[37m";
        sanity_passed++;
    }
    else std::cout << " \u001b[31mFailed.\n\u001b[37m";
    total_checks++;
}

// Given a small batch size and enough time, the network should be able to get its cost very close to zero.
void zero_check(int& sanity_passed, int& total_checks)
{
    Network net = default_net();
    std::cout << "Zero-cost check...";
    net.next_batch(net.data);
    float finalcost;
    for (int i = 0; i < 10000; i++) {
      net.feedforward();
      net.backpropagate();
      finalcost = net.cost();
      if (finalcost <= ZERO_THRESHOLD) {
        break;
      }
    }
    if (finalcost <= ZERO_THRESHOLD) {
      std::cout << " \u001b[32mPassed!\n\u001b[37m";
      sanity_passed++;
    }
    else std::cout << " \u001b[31mFailed.\n\u001b[37m";
    total_checks++;
}

// There should be no weird floating point numbers in layer updates.
void floating_point_check(int& sanity_passed, int& total_checks)
{
    Network net = default_net();
    std::cout << "Update floating-point check...";
    net.next_batch(net.data);
    net.feedforward();
    net.backpropagate();
    for (int i = 0; i < net.length-1; i++) {
        for (int j = 0; j < net.layers[i].m->rows(); j++) {
            for (int k = 0; k < net.layers[i].m->cols(); k++) {
                if ((*net.layers[i].m)(j,k) == -0 || (*net.layers[i].m)(j,k) == INFINITY || (*net.layers[i].m)(j,k) == NAN || (*net.layers[i].m)(j,k) == -INFINITY) {
                    std::cout << " \u001b[31mFailed.\n\u001b[37m";
                    total_checks++;
                    return;
                }
            }
        }
    }
    std::cout << " \u001b[32mPassed!\n\u001b[37m";
    sanity_passed++;
    total_checks++;
}

void update_check(int& sanity_passed, int& total_checks)
{
    // std::cout << "Layer updates sanity check...";
    // Network copy6 ("./data_banknote_authentication.txt", 16, 0.05, 0.03, L2, 0, 0.9);
    // Network copy7 ("./data_banknote_authentication.txt", 16, 0.05, 0.03, L2, 0, 0.9);
    // //copy2.list_net();
    // //copy1.list_net();
    // int passed;
    // for (int i = 0; i < copy1.layers.size()-1; i++) {
    //   if (*copy1.layers[i].weights == *copy2.layers[i].weights) {
    //     //      std::cout << *copy2.layers[i].weights <<"uninitweight\n\n";
    //     //      std::cout << *copy1.layers[i].weights << " "<<i<<"weight\n\n";
    //     passed = -1;
    //   }
    // }
    // for (int i = 1; i < copy1.layers.size(); i++) {
    //   if (*copy1.layers[i].bias == *copy2.layers[i].bias) {
    //     //      std::cout << *copy2.layers[i].bias <<"uninitbias\n\n";
    //     //  std::cout << *copy1.layers[i].bias <<" " << i << "bias\n\n";
    //     passed = -1;
    //   }
    // }
    // if (passed == 1) {
    //   std::cout << " \u001b[32mPassed!\n\u001b[37m";
    //   sanity_passed++;
    // }
    // else std::cout << " \u001b[31mFailed.\n\u001b[37m";
}

void sanity_checks()
{
    int sanity_passed = 0;
    int total_checks = 0;
    zero_check(sanity_passed, total_checks);
    floating_point_check(sanity_passed, total_checks);
    std::cout << "\u001b[1m\nPassed " << sanity_passed << "/" << total_checks <<" sanity checks.\u001b[0m\n";
    if ((float)sanity_passed/total_checks < 0.5) {
        std::cout << "Majority of sanity checks failed. Exiting." << "\n";
        exit(1);
    }
}

void run_check(int& basic_passed, int& total_checks)
{
    std::cout << "Default net check...";
    try {
        Network net = default_net();
        for (int i = 0; i < 50; i++) {
            net.train();
        }
    }
    catch (...) {
        std::cout << " \u001b[31mFailed.\n\u001b[37m";
        total_checks++;
        return;
    }
    std::cout << " \u001b[32mPassed!\n\u001b[37m";
    basic_passed++;
    total_checks++;
}

void optimizers_check(int& basic_passed, int& total_checks)
{
    std::cout << "Optimizers check...";
    try {
        std::string optimizers [5] = {"momentum", "demon", "adam", "adamax", "sgd"};
        for (std::string optimizer : optimizers) {
            Network net ("./data_banknote_authentication.txt", 16, 0.0155, 0.03, L2, 0, 0.9);
            net.add_layer(4, "linear", linear, linear_deriv);
            net.add_layer(5, "lecun_tanh", lecun_tanh, lecun_tanh_deriv);
            net.add_layer(2, "linear", linear, linear_deriv);
            if (optimizer == "momentum") net.init_optimizer("momentum", 0.9);
            if (optimizer == "demon") net.init_optimizer("demon", 0.9, 50);
            if (optimizer == "adam") net.init_optimizer("adam", 0.999, 0.9, pow(10,-6));
            if (optimizer == "adamax") net.init_optimizer("adamax", 0.999, 0.9, pow(10,-6));
            if (optimizer == "sgd") net.init_optimizer("sgd");
            net.initialize();
            net.silenced=true;
            for (int i = 0; i < 50; i++) {
                net.train();
            }
        }
    }
    catch (...) {
        std::cout << " \u001b[31mFailed.\n\u001b[37m";
        total_checks++;
        return;
    }
    std::cout << " \u001b[32mPassed!\n\u001b[37m";
    basic_passed++;
    total_checks++;
}

void prelu_check(int& basic_passed, int& total_checks)
{
    std::cout << "PReLU check...";
    try {
        Network net = default_net();
        for (int i = 0; i < 50; i++) {
            Network net ("./data_banknote_authentication.txt", 16, 0.0155, 0.03, L2, 0, 0.9);
            net.add_layer(4, "linear", linear, linear_deriv);
            net.add_prelu_layer(5, 0.01);
            net.add_layer(2, "linear", linear, linear_deriv);
            net.initialize();
            net.silenced=true;
        }
    }
    catch (...) {
        std::cout << " \u001b[31mFailed.\n\u001b[37m";
        total_checks++;
        return;
    }
    std::cout << " \u001b[32mPassed!\n\u001b[37m";
    basic_passed++;
    total_checks++;
}

void basic_checks()
{
    int basic_passed = 0;
    int total_checks = 0;
    run_check(basic_passed, total_checks);
    optimizers_check(basic_passed, total_checks);
    prelu_check(basic_passed, total_checks);
    std::cout << "\u001b[1m\nPassed " << basic_passed << "/" << total_checks <<" basic checks.\u001b[0m\n";
    if ((float)basic_passed/total_checks < 0.5) {
        std::cout << "Majority of basic checks failed. Exiting." << "\n";
        exit(1);
    }
}

void grad_checks() {}

// Eigen::MatrixXf Network::numerical_grad(int i, float epsilon)
// {
//     Eigen::MatrixXf gradient (layers[i].weights->rows(), layers[i].weights->cols());
//     for (int j = 0; j < layers[i].weights->rows(); j++) {
//         for (int k = 0; k < layers[i].weights->cols(); k++) {
//             float current_cost = cost();
//             std::vector<Layer> backup = layers;
//             (*layers[i].weights)(j,k) += epsilon;
//             feedforward();
//             float end_cost = cost();
//             gradient(j,k) = (end_cost - current_cost)/epsilon;
//             layers = backup;
//             batches = 0;
//         }
//     }
//     return gradient;
// }

// void Network::grad_check()
// {
//     std::vector<Layer> backup = layers;
//     feedforward();
//     layers = backup;
//     batches = 0;
//     std::vector<Eigen::MatrixXf> gradients;
//     std::vector<Eigen::MatrixXf> deltas;
//     Eigen::MatrixXf error (layers[length-1].contents->rows(), layers[length-1].contents->cols());
//     for (int i = 0; i < error.rows(); i++) {
//         for (int j = 0; j < error.cols(); j++) {
//             float truth;
//             if (j==(*labels)(i,0)) truth = 1;
//             else truth = 0;
//             error(i,j) = (*layers[length-1].contents)(i,j) - truth;
//             checknan(error(i,j), "gradient of final layer");
//         }
//     }
//     int counter = 1;
//     gradients.push_back(error);
//     deltas.push_back((*layers[length-2].contents).transpose() * gradients[0]);
//     for (int i = length-2; i >= 1; i--) {
//         gradients.push_back(cwise_product(gradients[counter-1] * layers[i].weights->transpose(),*layers[i].dZ));
//         std::cout << layers[i-1].contents->transpose() * gradients[counter];
//         deltas.push_back(layers[i-1].contents->transpose() * gradients[counter]);
//         counter++;
//     }
// }
