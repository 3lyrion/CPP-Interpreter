#pragma once

#include <Precompiled.h>

template <typename T>
class Tree
{
public:
	struct Node
	{
		bool		visited{}; // only for printing the tree
		Node*		back{};
		list<Node*> childs;
		T			value{};

		explicit Node(T const& value_, Node* back_ = nullptr) :
			value (value_),
			back  (back_) { }

		constexpr explicit Node(auto&&... args) :
			value (forward<decltype(args)>(args)...) { }
	};

	Tree() = default;

	explicit Tree(T const& value)
	{
		init(value);
	}

	constexpr explicit Tree(auto&&... args)
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
		if (m_node)
			return;

		m_node = new Node(forward<decltype(args)>(args)...);
		m_size++;
	}

	void clear()
	{
		if (!m_node)
			return;

		begin();

		auto head = m_node;

		_clear();

		delete head;
	}

	void push_back(T const& value)
	{
		m_node->childs.emplace_back(new Node(value))->back = m_node;
		m_size++;
	}

	void push_back(Tree&& tree)
	{
		tree.m_node->back = m_node;

		m_node->childs.push_back(tree.m_node);

		tree.m_node = nullptr;

		m_size += tree.m_size;
	}

	void push_front(T const& value)
	{
		m_node->childs.emplace_front(new Node(value))->back = m_node;
		m_size++;
	}

	void push_front(Tree&& tree)
	{
		tree.m_node->back = m_node;

		auto& childs = m_node->childs;
		childs.push_back(tree.m_node);

		tree.m_node = nullptr;

		m_size += tree.m_size;
	}

	constexpr void emplace_back(auto&&... args)
	{
		auto& b = m_node->childs.emplace_back(new Node(forward<decltype(args)>(args)...));
		b->back = m_node;
		m_size++;

		/*return b->value;*/
	}

	constexpr void emplace_front(auto&&... args)
	{
		auto& f = m_node->childs.emplace_front(new Node(forward<decltype(args)>(args)...));
		f->back = m_node;
		m_size++;

		/*return f->value;*/
	}

	void begin()
	{
		while (m_node->back)
			m_node = m_node->back;

		m_index = 0ull;
	}

	bool front()
	{
		if (m_node->back)
		{
			m_node  = m_node->back->childs.front();
			m_index = 0ull;
			return true;
		}

		return false;
	}

	bool back()
	{
		if (m_node->back)
		{
			auto& childs = m_node->back->childs;
			m_node = childs.back();
			m_index = childs.size() - 1ull;
			return true;
		}

		return false;
	}

	bool prev()
	{
		if (m_node->back && m_index != 0ull)
		{
			auto& childs = m_node->back->childs;
			auto it = childs.begin();
			advance(it, --m_index);
			m_node = *it;
			return true;
		}

		return false;
	}

	bool next()
	{
		if (m_node->back && m_index < m_node->back->childs.size() - 1ull)
		{
			auto& childs = m_node->back->childs;
			auto it = childs.begin();
			advance(it, ++m_index);
			m_node = *it;
			return true;
		}

		return false;
	}

	bool up()
	{
		if (m_node->back)
		{
			m_node = m_node->back;

			if (m_node->back)
			{
				auto& childs = m_node->back->childs;
				m_index = distance(childs.cbegin(), find(childs.cbegin(), childs.cend(), m_node));
			}

			else
				m_index = 0ull;

			return true;
		}

		return false;
	}

	bool down()
	{
		auto& childs = m_node->childs;
		if (!childs.empty())
		{
			m_node  = childs.front();
			m_index = 0ull;

			return true;
		}

		return false;
	}

	template <typename Hash>
	constexpr void exclude(unordered_set<T, Hash> const& values)
	{
		begin();

		_exclude(m_node, values);
	}

	T const& get() const
	{
		return m_node->value;
	}

	T const& get_back() const
	{
		if (!m_node->back)
			throw out_of_range("The node has no ancestor");

		return m_node->back->value;
	}

	Node* get_node() const
	{
		return m_node;
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
	Node*	m_node{};
	size_t	m_index{};
	size_t	m_size{};

private:
	void _clear()
	{
		for (auto ch : m_node->childs)
		{
			m_node = ch;

			_clear();

			delete ch;
		}
	}

	void _clear(Node* node)
	{
		node->childs.remove_if(
			[this](auto ch)
			{
				_clear(ch);

				delete ch;

				return true;
			}
		);
	}

	template <typename Hash>
	constexpr void _exclude(Node* node, unordered_set<T, Hash> const& values)
	{
		node->childs.remove_if(
			[this, node, &values](auto ch)
			{
				_exclude(ch, values);

				if (values.contains(ch->value))
				{
					for (auto _ch : ch->childs)
						_ch->back = node;

					auto& childs = node->childs;
					auto it = find(childs.begin(), childs.end(), ch);
					childs.insert(it, ch->childs.begin(), ch->childs.end());

					delete ch;

					return true;
				}

				return false;
			}
		);
	}

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

		if (_depth > 0)
			_depth--;
	}
};