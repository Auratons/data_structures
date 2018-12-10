#ifndef FIB_HEAP_HPP
#define FIB_HEAP_HPP

#define NDEBUG 0

#include <cstdint>
#include <cassert>
#include <cstring>
#include <limits>
#include <type_traits>

namespace fh {

	template<typename T, typename Function>
	class fibonacci_heap {
		class node {
		public:
			node* parent = nullptr;
			node* right = this;
			node* left = this;
			node* child = nullptr;
			T value;
			int_least32_t priority = 0;
			uint_least32_t rank = 0;
			bool marked = false;

			node() : value(T()) {}

			explicit node(T&& val) : value(std::forward<T>(val)) {}

			// As fast as possible (just C++ thing).
			explicit node(const T& val) : value(val) {}

			// At the moment, check that I do not do anything unexpected.
			node(const node& other) = delete;
			node(node&& other) noexcept = delete;
			node& operator=(const node& other) = delete;
			node& operator=(node&& other) noexcept = delete;

			// Merge two LLs into one LL, return one random node from the resulting LL.
			static node* merge(node* double_linked_list1, node* double_linked_list2) noexcept {
				if (!double_linked_list1)
					return double_linked_list2;
				if (!double_linked_list2)
					return double_linked_list1;
				auto last1 = double_linked_list1->right;
				auto last2 = double_linked_list2->left;
				double_linked_list1->right = double_linked_list2;
				double_linked_list2->left = double_linked_list1;
				last1->left = last2;
				last2->right = last1;
				return double_linked_list2;
			}

			// Remove this from the linked list of its neighbors and return handle to the linked list.
			node* remove_from_neighbors() noexcept {
				// Trivial case of linked list with one element.
				if (!has_neighbors())
					return nullptr;
				auto handle = right;
				right->left = left;
				left->right = right;
				left = right = this;
				return handle;
			}

			bool has_neighbors() const noexcept { return !((right == left) && (left == this)); }
			bool is_root() const noexcept { return parent == nullptr; }

			~node() {
				delete parent;
				if (right != this) delete right;
				if (left != this) delete left;
				delete child;
			}
		};

		template<typename Action>
		void list_foreach(node* list, Action&& action) const {
			if (!list)
				return;
			auto end = list;
			auto iterator = list;
			for (; iterator->right != end; iterator = iterator->right) {
				action(iterator);
			} // Cycle stops before running action on the last node.
			action(iterator); // Handles even list with one element.
		}

		Function& fun_;
	public:
		using node_t = node*;
		node* min_node = nullptr;
		uint_least32_t elements_count = 0;
		// I strove for using some template magic with SFINAE, but my compiler
		// protested (low version with only mobile data internet access is a bad combination).
		// If using older standard than c++17 the complexity and readability
		// would grew too much, so I used only bool and one explicit constructor
		// (if function is not needed one can use [](){} as input).
		bool naive_implementation = false;

		explicit fibonacci_heap(Function& f) : fun_(f) {}

		// At the moment, check that I do not do anything unexpected.
		fibonacci_heap(const fibonacci_heap& other) = delete;
		fibonacci_heap(fibonacci_heap&& other) noexcept = delete;
		fibonacci_heap& operator=(const fibonacci_heap& other) = delete;
		fibonacci_heap& operator=(fibonacci_heap&& other) noexcept = delete;

		// We return const pointer (node_handle_t).
		node_t insert(T&& value, const int_least32_t priority) {
			auto new_node = new node(value);
			new_node->priority = priority;
			// This follows basic way of merging this heap
			// with new heap with only one new_node inside.
			// But practically it is useless to create new
			// complete heap, we can use just private merge.
			node::merge(min_node, new_node);
			min_node = (min_node && min_node->priority < priority) ? min_node : new_node;
			++elements_count;
			return new_node;
		}

		// As fast as possible (just C++ things).
		// We return const pointer (node_handle_t).
		node_t insert(const T& value, const int_least32_t priority) {
			auto new_node = new node(value);
			new_node->priority = priority;
			// This follows basic way of merging this heap
			// with new heap with only one new_node inside.
			// But practically it is useless to create new
			// complete heap, we can use just private merge.
			node::merge(min_node, new_node);
			min_node = (min_node && min_node->priority < priority) ? min_node : new_node;
			++elements_count;
			return new_node;
		}

		node_t find_min() const noexcept {
			return min_node;
		}

		// Return min node with correctly left and right pointers set to node
		// and child as nullptr. Sets this min_node to new min node in heap.
		node* delete_min() {
			if (!min_node)
				return nullptr;
			--elements_count;
			// Update statistics (just step count).
			fun_(false, true);
			auto heap_minimum_ptr = min_node;
			// Returns new heap LL of trees!
			min_node = detach_min_and_level_min_children_up();
			consolidate();
			// Find new min.
			if (min_node) {
				auto min_it = min_node;
				list_foreach(min_node, [&min_it](auto it) {
					assert(!it->parent);
					assert(!it->marked);
					if (it->priority < min_it->priority)
						min_it = it;
				});
				min_node = min_it;
			}
			return heap_minimum_ptr;
		}

		void decrease(node* node, int_least32_t new_priority) {
			if (!node)
				return;
			node->priority = new_priority;
			if (node->is_root()) {
				if (min_node->priority >= new_priority)
					min_node = node;
				return;
			}
			if (node->parent->priority <= node->priority)
				return;
			cut(node);
			// Update statistics (step_(naive_)decrease_count).
			fun_(true, true);
		}

		fibonacci_heap merge(fibonacci_heap&& heap2) {
			min_node = (min_node <= heap2.min_node) ? min_node : heap2.min_node;
			node::merge(min_node, heap2.min_node);
			elements_count += heap2.elements_count;
			heap2.min_node = nullptr;
			return this;
		}

		node* remove_node(node* node) {
			// I could easily avoid using STL, but this is a trifle and
			// much more readable and better maintainable.
			decrease(node, std::numeric_limits<int_least32_t>::min());
			--elements_count;
			return delete_min();
		}

		void clear() {
			// Simple and asymptotically slower than necessary, I know. But faster to write
			// than DFS collection of nodes and then deleting one by one.
			auto n = delete_min();
			while (n) {
				delete n;
				n = delete_min();
			}
			elements_count = 0;
			min_node = nullptr;
		}

		virtual ~fibonacci_heap() {
			clear();
		}

	private:
		// Put min_node children on min's top heap level LL, null min_node's child ptr.
		node* detach_min_and_level_min_children_up() {
			// For not losing pointer to the linked list.
			auto min_node_neighbors = min_node->remove_from_neighbors();
			// Remove parental pointers from children (min_node has still pointer to the child list).
			// Update statistics ((naive_)delete_min_count).
			list_foreach(min_node->child, [&](auto it) { it->parent = nullptr; it->marked = false; fun_(false, false); });
			// Join top heap level linked list with minimum's children and null min's child.
			auto heap_trees = node::merge(min_node->child, min_node_neighbors);
			min_node->child = nullptr;
			min_node->rank = 0;
			return heap_trees;
		}

		// Removes node with its subtree from the tree it belongs to and merges it with top level heap LL.
		void cut(node* node) {
			if (!node || node->is_root())
				return;
			// Update statistics ((naive_)decrease_count).
			fun_(true, false);
			auto node_parent = node->parent;
			node_parent->child = node->remove_from_neighbors();
			--(node_parent->rank);
			node->marked = false;
			node->parent = nullptr;
			node::merge(min_node, node);
			if (!naive_implementation) {
				if (node_parent->marked)
					cut(node_parent);
			}
			if (!node_parent->is_root())
				node_parent->marked = true;
		}

		void consolidate() {
			// Trivial cases: heap has none or one node.
			if (!min_node || !min_node->has_neighbors())
				return;
			const auto size = 100; //static_cast<std::size_t>(ceil(log2(elements_count + 1)) + 1);

			// Within consolidation we reconnect tree pointers, so its safer to hold trees separately.
			std::size_t tree_num = 0;
			auto trees = new node*[size];
			list_foreach(min_node, [&trees, &tree_num](auto it) {
				trees[tree_num] = it; 
				++tree_num;
			});
			for (std::size_t i = 0; i < tree_num; i++)
				trees[i]->remove_from_neighbors();

			auto pointers = new node*[size];
			std::memset(pointers, 0, size * sizeof(node*) / sizeof(unsigned char));

			const auto join_trees = [&](node* t1, node* t2) {
				assert(!t1->has_neighbors() && !t2->has_neighbors());					
				if (t1->priority < t2->priority) {
					t2->parent = t1;
					t1->child = node::merge(t1->child, t2);
					++(t1->rank);
					return t1;
				}
				t1->parent = t2;
				t2->child = node::merge(t2->child, t1);
				++(t2->rank);
				return t2;
			};

			// For each tree in the heap that has at least two trees.
			for (std::size_t i = 0; i < tree_num; i++) {
				auto tree = trees[i];
				auto rank = tree->rank;
				while (pointers[rank]) {
					// Update statistics ((naive_)delete_min_count).
					fun_(false, false);
					tree = join_trees(pointers[rank], tree);
					pointers[rank] = nullptr;
					++rank;
				}
				pointers[rank] = tree;
			}
			// Recreate the array into another valid linked list.
			node* iterator = nullptr;
			for (std::size_t i = 0; i < size; i++) {
				if (pointers[i]) {
					assert(!pointers[i]->has_neighbors());
					iterator = node::merge(iterator, pointers[i]);
				}
			}
			min_node = iterator;
			delete[] pointers;
			delete[] trees;
		}
	};
}


#endif /* FIB_HEAP_HPP */