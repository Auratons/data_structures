#include "../src/fib_heap.h"
#include "gtest/gtest.h"
namespace fh {

	volatile auto f = []() {};
	using heap_t = fibonacci_heap<int, decltype(f)>;

	class NodeTests : public ::testing::Test {
	protected:
		using node = heap_t::node;
		node a{1};
		node b{2};
		node c{3};
		node d{4};
		node e{5};

		void TearDown() override {
			a.remove_from_neighbors();
			b.remove_from_neighbors();
			c.remove_from_neighbors();
			d.remove_from_neighbors();
			e.remove_from_neighbors();
		}
	};

	// case 1, merging just two nodes
	TEST_F(NodeTests, MergeNodes) {
		auto res = node::merge(&a, &b);
		ASSERT_EQ(res, &b);
		EXPECT_EQ(res->right, &a);
		EXPECT_EQ(res->left, &a);
		res = res->right;
		EXPECT_EQ(res->right, &b);
		EXPECT_EQ(res->left, &b);
	}

	TEST_F(NodeTests, MergeNullNode) {
		auto res = node::merge(&a, nullptr);
		ASSERT_EQ(res, &a);
		res = node::merge(nullptr, &a);
		ASSERT_EQ(res, &a);
		res = node::merge(nullptr, nullptr);
		ASSERT_EQ(res, nullptr);
	}

	TEST_F(NodeTests, MergeLists) {
		const auto list1 = &a;
		a.right = &b; a.left = &c;
		b.right = &c; b.left = &a;
		c.right = &a; c.left = &b;
		const auto list2 = &d;
		d.right = d.left = &e;
		e.right = e.left = &d;
		auto res = node::merge(list1, list2);
		ASSERT_EQ(res, list2);
		ASSERT_EQ(res, &d);
		ASSERT_EQ(res->right, &e);
		ASSERT_EQ(res->right->right, &b);
		ASSERT_EQ(res->right->right->right, &c);
		ASSERT_EQ(res->right->right->right->right, &a);
		ASSERT_EQ(res->right->right->right->right->right, &d);
		ASSERT_EQ(res->left, &a);
		ASSERT_EQ(res->left->left, &c);
		ASSERT_EQ(res->left->left->left, &b);
		ASSERT_EQ(res->left->left->left->left, &e);
		ASSERT_EQ(res->left->left->left->left->left, &d);
	}

	TEST_F(NodeTests, MergeListNode) {
		const auto list1 = &a;
		a.right = &b; a.left = &c;
		b.right = &c; b.left = &a;
		c.right = &a; c.left = &b;
		const auto list2 = &d;
		auto res = node::merge(list1, list2);
		ASSERT_EQ(res, list2);
		ASSERT_EQ(res, &d);
		ASSERT_EQ(res->right, &b);
		ASSERT_EQ(res->right->right, &c);
		ASSERT_EQ(res->right->right->right, &a);
		ASSERT_EQ(res->right->right->right->right, &d);
		ASSERT_EQ(res->left, &a);
		ASSERT_EQ(res->left->left, &c);
		ASSERT_EQ(res->left->left->left, &b);
		ASSERT_EQ(res->left->left->left->left, &d);
	}

	int main(int argc, char* argv[]) {
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	}
}
