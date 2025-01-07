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

		explicit Node(T const& value_, Node* back_ = nullptr) :
			value (value_),
			back  (back_) { }

		constexpr explicit Node(auto&&... args) :
			value (forward<decltype(args)>(args)...) { }
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

		begin();

		auto head = m_node;

		_clear();

		//cout << head->value << '\n';

		delete head;
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

		tree.m_node = nullptr;

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

		tree.m_node = nullptr;

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

	size_t m_index = 0;

	size_t m_size = 0;

private:
	void _clear()
	{
		for (auto& ch : m_node->childs)
		{
			m_node = ch;

			_clear();

			//cout << ch->value << '\n';

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
		/*auto& childs = node->childs;
		auto it = remove_if(childs.rbegin(), childs.rend(),
			[this, node, &values](auto ch)
			{
				_exclude(ch, values);

				if (values.contains(ch->value))
				{
					for (auto _ch : ch->childs)
					{
						_ch->back = node;
						node->childs.push_back(_ch);
					}

					delete ch;
					
					return true;
				}

				return false;
			}
		);

		childs.erase(it.base(), childs.end());*/

		/*for (auto it = node->childs.rbegin(); it != node->childs.rend(); )
		{
			auto ch = *it;

			_exclude(ch, values);

			if (values.contains(ch->value))
			{
				for (auto _ch : ch->childs)
				{
					_ch->back = node;
					node->childs.push_back(_ch);
				}

				delete ch;

				auto _it = it;
				advance(_it, 1ull);
					
				it = decltype(it)(node->childs.erase(_it.base()));
			}

			else
				++it;
		}*/

		node->childs.remove_if(
			[this, node, &values](auto ch)
			{
				_exclude(ch, values);

				if (values.contains(ch->value))
				{
					/*auto& childs = ch->childs;
					for_each(childs.rbegin(), childs.rend(),
						[node](auto _ch)
						{
							_ch->back = node;
							node->childs.push_front(_ch);
						}
					);*/

					for (auto _ch : ch->childs)
					{
						_ch->back = node;



					//	node->childs.push_back(_ch);
					}
					auto& childs = node->childs;
					auto it = find(childs.begin(), childs.end(), ch);
					//it = childs.erase(it);
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