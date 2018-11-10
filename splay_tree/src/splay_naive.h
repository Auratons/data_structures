#ifndef SPLAY_NAIVE_HPP
#define SPLAY_NAIVE_HPP

#include <exception>
#include "splay.h"

namespace tp {

	template<typename T>
	class splay_naive : public splay_tree<T> {
	using node_t = node<T>;
	public:

		void insert(const T& data) override {
			auto res = splay_tree<T>::insert_find(data);
			if (res.second) {
				throw std::exception("Inserted node already exists in the tree.");
			}

			auto ptr = new node_t(data);
			if (!res.first) {
				this->fst_node = ptr;
				return;
			}
			if (data < res.first->data) {
				res.first->left = ptr;
				ptr->parent = res.first;
			}
			else {
				res.first->right = ptr;
				ptr->parent = res.first;
			}

			this->fst_node = splay(ptr);

		}

	private:

		node_t* splay(node_t* ptr) {
			for (;;) {
				auto direction = ptr->son_side();
				ptr = splay_tree<T>::simple_rot(ptr, direction);
				if (!ptr->parent)
					break;
			}
			return ptr;
		}
	};

}
#endif // SPLAY_NAIVE_HPP