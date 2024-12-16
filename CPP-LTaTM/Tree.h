#pragma once

#include "Precompiled.h"

template <typename T>
class Tree
{
	struct Node
	{
		bool visited = false;

		Node* back = nullptr;

		list<Node*> childs;

		T value = T(0);

		Node(T const& value_, Node* back_ = nullptr) :
			value (value_),
			back  (back_) { }

		constexpr Node(auto&&... args, Node* back_ = nullptr) :
			value (forward<decltype(args)>(args)...),
			back  (back_) { }
	};

public:
	Tree() = default;

	Tree(T const& value)
	{
		init(value);
	}

	constexpr Tree(auto&&... args)
	{
		init(forward<decltype(args)>(args)...);
	}

	~Tree()
	{
		clear();
	}

	void init(T const& value)
	{
		if (m_node) return;

		m_node = new Node(value);
		m_size++;
	}

	constexpr void init(auto&&... args)
	{
		if (m_node) return;

		m_node = new Node(forward<decltype(args)>(args)...);
		m_size++;
	}

	void clear()
	{
		if (!m_node) return;

		// ??? 
	}

	void push_back(T const& value)
	{
		auto& childs = m_node->childs;
		childs.emplace_back(new Node(value));

		childs.back()->back = m_node;

		m_size++;
	}

	void push_back(Tree&& tree)
	{
		tree.m_node->back = m_node;

		auto& childs = m_node->childs;
		childs.emplace_back(tree.m_node);

		m_size += tree.m_size;
	}

	void push_front(T const& value)
	{
		auto& childs = m_node->childs;
		childs.emplace_front(new Node(value));

		childs.front()->back = m_node;

		m_size++;
	}

	void push_front(Tree&& tree)
	{
		tree.m_node->back = m_node;

		auto& childs = m_node->childs;
		childs.emplace_front(tree.m_node);

		m_size += tree.m_size;
	}

	constexpr /*T&*/ void emplace_back(auto&&... args)
	{
		auto& childs = m_node->childs;
		auto& b = childs.emplace_back(new Node(forward<decltype(args)>(args)...));

		b->back = m_node;
		m_size++;

		/*return b->value;*/
	}

	constexpr /*T&*/ void emplace_front(auto&&... args)
	{
		auto& childs = m_node->childs;
		auto& f = childs.emplace_front(new Node(forward<decltype(args)>(args)...));

		f->back = m_node;
		m_size++;

		/*return f->value;*/
	}

	T const& get() const
	{
		return m_node->value;
	}
	
	void begin()
	{
		while (m_node->back)
			m_node = m_node->back;
	}

	bool go_back()
	{
		if (m_node->back)
		{
			m_node = m_node->back;

			return true;
		}

		return false;
	}

	void front()
	{
		m_node = m_node->childs.front();
	}

	void back()
	{
		m_node = m_node->childs.back();
	}

	bool select(uint32_t index)
	{
		auto& childs = m_node->childs;

		if (index < childs.size())
		{
			auto it = childs.begin();
			advance(it, index);

			m_node = *it;

			return true;
		}

		return false;
	}

	size_t size() const
	{
		return m_size;
	}

	bool empty() const
	{
		return m_size == 0;
	}
	
	void print()
	{
		begin();
		visit();

		seek();
	}
	
private:
	Node* m_node = nullptr;

	size_t m_size = 0;

private:
	size_t _count = 0;
	size_t _depth = 0;

	void visit()
	{
		m_node->visited = true;

		_count++;

		for (size_t i = 1; i < _depth; i++)
			cout << '\t';

		if (_count == 1)
			cout << get() << '\n';

		else
		{
			wcout << L"╙────── ";
			cout << get() << '\n';
		}
	}

	void seek()
	{
		_depth++;

		for (auto& ch : m_node->childs)
		{
			m_node = ch;
			visit();

			seek();
		}

		_depth--;
	}
};