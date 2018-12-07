#ifndef FIB_HEAP_HPP
#define FIB_HEAP_HPP

#define DEBUG 0

#include <utility>
#include <cstdint>
#include <cmath>
#include <limits>

namespace fh {

template<typename T>
class fibonacci_heap {
public:
	using priority_t = int_least32_t;
	using rank_t = uint_least32_t;
	class node;
	using node_handle_t = node * const;
private:
	class node {
	public:
		node* parent = nullptr;
		node* right = this;
		node* left = this;
		node* child = nullptr;
		T value;
		priority_t priority = 0;
		rank_t rank = 0;
		bool marked = false;

		node() : value(T()) {
		}

		explicit node(T&& val) : value(std::move(val)) {
		}

		explicit node(const T& val) : value(val) {
		}

		// Append to_append on the right side of this.
		node* append_right(node* to_append) {
			to_append->left = this;
			to_append->right = this->right;
			this->right = to_append;
			to_append->right->left = to_append;
			return to_append;
		}

		// Append to_append on the left side of this.
		node* append_left(node* to_append) {
			to_append->right = this;
			to_append->left = this->left;
			this->left = to_append;
			to_append->left->right = to_append;
			return to_append;
		}

		// Remove this from the linked list of its neighbors and return handle to the linked list.
		node* remove_from_neighbors() {
			// Trivial case of linked list with one element.
			if (right == this)
				return nullptr;
			auto handle = right;
			right->left = left;
			left->right = right;
			left = right = this;
			return handle;
		}
	};
public:
	using priority_t = int_least32_t;
	using rank_t = uint_least32_t;
	using node_handle_t = node* const;

	node* min_node = nullptr;
	uint_least32_t elements_count = 0;

	fibonacci_heap() = default;

	// We return const pointer (node_handle_t).
	node_handle_t insert(const T& value) {
		auto new_node = new node(value);
		// This follows basic way of merging this heap
		// with new heap with only one new_node inside.
		// But practically it is useless to create new
		// complete heap, we can use just private merge.
		min_node = merge(min_node, new_node);
		++elements_count;
		return new_node;
	}

	node_handle_t find_min() {
		return min_node;
	}

	// Return min node with correctly left and right pointers set to node
	// and child as nullptr. Sets this min_node to new min node in heap.
	node* extract_min() {
		if (!min_node)
			return nullptr;
		auto minimum = min_node;
		// For not losing pointer to the linked list.
		auto temp_handle = min_node->remove_from_neighbors();
		// Remove parental pointers from children.
		auto ptr = min_node->child;
		if (ptr) {
			while (ptr->right != min_node->child)
				ptr->parent = nullptr;
			// Hold only one child.
			min_node->child->parent = nullptr;
		}
		// Join top heap level linked list with minimum's children and null min's child.
		temp_handle = merge(temp_handle, min_node->child);
		min_node->child = nullptr;
		// The heap represented by min_node now shows temp_handle linked list.
		min_node = temp_handle;
		consolidate();
		// Find new min.
		ptr = min_node;
		auto min = min_node;
		while (ptr->right != min_node) {
			if (ptr->value < min->value)
				min = ptr;
#ifdef DEBUG
			static_assert(!ptr->parent);
			static_assert(!ptr->marked);
#endif
			ptr = ptr->right;
		}
		min_node = min;
		// Update elements_count.
		--elements_count;
		return minimum;
	}

	void decrease(node* node, priority_t new_priority) {
		if (!node)
			return;
		node->priority = new_priority;
		if (node == min_node || node->parent->priority <= node->priority)
			return;
		cut(node);
	}

	fibonacci_heap merge(fibonacci_heap&& heap2) {
		auto temp = std::move(heap2);
		min_node = merge(min_node, heap2.min_node);
		elements_count += temp.elements_count;
		heap2.min_node = nullptr;
		return this;
	}

	node* remove_node(node* node) {
		// I could easily avoid using STL, but this is a trifle and
		// much more readable and better maintainable.
		decrease(node, std::numeric_limits<priority_t>::min());
		--elements_count;
		return extract_min();
	}

private:
	// Removes node with its subtree from the tree it belongs to and merges it with top level heap LL.
	void cut(node* node) {
		if (!node)
			return;
		auto f = node->parent;
		--(f->rank);
		auto handle = node->remove_from_neighbors();
		f->child = handle;
		node->marked = false;
		node->parent = nullptr;
		merge(min_node, node);
		if (f->marked)
			fhcut(f);
	}

	// Merge two nodes' neighbors into one LL, return one random of the resulting nodes' list.
	static node* merge(node* heap1, node* heap2) {
		if (!heap1)
			return heap2;
		if (!heap2)
			return heap1;
		// It does not matter on the way how we merge the linked lists.
		heap2->append_right(heap1);
		return (heap1->priority < heap2->priority) ? heap1 : heap2;
	}

	void consolidate() {
		// Solve trivial cases.
		if (!min_node || min_node->right == min_node)
			return;
		// Create sufficiently large array for storing pointers and initialize it to nullptrs.
		const size_t size = ceil(log2(elements_count + 1));
		auto pointers = new node*[size];
		std::memset(pointers, nullptr, size);

		auto ptr = min_node;
		// For each tree in the heap that has at least two trees.
		while (ptr->right != min_node) {
			auto rank = ptr->rank;
			while (pointers[rank]) {
				const auto join_trees = [](node* t1, node* t2) {
					if (t1->value < t2->value) {
						t1->child->append_right(t2);
						t1->rank += t2->rank;
						return t1;
					}
					t2->child->append_right(t1);
					t2->rank += t1->rank;
					return t2;
				};
				ptr = join_trees(pointers[rank], ptr);
				pointers[rank] = nullptr;
				++rank;
			}
			pointers[rank] = ptr;
		}
		// Recreate the array into another valid linked list.
		for (size_t i = 1; i < size - 1; i++) {
			pointers[i]->right = pointers[i + 1];
			pointers[i]->left = pointers[i - 1];
		}
		pointers[0]->right = pointers[1];
		pointers[0]->left = pointers[size - 1];
		pointers[size - 1]->right = pointers[0];
		pointers[size - 1]->left = pointers[size - 2];
		delete[] pointers;
	}
};
	
}


#endif /* FIB_HEAP_HPP */