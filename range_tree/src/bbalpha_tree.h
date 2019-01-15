#ifndef DATA_STRUCTURES_BBALPHA_TREE_H
#define DATA_STRUCTURES_BBALPHA_TREE_H

#include <stdint.h>
#include <cstddef>
#include <cstring>  // Memcpy for debugging purposes.
#include <cassert>
#include <tuple>  // Usage of tuple and tie can be easily avoided. Used for cleaner & readable code.
#include <algorithm>  // For sorting within debugging parts of code.

#define NDEBUG

namespace rt {

    struct statistics_ {
        uint_least32_t max_range_count_visits = 0;
        uint_least32_t last_range_count_visits = 0;
        uint_least32_t range_count_visits = 0;
        uint_least32_t range_count_calls = 0;
        uint_least32_t max_insert_visits = 0;
        uint_least32_t last_insert_visits = 0;
        uint_least32_t insert_visits = 0;
        uint_least32_t insert_calls = 0;

        void reset() {
            max_range_count_visits = 0;
            last_range_count_visits = 0;
            range_count_visits = 0;
            range_count_calls = 0;
            max_insert_visits = 0;
            last_insert_visits = 0;
            insert_visits = 0;
            insert_calls = 0;
        }
    };

    template<typename T>
    class bbalpha {
    public:
        class node {
        public:
            node *right = nullptr;
            node *left = nullptr;
            node *parent = nullptr;
            int_least32_t subtree_size = 1;
            T value = T();

            node() = default;
            explicit node(const T &v) : value(v) {}
            explicit node(T &&v) : value(std::move(v)) {}
            node(const node &other) = delete;
            node(node &&other) = delete;
            node &operator=(const node &other) = delete;
            node &operator=(node &&other) = delete;
            node **parent_ptr_address() const noexcept {
                node **address = nullptr;
                if (parent)
                    address = (parent->right == this) ? &(parent->right) : &(parent->left);
                return address;
            }

            /*
             * Simple recursive destructor.
             */
            ~node() {
                delete right;
                delete left;
            }
        };

        struct {
            bool operator()(node *a, node *b) const {
                return a->value > b->value;
            }
        } node_comparator;

    public:
        node *tree = nullptr;
        const double alpha;
        statistics_& stats;
        int_least32_t elements_count = 0;

        explicit bbalpha(double a, statistics_& s) : alpha(a), stats(s) {}
        bbalpha(const bbalpha &other) = delete;
        bbalpha(bbalpha &&other) noexcept :
        tree(other.tree),
        alpha(other.alpha),
        stats(other.stats),
        elements_count(other.elements_count) {
            other.tree = nullptr;
            other.elements_count = 0;
        }
        bbalpha &operator=(const bbalpha &other) = delete;
        bbalpha &operator=(bbalpha &&other) noexcept {
            tree = other.tree;
            alpha = other.alpha;
            stats = other.stats;
            elements_count = other.elements_count;
            other.tree = nullptr;
            other.elements_count = 0;
            return *this;
        }

        void statistics(const bool insert, const bool operation_end) noexcept {
            if (insert) {
                if (operation_end) {
                    if (stats.last_insert_visits > stats.max_insert_visits)
                        stats.max_insert_visits = stats.last_insert_visits;
                    stats.last_insert_visits = 0;
                    ++stats.insert_calls;
                }
                else {
                    ++stats.last_insert_visits;
                    ++stats.insert_visits;
                }
            }
            else {
                if (operation_end) {
                    if (stats.last_range_count_visits > stats.max_range_count_visits)
                        stats.max_range_count_visits = stats.last_range_count_visits;
                    stats.last_range_count_visits = 0;
                    ++stats.range_count_calls;
                }
                else {
                    ++stats.last_range_count_visits;
                    ++stats.range_count_visits;
                }
            }
        }

        virtual ~bbalpha() {
            clear();
        }

        /*
         * Deleting tree results in deleting the whole structure via recursive delete in node class.
         */
        void clear() {
            elements_count = 0;
            delete tree;
            tree = nullptr;
        }

        /*
         * Simple recursive tree building. Array must be sorted and contain
         * element_count number of elements.
         */
        node *build_t(node **array, node *parent_of_sequence, const int_least32_t begin, const int_least32_t end) {
            if (!array || begin >= end)
                return nullptr;
            statistics(true, false);
            int_least32_t half = begin + int_least32_t((end - begin) / 2);
            node *n = array[half];
            n->parent = parent_of_sequence;
            n->subtree_size = end - begin;
            n->right = build_t(array, n, begin, half);
            n->left = build_t(array, n, half + 1, end);
            return n;
        }


        /*
         * Array must be sorted and contain element_count number of elements.
         */
        node *build(T array[], const int_least32_t element_count) {
            node **ptrs = new node *[element_count];
            for (int_least32_t i = 0; i < element_count; ++i) {
                node *new_node = new node(array[i]);
                ptrs[i] = new_node;
            }
            auto n = build_t(ptrs, nullptr, 0, element_count);
            delete[] ptrs;  // Deallocate only the array, not nodes themselves.
            return n;
        }

        /*
         * Insert new node containing inserted value (if value already exists
         * in the tree, a new node with duplicity will be added to the right
         * subtree of a previous value). Then, the tree is traversed upwards,
         * while updating subtree sizes along the way and tracking for the
         * highest unbalanced node, if such exists. Then, the highest unbalanced
         * node's subtree is rebuild to perfectly balanced analogy.
         */
        node *insert(const T &val) {
            node *node_to;
            node **insertion_place;
            std::tie(node_to, insertion_place) = insert_find(val);
            // Firstly, trivially insert the new node, possibly violating the tree invariant.
            // If we inserted the very first node in the tree, we end.
            node *to_insert = new node(val);
            statistics(true, false);
            if (trivial_insert(node_to, insertion_place, to_insert) == tree)
                return tree;
            // Update subtree sizes above inserted node, track unbalanced nodes.
            node *highest_unbalanced = update_subtree_sizes_from_node_to_root(node_to);
            // If there is any unbalanced node, the following condition is true
            // and by rebuilding only highest_unbalanced we balance the whole tree at once.
            if (highest_unbalanced) {
                int_least32_t size = highest_unbalanced->subtree_size;
                // Rebuild the ptr subtree.
                node **array = new node *[size];
                // For rebuilding, sorted nodes are necessary.
                sort_tree(array, highest_unbalanced);
#ifndef NDEBUG
                assert_correct_sorting(array, size);
#endif
                // Correctly hang rebuilt tree.
                node **address_of_ptr_to_highest_unbalanced = highest_unbalanced->parent_ptr_address();
                if (!address_of_ptr_to_highest_unbalanced)
                    address_of_ptr_to_highest_unbalanced = &tree;
                *address_of_ptr_to_highest_unbalanced = build_t(array, highest_unbalanced->parent, 0, size);

                delete[] array;
            }
            return *insertion_place;
        }

        template<typename Node, typename Lambda>
        void inorder_dfs(Node *n, Lambda &&f) {
            if (!n)
                return;
            inorder_dfs(n->right, f);
            f(n);
            inorder_dfs(n->left, f);
        }

        template<typename Node, typename Lambda>
        void postorder_dfs(Node *n, Lambda &&f) {
            if (!n)
                return;
            postorder_dfs(n->right, f);
            postorder_dfs(n->left, f);
            f(n);
        }

    protected:
        // Returns address of place where to insert pointer to inserted node +
        // pointer to the newly parental node.
        // Ignores duplicities, always finds a place for insertion
        auto insert_find(const T &node_data) noexcept {
            node *ptr = tree;
            node *pre_ptr = nullptr;
            node **insertion_place = nullptr;
            while (ptr != nullptr) {
                statistics(true, false);
                const auto &current_data = ptr->value;
                if (current_data <= node_data) {
                    insertion_place = &(ptr->right);
                    pre_ptr = ptr;
                    ptr = ptr->right;
                } else {
                    insertion_place = &(ptr->left);
                    pre_ptr = ptr;
                    ptr = ptr->left;
                }
            }
            return std::make_tuple(pre_ptr, insertion_place);
        }

        /*
         * Goes from the from node to the root node, updating subtree sizes
         * along the way up after node insertion by adding 1 and track highest
         * unbalanced node in the tree (the closest one to the root).
         */
        node *update_subtree_sizes_from_node_to_root(node *from) noexcept {
            node *ptr = from;
            node *highest_unbalanced = nullptr;
            while (ptr) {
                statistics(true, false);
                ++(ptr->subtree_size);
                int_least32_t r_size = (ptr->right) ? ptr->right->subtree_size : 0;
                int_least32_t l_size = (ptr->left) ? ptr->left->subtree_size : 0;
                int_least32_t size = ptr->subtree_size;
                if (r_size > alpha * size || l_size > alpha * size) {
                    // Update highest unbalanced node.
                    highest_unbalanced = ptr;
                }
                ptr = ptr->parent;
            }
            return highest_unbalanced;
        }

        /*
         * Convenience method -- place node and update
         * parent pointer, solve inserting into an empty tree.
         */
        node *trivial_insert(
                node *parent,
                node **insertion_place_within_parent,
                node *node_to_insert_under_parent) {
            // There is no node in the whole tree.
            if (!insertion_place_within_parent) {
                tree = node_to_insert_under_parent;
            } else {
                *insertion_place_within_parent = node_to_insert_under_parent;
                node_to_insert_under_parent->parent = parent;
            }
            ++elements_count;
            return node_to_insert_under_parent;
        }

        /*
         * Sort tree under root_of_tree_to_sort, return out_sorted_array with
         * pointers to nodes in sorted order (from smallest to largest values).
         */
        void sort_tree(node **out_sorted_array, node *root_of_tree_to_sort) {
            int_least32_t idx = 0;
            inorder_dfs(root_of_tree_to_sort, [=, &idx](auto n) {
                this->statistics(true, false);
                out_sorted_array[idx] = n;
                ++idx;
            });
        }

#ifndef NDEBUG

        /*
         * Sort a copy of the array again with STL sort function and compare results.
         */
        void assert_correct_sorting(node **sorted_array_to_check, int_least32_t array_size) {
            node **test_array = new node *[array_size];
            std::memcpy(test_array, sorted_array_to_check, array_size * sizeof(node *) / sizeof(unsigned char));
            std::sort(test_array, test_array + array_size, node_comparator);
            for (int i = 0; i < array_size; ++i) {
                assert(test_array[i] == sorted_array_to_check[i]);
            }
            delete[] test_array;
        }

#endif
    };

}


#endif //DATA_STRUCTURES_BBALPHA_TREE_H
