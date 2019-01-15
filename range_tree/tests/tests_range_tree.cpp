#include "../src/bbalpha_tree.h"
#include "../src/2Drange_tree.h"
#include "gtest/gtest.h"
#include <limits>

using namespace std;
using namespace rt;

using node = bbalpha<int>::node;

template<typename TNode, typename Lambda>
void postorder_dfs(TNode n, Lambda &&f) {
    if (!n)
        return;
    postorder_dfs(n->right, f);
    postorder_dfs(n->left, f);
    f(n);
}

void test_ytree(bbalpha<fst_dim>::node *xnode) {
    postorder_dfs(xnode->value.ytree.tree, [=](auto n) {
        auto val = 1;
        val += (n->left) ? n->left->subtree_size : 0;
        val += (n->right) ? n->right->subtree_size : 0;
        EXPECT_EQ(n->subtree_size, val);
        EXPECT_LE(n->value.ycoord, (n->right) ? n->right->value.ycoord : std::numeric_limits<int>::max());
        EXPECT_GE(n->value.ycoord, (n->left) ? n->left->value.ycoord : 0);
        EXPECT_GE(xnode->value.ytree.alpha * n->subtree_size, (n->right) ? n->right->subtree_size : 0);
        EXPECT_GE(xnode->value.ytree.alpha * n->subtree_size, (n->left) ? n->left->subtree_size : 0);
    });
    EXPECT_EQ(xnode->subtree_size, xnode->value.ytree.elements_count);
    EXPECT_EQ(xnode->value.ytree.tree->subtree_size, xnode->value.ytree.elements_count);
}

void test_xtree(range_tree_2d tree, int_least32_t x1, int_least32_t y1, int_least32_t x2, int_least32_t y2) {
    int_least32_t test_count = 0;
    tree.postorder_dfs(tree.tree, [x1, x2, y1, y2, alpha = tree.alpha, &test_count](auto n) {
        test_ytree(n);
        auto val = 1;
        val += (n->left) ? n->left->subtree_size : 0;
        val += (n->right) ? n->right->subtree_size : 0;
        if (x1 <= n->value.xcoord && n->value.xcoord <= x2 &&
            y1 <= n->value.ycoord && n->value.ycoord <= y2)
            ++test_count;
        EXPECT_EQ(n->subtree_size, val);
        EXPECT_EQ(n->subtree_size, n->value.ytree.tree->subtree_size);
        EXPECT_LE(n->value.xcoord, (n->right) ? n->right->value.xcoord : std::numeric_limits<int>::max());
        EXPECT_GE(n->value.xcoord, (n->left) ? n->left->value.xcoord : 0);
        EXPECT_GE(alpha * n->subtree_size, (n->right) ? n->right->subtree_size : 0);
        EXPECT_GE(alpha * n->subtree_size, (n->left) ? n->left->subtree_size : 0);
    });
    ASSERT_EQ(tree.range_query(x1, y1, x2, y2), test_count);
    ASSERT_EQ(tree.elements_count, tree.tree->subtree_size);
}

TEST(RangeTreeTests, RangeCount1) {
    statistics_ s;
    range_tree_2d tree(0.65f, s);
    tree.insert(3, 2);
    tree.insert(2, 1);
    tree.insert(1, 3);
    tree.insert(1, 3);
    tree.insert(1, 4);
    tree.insert(1, 2);
    test_xtree(std::move(tree), 0, 0, 3, 3);
}

TEST(RangeTreeTests, RangeCount2) {
    statistics_ s;
    range_tree_2d tree(0.65f, s);
    tree.insert(1, 2);
    tree.insert(2, 1);
    tree.insert(3, 3);
    tree.insert(4, 3);
    tree.insert(5, 3);
    tree.insert(6, 2);
    tree.insert(7, 1);
    tree.insert(8, 3);
    tree.insert(9, 3);
    tree.insert(10, 3);
    tree.insert(11, 2);
    test_xtree(std::move(tree), 0, 0, 1, 4);
}

TEST(RangeTreeTests, RangeCount3) {
    statistics_ s;
    range_tree_2d tree(0.65f, s);
    tree.insert(2, 1);
    tree.insert(1, 2);
    tree.insert(3, 3);
    tree.insert(2, 1);
    test_xtree(std::move(tree), 0, 0, 12, 4);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
