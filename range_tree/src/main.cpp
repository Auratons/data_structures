//
// Created by auratons on 12/29/18.
//

#include "bbalpha_tree.h"

using namespace rt;
using namespace std;

int main(int argc, char* argv[]) {
    auto x = bbalpha<int>(0.68f);
    int array[10] = {1,2,3,4,5,6,7,8,9,10};
    x.insert(1);
    x.build(array, 10);
}