#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include "../src/fib_heap.h"
#include "gtest/gtest.h"
namespace fh {

	volatile auto f = []() {};
	using heap_t = fibonacci_heap<unsigned long, decltype(f)>;

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

	TEST_F(NodeTests, ListForeach) {
		const auto list1 = &a;
		a.right = &b; a.left = &e;
		b.right = &c; b.left = &a;
		c.right = &d; c.left = &b;
		d.right = &e; c.left = &c;
		e.right = &a; c.left = &d;
		int i = 0;
		heap_t::list_foreach(list1, [&i](auto){++i;});
		ASSERT_EQ(5, i);
	}

	TEST(HeapTests, HeapForEachTest) {
		heap_t heap(f);
		heap.insert(0, 87492);
		heap.insert(1, 10204);
		heap.insert(2, 47342);
		heap.insert(3, 84604);
		heap.insert(4, 10687);
		heap.insert(5, 74631);
		heap.insert(6, 52664);
		heap.insert(7, 72255);
		heap.insert(8, 54064);
		heap.insert(9, 60859);
		heap.insert(10, 51388);
		heap.insert(11, 14960);
		heap.insert(12, 33036);
		heap.insert(13, 32276);
		heap.insert(14, 54469);
		heap.insert(15, 76356);
		heap.insert(16, 62080);
		heap.insert(17, 13224);
		heap.insert(18, 59346);
		heap.insert(19, 59802);
		heap.insert(20, 88505);
		heap.insert(21, 99228);
		heap.insert(22, 88038);
		heap.insert(23, 97396);
		heap.insert(24, 27601);
		heap.insert(25, 48481);
		heap.insert(26, 50541);
		heap.insert(27, 39545);
		heap.insert(28, 65430);
		heap.insert(29, 25065);
		heap.insert(30, 21241);
		heap.insert(31, 1956);
		heap.insert(32, 68790);
		heap.insert(33, 18763);
		heap.insert(34, 64015);
		heap.insert(35, 27660);
		heap.insert(36, 51960);
		heap.insert(37, 98439);
		heap.insert(38, 13394);
		heap.insert(39, 22170);
		heap.insert(40, 98724);
		heap.insert(41, 51259);
		heap.insert(42, 36340);
		heap.insert(43, 34283);
		heap.insert(44, 6592);
		heap.insert(45, 70561);
		heap.insert(46, 92654);
		heap.insert(47, 96953);
		heap.insert(48, 95482);
		heap.insert(49, 73280);
		heap.insert(50, 347);
		heap.insert(51, 1213);
		heap.insert(52, 43369);
		heap.insert(53, 84935);
		heap.insert(54, 868);
		heap.insert(55, 74310);
		heap.insert(56, 1648);
		heap.insert(57, 7582);
		heap.insert(58, 20389);
		heap.insert(59, 4324);
		heap.insert(60, 98696);
		heap.insert(61, 41);
		heap.insert(62, 36642);
		heap.insert(63, 94769);
		heap.insert(64, 55817);
		heap.insert(65, 86189);
		heap.insert(66, 78894);
		heap.insert(67, 48759);
		heap.insert(68, 52388);
		ASSERT_EQ(heap.elements_count, 69);
		int i = 0;
		auto ff = [&](auto) { ++i; };
		heap_t::heap_foreach(heap.min_node, ff);
		ASSERT_EQ(i, 69);
	}
}
