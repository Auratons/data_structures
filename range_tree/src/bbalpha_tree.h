#ifndef DATA_STRUCTURES_BBALPHA_TREE_H
#define DATA_STRUCTURES_BBALPHA_TREE_H

#include <stdint.h>
#include <cstddef>
#include <tuple>  // Usage of tuple and tie can be easily avoided. Used for cleaner & readable code.
#include <algorithm>  // For sorting (should be useless).

namespace rt {

    template<typename T>
    class bbalpha {
    public:
        class node {
        public:
            node* right = nullptr;
            node* left = nullptr;
            node* parent = nullptr;
            int_least32_t subtree_size = 1;
            T value = T();

            node() = default;
            node(const node* r, const node* l) : right(r), left(l) {}
            node(const node* r, const node* l, const T& v) : right(r), left(l), value(v) {}
            explicit node(const T& v) : value(v) {}
            node(const node& other) = delete;
            node(node&& other) = delete;
            node& operator=(const node& other) = delete;
            node& operator=(node&& other) = delete;

            /*
             * Simple recursive destructor.
             */
            ~node() {
                delete right;
                delete left;
            }
        };
    public:
        node* tree = nullptr;
        const float alpha;
        int_least32_t elements_count = 0;

        explicit bbalpha(float a) : alpha(a) {}

        /*
         * Simple recursive tree building. Array must be sorted and contain
         * element_count number of elements.
         */
        node* build_t(node** array, const int_least32_t begin, const int_least32_t end) {
            if (!array || begin >= end)
                return nullptr;
            int_least32_t half = begin + int_least32_t((end - begin) / 2);
            node* n = array[half];
            n->subtree_size = end - begin;
            n->right = build_t(array, begin, half);
            if (n->right) n->right->parent = n;
            n->left = build_t(array, half + 1, end);
            if (n->left) n->left->parent = n;
            return n;
        }


        /*
         * Array must be sorted and contain element_count number of elements.
         */
        node* build(T array[], const int_least32_t element_count) {
            auto ptrs = new node*[element_count];
            for (int i = 0; i < element_count; ++i) {
                ptrs[i] = new node(array[i]);
            }
            auto n = build_t(ptrs, 0, element_count);
            delete[] ptrs;  // Deallocate only the array, not node themselves.
            return n;
        }

        node* insert(const T& val) {
            node* node_to;
            node*  maybe_found;
            node** insertion_place;
            std::tie(node_to, insertion_place, maybe_found) = insert_find(val);
            if (!tree) {
                tree = new node(val);
                ++elements_count;
                return tree;
            }
            if (!maybe_found) {
                *insertion_place = new node(val);
                (*insertion_place)->parent = node_to;
                ++elements_count;

                // Update subtree sizes above inserted node, possibly repair tree.
                node* ptr = node_to;
                while (ptr) {
                    ++(ptr->subtree_size);
                    int_least32_t r_size = (ptr->right) ? ptr->right->subtree_size : 0;
                    int_least32_t l_size = (ptr->left) ? ptr->left->subtree_size : 0;
                    int_least32_t size = ptr->subtree_size;
                    if (r_size > alpha * size || l_size > alpha * size) {
                        // Rebuild the ptr subtree.
                        node** array = new node*[size];
                        int_least32_t idx = 0;
                        inorder_dfs(ptr, [&idx, &array](auto n){
                            array[idx] =n;
                            ++idx;
                        });
                        struct {
                            bool operator()(node* a, node* b) const {
                                return a->value < b->value;
                            }
                        } comparator;
                        std::sort(array, array + size, comparator);
                        node** ptr_parent_ptr = nullptr;
                        node* ptr_parent = ptr->parent;
                        if (ptr_parent)
                            ptr_parent_ptr = (ptr->parent->right == ptr) ? &(ptr->parent->right) : &(ptr->parent->left);
                        else
                            ptr_parent_ptr = &tree;
                        *ptr_parent_ptr = build_t(array, 0, size);  // Correctly hang rebuilt tree.
                        (*ptr_parent_ptr)->parent = ptr_parent;  // Correctly set parent to the top of the rebuilt tree.
                        ptr = ptr_parent;  // Ptr now correctly heads above just rebuilt tree.

                        delete[] array;
                    }
                    else
                        ptr = ptr->parent;
                }
            }
            return *insertion_place;
        }

        template<typename Lambda>
        void inorder_dfs(node* n, Lambda&& f) {
            if (!n)
                return;
            inorder_dfs(n->right, f);
            f(n);
            inorder_dfs(n->left, f);
        }

    private:
        // Stop one at last but one node before final find -> can be used for insert and normal find.
        auto insert_find(const T& node_data) noexcept {
            node* ptr = tree;
            node* pre_ptr = nullptr;
            node** insertion_place = nullptr;
            while (ptr != nullptr) {
                const auto& current_data = ptr->value;
                if (current_data == node_data) {
                    return std::make_tuple(pre_ptr, insertion_place, ptr);
                }
                if (current_data > node_data) {
                    insertion_place = &(ptr->right);
                    pre_ptr = ptr;
                    ptr = ptr->right;
                }
                else {
                    insertion_place = &(ptr->left);
                    pre_ptr = ptr;
                    ptr = ptr->left;
                }
            }
            return std::make_tuple(pre_ptr, insertion_place, ptr);
        }
    };

}


#endif //DATA_STRUCTURES_BBALPHA_TREE_H
