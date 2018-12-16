#ifndef FIB_HEAP_HPP
#define FIB_HEAP_HPP

#include <cstdint>
#include <cassert>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>
#include <functional>
#include <new>
#include "gtest/gtest.h"

#define NDEBUG


namespace fh
{
	template<typename T, typename Function>
	class fibonacci_heap {
#ifndef NDEBUG
    public:
#endif
		friend class NodeTests;
		class node {
		public:
			node* parent = nullptr;
			node* right = nullptr;
			node* left = nullptr;
			node* child = nullptr;
			T value;
			int_least32_t priority = 0;
			uint_least32_t rank = 0;
			bool marked = false;

			node() : right(this), left(this), value(T()) {
			}
			explicit node(T&& val) : right(this), left(this), value(std::forward<T>(val)) {
			}
			// As fast as possible (just C++ thing).
			explicit node(const T& val) : right(this), left(this), value(val) {
			}
            node(T&& val, const int_least32_t priority) : right(this), left(this), value(std::forward<T>(val)),
                                                          priority(priority) {
            }
            // As fast as possible (just C++ thing).
            node(const T& val, const int_least32_t priority) : right(this), left(this), value(val), priority(priority) {
            }

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
			node* remove_from_neighbors() {
				// Trivial case of linked list with one element.
				if (!has_neighbors())
					return nullptr;
				auto handle = right;
				right->left = left;
				left->right = right;
				left = right = this;
				return handle;
			}

			bool has_neighbors() const { return right != this; }
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

        // Copy array to memory block twice the array_size whichc is in-out parameter
        // set to the doubled size. Delete array, return pointer to the realocated memory block.
        node** double_array(node** array, std::size_t* array_size) {
            auto nextSize = (*array_size) * 2;
            auto temp = new node*[nextSize];
            std::memcpy( temp, array, (*array_size) * sizeof(node*) / sizeof(unsigned char) );
            *array_size = nextSize;
            delete[] array;
            return temp;
        }

        template<typename Action>
        void heap_foreach(node* node_, Action& action) {
            if (node_) {
                std::size_t size = 100;
                auto stack = new node*[size];
                int_least32_t stack_top = 0;
                stack[0] = min_node;

                while (stack_top >= 0) {
                    auto curr = stack[stack_top];
                    --stack_top;
                    list_foreach(curr, [&](auto it) {
                        node n(it->value, it->priority);
                        n.marked = it->marked;
                        action(&n); // dont allow problem cause by recursive application of the action
                        if (it != curr && it->child){
                            ++stack_top;
                            if (stack_top >= int_least64_t(size)) {
                                stack = double_array(stack, &size);
                            }
                            stack[stack_top] = it->child;
                        }
                    });
                }
                delete[] stack;
            }
        }

		Function& fun_;
	public:
        using node_t = node *;
//        struct node_handle {
//            node_handle() : value(T()), priority(0) {}
//            node_handle(T val, int_least32_t prio, node_t rf) : value(val), priority(prio), ref(rf) {}
//            T value;
//            int_least32_t priority = 0;
//            node_t ref = nullptr;
//            bool valid() const noexcept { return priority != std::numeric_limits<int_least32_t>::min(); }
//        };
		node* min_node = nullptr;
		uint_least32_t elements_count = 0;
		// I strove for using some template magic with SFINAE, but my compiler
		// protested (low version with only mobile data internet access is a bad combination).
		// If using older standard than c++17 the complexity and readability
		// would grew too much, so I used only bool and one explicit constructor
		// (if function is not needed one can use [](){} as input).
		bool naive_implementation = false;

		explicit fibonacci_heap(Function& f) : fun_(f) {
		}

		// At the moment, check that I do not do anything unexpected.
		fibonacci_heap(const fibonacci_heap& other) = delete;
		fibonacci_heap(fibonacci_heap&& other) noexcept = delete;
		fibonacci_heap& operator=(const fibonacci_heap& other) = delete;
		fibonacci_heap& operator=(fibonacci_heap&& other) noexcept = delete;

		// We return const pointer (node_handle_t).
		node_t insert(const T& value, const int_least32_t priority) {
			auto next_node = new node(value, priority);
			node::merge(min_node, next_node);
            if (!min_node)
                min_node = next_node;
            else
			    min_node = (min_node->priority > priority) ? next_node : min_node;
			++elements_count;
            #ifndef NDEBUG
			list_foreach(min_node, [this](auto it) {
				assert(!it->parent);
				assert(!it->marked);
			});
            heap_foreach(min_node, check_minimality);
            #endif
			return next_node;
		}

		node_t find_min() const noexcept {
			return min_node;
		}

		// Return min node with left and right pointers pointing to min node, parent, child are nullptr.
		node* delete_min() {
			if (!min_node)
				return nullptr;
			--elements_count;
			// Update statistics (just step count).
			auto heap_minimum_ptr = min_node;
			// at the next step min_node is not minimal
			min_node = detach_min_and_level_min_children_up();
			min_node = consolidate(min_node); // finds minimum
            #ifndef NDEBUG
            heap_foreach(min_node, check_minimality);
            #endif
            fun_(false, true); // decrease = false, end of operation = true
            return heap_minimum_ptr;
		}

		void decrease(node_t node_, int_least32_t next_priority) {
			if (!node_ || node_->priority < next_priority)
				return;
            node_->priority = next_priority;
			if (node_->is_root()) {
				if (min_node->priority > next_priority){
                    assert(!node_->marked);
                    min_node = node_;
                }
                #ifndef NDEBUG
				list_foreach(min_node, [this](auto it) {
					assert(!it->parent);
					assert(!it->marked);
					assert(it->priority >= min_node->priority);
				});
                #endif
				return;
			}
			if (node_->priority < node_->parent->priority)
			    cut(node_);
			// Update statistics (step_(naive_)decrease_count).
			if (min_node->priority > node_->priority){
				assert(!node_->marked);
				min_node = node_;
			}
            #ifndef NDEBUG
            heap_foreach(min_node, check_minimality);
            #endif
            fun_(true, true); // decrease = true, end of operation = true
		}

		fibonacci_heap merge(fibonacci_heap&& heap2) {
			min_node = (min_node <= heap2.min_node) ? min_node : heap2.min_node;
			node::merge(min_node, heap2.min_node);
			elements_count += heap2.elements_count;
			heap2.min_node = nullptr;
			return *this;
		}

		void clear() {
			dfs_clear();
			elements_count = 0;
			min_node = nullptr;
		}

		virtual ~fibonacci_heap() {
			dfs_clear();
		}

	private:
		std::function<void(node*)> check_minimality = [this](node* it) {
			assert(it->priority >= min_node->priority);
			if (it->parent)
				assert(it->priority >= it->parent->priority);
		};

		void dfs_clear() {
			if (min_node) {
				auto pointers = new node*[elements_count];
                std::size_t size = 100;
				auto stack = new node*[size];
				std::size_t first_unused_ptr = 0;
                int_least64_t stack_top = 0;
				stack[0] = min_node;

				while (stack_top >= 0) {
					auto curr = stack[stack_top];
					--stack_top;
					auto fst_unused = first_unused_ptr;
					list_foreach(curr, [&fst_unused, pointers, this](auto it) {

                        if (fst_unused >= elements_count)
                            throw std::runtime_error("Pointers too small.");

                        pointers[fst_unused] = it;
						++fst_unused;
					});
					for (auto i = first_unused_ptr; i < fst_unused; ++i) {
						++stack_top;
                        if (stack_top >= int_least64_t(size)) {
                            stack = double_array(stack, &size);
                        }
						stack[stack_top] = pointers[i]->child;
						pointers[i]->child = nullptr;
						pointers[i]->remove_from_neighbors();
						pointers[i]->parent = nullptr;
					}
					first_unused_ptr = fst_unused;
				}
				for (size_t i = 0; i < elements_count; ++i) {
					delete pointers[i];
				}
				delete[] stack;
				delete[] pointers;
			}
		}

		// Put min_node children on min's top heap level LL, null min_node's child ptr.
		node* detach_min_and_level_min_children_up() {
		#ifndef NDEBUG
			auto neighbors = 0;
			list_foreach(min_node, [&](auto it) { ++neighbors; assert(!it->marked); });
		#endif
            // For not losing pointer to the linked list.
			auto min_node_neighbors = min_node->remove_from_neighbors();
		#ifndef NDEBUG
			auto neighbors_next = 0;
			list_foreach(min_node_neighbors, [&](auto) { ++neighbors_next; });
			assert(neighbors == neighbors_next + 1);
		#endif
			// Remove parental pointers from children (min_node has still pointer to the child list).
			// Update statistics ((naive_)delete_min_count).
			list_foreach(min_node->child, [&](auto it) {
				it->parent = nullptr;
				it->marked = false;
				fun_(false, false); // decrease = false, end of operation = false
				assert(min_node->priority <= it->priority);
			});
		#ifndef NDEBUG
			auto children = 0;
			list_foreach(min_node->child, [&](auto) { ++children; });
		#endif
			// Join top heap level linked list with minimum's children and null min's child.
			auto heap_trees = node::merge(min_node->child, min_node_neighbors);
		#ifndef NDEBUG
			auto all = 0;
			list_foreach(heap_trees, [&](auto) { ++all; });
			assert(all == neighbors_next + children);
		#endif
			min_node->child = nullptr;
			min_node->rank = 0;
			return heap_trees;
		}

		// Removes node with its subtree from the tree it belongs to and merges it with top level heap LL.
		void cut(node* node) {
			if (!node || node->is_root())
				return;
			// Update statistics ((naive_)decrease_count).
			auto node_parent = node->parent;
			node_parent->child = node->remove_from_neighbors();
			--(node_parent->rank);
			node->marked = false;
			node->parent = nullptr;

		#ifndef NDEBUG
			auto neighbors = 0;
			list_foreach(min_node, [&](auto) { ++neighbors; });
        #endif

            fun_(true, false); // decrease = true, end of operation = false
			node::merge(min_node, node);

		#ifndef NDEBUG
			auto neighbors_next = 0;
			list_foreach(min_node, [&](auto) { ++neighbors_next; });
			assert(neighbors_next == neighbors + 1);
        #endif

			if (!naive_implementation) {
				if (node_parent->marked)
					cut(node_parent);
			}
			if (!node_parent->is_root())
				node_parent->marked = true;
			if (min_node->priority > node->priority){
				assert(!node->marked);
				min_node = node;
			}
		}

		node* consolidate(node* node_list) {
			// Trivial cases: heap has none or one node.
			if (!node_list || !node_list->has_neighbors())
				return node_list;
			const auto size = 100; //static_cast<std::size_t>(ceil(log2(elements_count + 1)) + 1);

			// Within consolidation we reconnect tree pointers, so its safer to hold trees separately.
			std::size_t tree_num = 0;
			list_foreach(node_list, [&tree_num](auto) { ++tree_num; });
            auto trees = new node*[tree_num];
            int_fast64_t tree_num_counter = tree_num - 1;
            list_foreach(node_list, [&trees, &tree_num_counter](auto it) {
                assert(it->parent == nullptr);
                assert(it->marked == false);
                trees[tree_num_counter] = it;
                --tree_num_counter;
            });
			for (std::size_t i = 0; i < tree_num; ++i)
				trees[i]->remove_from_neighbors();

			auto pointers = new node*[size];
			std::memset(pointers, 0, size * sizeof(node*) / sizeof(unsigned char));
            for (int j = 0; j < size; ++j) {
                assert(!pointers[j]);
            }

			const auto join_trees = [&](node* const t1, node* const t2) {
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
			for (std::size_t i = 0; i < tree_num; ++i) {
				auto tree = trees[i];
				auto rank = tree->rank;
                if (rank >= size)
                    throw std::runtime_error("Weird rank or small size.");
				while (pointers[rank]) {
					// Update statistics ((naive_)delete_min_count).
					fun_(false, false); // decrease = false, end of operation = false
					tree = join_trees(pointers[rank], tree);
					pointers[rank] = nullptr;
					++rank;
                    if (rank >= size)
                        throw std::runtime_error("Weird rank or small size.");
				}
				pointers[rank] = tree;
			}
			// Recreate the array into another valid linked list && find min of the list.
			node* iterator = nullptr;
            node* next_min = nullptr;
			for (std::size_t i = 0; i < size; ++i) {
				if (pointers[i]) {
                    if (!next_min)
                        next_min = pointers[i];
                    else {
                        next_min = (pointers[i]->priority < next_min->priority) ? pointers[i] : next_min;
                    }
					assert(!pointers[i]->has_neighbors());
					iterator = node::merge(pointers[i], iterator);
				}
			}
            list_foreach(iterator, [&, this](auto it){ assert(it->priority >= next_min->priority); });
			delete[] pointers;
			delete[] trees;
			return next_min;
		}
	};
}


#endif /* FIB_HEAP_HPP */
