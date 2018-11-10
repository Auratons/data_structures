#ifndef SPLAY_HPP
#define SPLAY_HPP

#include <utility>
#include <stack>

namespace tp {

	// An enum class used for describing of descendant type --
	// if a given node is right or left son of its father.
	enum class direction {
		left,
		right
	};

	// A tree consists of nodes that bear its pointer structure.
	template<typename T>
	struct node {
		explicit node(const T& node_data) : parent(nullptr), left(nullptr), right(nullptr), data(node_data) {}
		node* parent;
		node* left;
		node* right;
		T data;

		// Returns if the node is right or left son of its father.
		direction son_side() {
			if (parent == nullptr)
				throw std::exception("This is a root of a tree and thus cannot be a son.");
			return (parent->left == this) ? direction::left : direction::right;
		}
	};

	// A splay tree base class.
	template<typename T>
	class splay_tree {
	using node_t = node<T>;
	public:
		node_t* fst_node;

		splay_tree() : fst_node(nullptr) {}

		virtual void insert(const T& data) = 0;

		auto find(const T& node_data) noexcept {
			auto res = insert_find(node_data);
			return res.second;
		}

		virtual ~splay_tree() {
			if (!fst_node) return;

			std::stack<node_t*> stack;
			stack.push(fst_node);
			while (!stack.empty())
			{
				node_t* node = stack.top();
				stack.pop();

				if (node->right)
					stack.push(node->right);
				if (node->left)
					stack.push(node->left);
				delete node;
			}
		}

	protected:
		// Stop one at last but one node before final find -> can be used for insert and normal find.
		auto insert_find(const T& node_data) noexcept {
			node_t* ptr = fst_node;
			node_t* pre_ptr = nullptr;
			while (ptr != nullptr) {
				const auto& current_data = ptr->data;
				if (current_data == node_data) {
					std::pair<node_t*, node_t*> pair(pre_ptr, ptr);
					return std::move(pair);
				}
				if (current_data < node_data) {
					pre_ptr = ptr;
					ptr = ptr->right;
				}
				else {
					pre_ptr = ptr;
					ptr = ptr->left;
				}
			}
			std::pair<node_t*, node_t*> pair(pre_ptr, nullptr);
			return std::move(pair);
		}

		auto simple_rot(node<T>* x, direction dir) {
			node<T>* b;
			node<T>* y;
			if (dir == direction::left) {
				b = x->right;
				y = x->parent;
				x->right = y;
				if (y) y->left = b;
			}
			else {
				b = x->left;
				y = x->parent;
				x->left = y;
				if (y) y->right = b;
			}
			if (y && y->parent) {
				if (y->son_side() == direction::left)
					y->parent->left = x;
				else
					y->parent->right = x;
			}
			x->parent = (y) ? y->parent : nullptr;
			if (b) b->parent = y;
			if (y) y->parent = x;

			return x;
		}
	};

}
#endif // SPLAY_HPP