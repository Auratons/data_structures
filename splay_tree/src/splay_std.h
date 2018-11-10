#ifndef SPLAY_STD_HPP
#define SPLAY_STD_HPP

#include <exception>

#include "splay.h"

namespace tp {

	template<typename T>
	class splay_std : public splay_tree<T> {
	using node_t = node<T>;
	public:

		void insert(const T& data) override {
			auto res = splay_tree<T>::insert_find(data);
			if (res.second != nullptr) {
				throw std::exception("Inserted node already exists in the tree.");
			}

			auto ptr = new node_t(data);
			if (!res.first) {
				this->fst_node = ptr;
				return;
			}
			if (data < res.first->data) {
				res.first->left = ptr;
				res.first->left->parent = res.first;
			}
			else {
				res.first->right = ptr;
				res.first->right->parent = res.first;
			}

			this->fst_node = splay(ptr);
		}

	private:

		auto zig(node_t* x, direction dir) {
			return splay_tree<T>::simple_rot(x, dir);
		}

		auto zig_zig(node_t* x, direction dir) {
			if (x->parent && x->parent->parent) {
				switch (dir) {
				case direction::right:
					x = splay_tree<T>::simple_rot(x->parent, direction::left);
					x = splay_tree<T>::simple_rot(x->left, direction::left);
					break;
				case direction::left:
					x = splay_tree<T>::simple_rot(x->parent, direction::right);
					x = splay_tree<T>::simple_rot(x->right, direction::right);
					break;
				default:
					throw std::exception("Direction should be left or right, this should never happen.");
				}
			}
			return x;
		}

		auto zig_zag(node_t* x, direction dir) {
			if (x->parent && x->parent->parent) {
				switch (dir) {
				case direction::right:
					x = splay_tree<T>::simple_rot(x, direction::right);
					x = splay_tree<T>::simple_rot(x, direction::left);
					break;
				case direction::left:
					x = splay_tree<T>::simple_rot(x, direction::left);
					x = splay_tree<T>::simple_rot(x, direction::right);
					break;
				default:
					throw std::exception("Direction should be left or right, this should never happen.");
				}
			}
			return x;
		}

		node_t* splay(node_t* ptr) {
			for (;;) {
				auto ptr_dir = ptr->son_side();
				if (!ptr->parent->parent) {
					// Decide in which direction we should rotate right under root.
					ptr = zig(ptr, ptr_dir);
				}
				else {
					auto ptrparent_dir = ptr->parent->son_side();
					if (ptr_dir == direction::left) {
						if (ptrparent_dir == direction::left)
							ptr = zig_zig(ptr, direction::right);
						else
							ptr = zig_zag(ptr, direction::left);
					}
					else {
						if (ptrparent_dir == direction::left)
							ptr = zig_zag(ptr, direction::right);
						else
							ptr = zig_zig(ptr, direction::left);
					}
				}
				if (!ptr->parent)
					break;
			}
			return ptr;
		}
	};

}
#endif // SPLAY_STD_HPP
