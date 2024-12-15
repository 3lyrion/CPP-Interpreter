#pragma once

#include "Precompiled.h"

template <typename T>
class BinTree
{
	struct Node
	{
		bool visited = false;

		Node* back = nullptr;
		Node* left = nullptr;
		Node* right = nullptr;

		T value = T(0);

		Node(T const& value_, Node* back_ = nullptr, Node* left_ = nullptr, Node* right_ = nullptr) :
			value (value_),
			back  (back_),
			left  (left_),
			right (right_) { }

		Node(auto&&... args, Node* back_ = nullptr, Node* left_ = nullptr, Node* right_ = nullptr) :
			value (forward<decltype(args)>(args)...),
			back  (back_),
			left  (left_),
			right (right_) { }

		void copyBranchesTo(Node& node)
		{
			node.back = back;
			node.left = left;
			node.right = right;
		}
	};

public:
	BinTree(T const& value)
	{
		push(m_node, value);
	}

	BinTree(auto&&... args)
	{
		emplace(m_node, forward<decltype(args)>(args)...);
	}

	~BinTree()
	{
		clear();
	}

	void clear()
	{
		if (!m_node) return;

		/*for (size_t i = 0; i < m_size; i++)
		{
			while (m_node->left)
				m_node = m_node->left;

			while (m_node->right)
				m_node = m_node->right;

			auto node = m_node;
			m_node = node->back;

			util::dispose(&node);
		}*/
	}

	void push_left(T const& value)
	{
		push(m_node->left, value);
		
		m_node->left->back = m_node;
	}

	void push_right(T const& value)
	{
		push(m_node->right, value);
		
		m_node->right->back = m_node;
	}

	template <typename... Args>
	constexpr T& emplace_left(Args&&... args)
	{
		emplace(m_node->left, forward<Args>(args)...);
		
		m_node->left->back = m_node;

		return m_node->left->value;
	}

	template <typename... Args>
	constexpr T& emplace_right(Args&&... args)
	{
		emplace(m_node->right, forward<Args>(args)...);
		
		m_node->right->back = m_node;

		return m_node->right->value;
	}

	T const& get() const
	{
		return m_node->value;
	}

	void pop()
	{
		auto b = m_node->back;
		auto l = m_node->left;
		auto r = m_node->right;

		if (b)
		{
			if (b->left) b->left = nullptr;
			else         b->right = nullptr;
		}

		if (l)
			l->back = nullptr;

		if (r)
			r->back = nullptr;

		util::dispose(&m_node);
	}

	void begin()
	{
		while (m_node->back)
			m_node = m_node->back;
	}

	bool back()
	{
		if (m_node->back)
		{
			m_node = m_node->back;

			return true;
		}

		return false;
	}

	bool left()
	{
		if (m_node->left)
		{
			m_node = m_node->left;

			return true;
		}

		return false;
	}

	bool right()
	{
		if (m_node->right)
		{
			m_node = m_node->right;

			return true;
		}

		return false;
	}

	void bypass(bool print = true)
	{
		vector<T> out;

		size_t count = 0;
		size_t depth = 0;

		auto up = [&, this](size_t num = 1)
		{
			for (size_t i = 0; i < num; i++)
			{
				if (back())
					depth--;

				else break;
			}
		};

		auto visit = [&, this]
		{
			m_node->visited = true;

			count++;

			for (size_t i = 1; i < depth; i++)
			//	cout << "--------";
				cout << '\t';

			if (count == 1)
				cout << get() << '\n';
			else
			{
				wcout << L"╙────── ";
				cout << get() << '\n';
				//cout << "------ " << get() << '\n';
			}
		};

		begin();
		visit();

		while (count != m_size)
		{
			int ret = 0;

			if (!m_node->left)
				ret++;

			while (left())
			{
				if (m_node->visited)
				{
					depth++;
					up();

					ret++;

					break;
				}

				depth++;
				visit();
			}

			up();

			if (!m_node->right)
				ret++;

			while (right())
			{
				if (m_node->visited)
				{
					depth++;
					up();

					ret++;

					break;
				}

				depth++;
				visit();
			}

			if (ret == 2)
				up();
		}
	}

private:
	Node* m_node = nullptr;

	size_t m_size = 0;

	void push(Node*& node, T const& value)
	{
		if (node)
			node->value = value;

		else
		{
			m_size++;
			node = new Node(value);
		}
	}

	template <typename... Args>
	constexpr void emplace(Node*& node, Args&&... args)
	{
		if (node)
		{
			auto b = node->back;
			auto l = node->left;
			auto r = node->right;

			delete node;

			node = new Node(forward<Args>(args)..., b, l, r);
		}

		else
		{
			node = new Node(forward<Args>(args)...);

			m_size++;
		}
	}
};