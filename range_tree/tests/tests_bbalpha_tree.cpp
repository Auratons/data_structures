#include "../src/bbalpha_tree.h"
#include "gtest/gtest.h"
#include <limits>

using namespace std;
using namespace rt;

using node = bbalpha<int>::node;
//    class NodeTests : public ::testing::Test {
//    protected:
//        void TearDown() override {
//        }
//    };

TEST(BBTreeTests, Build) {
    int array[10] = {1,2,3,4,5,6,7,8,9,10};
    bbalpha<int> tree(0.65f);
    tree.build(array, 10);
    tree.inorder_dfs(tree.tree, [&tree](node* n){
        auto val = 1;
        val += (n->left) ? n->left->subtree_size : 0;
        val += (n->right) ? n->right->subtree_size : 0;
        ASSERT_EQ(n->subtree_size, val);
        ASSERT_GE(n->value, n->right->value);
        ASSERT_LE(n->value, n->left->value);
        ASSERT_GE(tree.alpha * n->subtree_size, n->right->subtree_size);
        ASSERT_GE(tree.alpha * n->subtree_size, n->left->subtree_size);
    });
}

TEST(BBTreeTests, Insert) {
    bbalpha<int> tree(0.65f);
    tree.insert(2);
    tree.insert(4);
    tree.insert(10);
    tree.insert(1);
    tree.insert(3);
    tree.insert(7);
    tree.insert(8);
    tree.insert(5);
    tree.insert(6);
    tree.insert(9);
    tree.inorder_dfs(tree.tree, [&tree](node* n){
        auto val = 1;
        val += (n->left) ? n->left->subtree_size : 0;
        val += (n->right) ? n->right->subtree_size : 0;
        EXPECT_EQ(n->subtree_size, val);
        EXPECT_GE(n->value, (n->right) ? n->right->value : 0);
        EXPECT_LE(n->value, (n->left) ? n->left->value : std::numeric_limits<int>::max());
        EXPECT_GE(tree.alpha * n->subtree_size, (n->right) ? n->right->subtree_size : 0);
        EXPECT_GE(tree.alpha * n->subtree_size, (n->left) ? n->left->subtree_size : 0);
    });
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
