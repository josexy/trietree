
#pragma once

#include <math.h>
#include <time.h>

#include <algorithm>
#include <iostream>
using namespace std;

template <class KeyType, class ValueType>
struct element_t {
    using key_type = KeyType;
    using value_type = ValueType;
    using reference_type = value_type&;
    using const_reference_value_type = const value_type&;

    key_type first;
    value_type second;

    element_t() {}
    explicit element_t(key_type _key, const_reference_value_type _value)
        : first(_key), second(_value) {}

    operator std::pair<key_type, value_type>() {
        return std::make_pair(first, second);
    }
    bool operator==(const element_t& other) { return other.first == first; }
    bool operator!=(const element_t& other) { return other.first != first; }
};
template <class KeyType, class ValueType>
struct skipNode {
    using element_type = element_t<KeyType, ValueType>;
    using element_reference_type = element_type&;
    using element_const_reference_type = const element_reference_type;
    using element_pointer_type = element_type*;

    using self_type = skipNode<KeyType, ValueType>;
    using self_pointer_type = self_type*;
    using self_reference_type = self_type&;

    // element
    element_type element;
    // next nodes
    self_pointer_type* next = nullptr;

    skipNode() {}
    explicit skipNode(KeyType key, const ValueType& value, int size) {
        element.first = key;
        element.second = value;
        next = new self_pointer_type[size];
    }
    explicit skipNode(element_const_reference_type theElement, int size) {
        element = theElement;
        next = new self_pointer_type[size];
    }
};

template <class KeyType, class ValueType>
class skiplist {
   public:
    using element_type = typename skipNode<KeyType, ValueType>::element_type;
    using element_reference_type =
        typename skipNode<KeyType, ValueType>::element_reference_type;
    using element_const_reference_type =
        typename skipNode<KeyType, ValueType>::element_const_reference_type;
    using element_pointer_type =
        typename skipNode<KeyType, ValueType>::element_pointer_type;

    using key_type = KeyType;
    using const_key_type = const key_type&;
    using value_type = ValueType;
    using reference_type = value_type&;
    using const_reference_value_type = const value_type&;

    using node_type = skipNode<KeyType, value_type>;
    using node_pointer_type = node_type*;
    using node_reference_type = node_type&;
    using node_const_reference = const node_reference_type;
    using node_pointer_reference_type = node_pointer_type&;

    static constexpr key_type TAIL_INFINITY_KEY =
        numeric_limits<key_type>::max();

   public:
    explicit skiplist(float prob = 0.5f,
                      KeyType tailLargeKey = TAIL_INFINITY_KEY,
                      int max_level = 10, int number_node = -1) {
        srand(time(nullptr));
        m_prob = prob;
        m_size = 0;

        // 计算最大链表层数
        if (number_node != -1)
            m_maxLevel = MaxLevel(number_node, prob);
        else
            m_maxLevel = max_level;

        m_curMaxLevel = 0;

        // 初始化尾节点
        m_tailKey = tailLargeKey;
        element_type tailPair(m_tailKey, 0);
        m_tailNode = new node_type(tailPair, 0);

        // 初始化头结点
        m_headNode = new node_type(tailPair, m_maxLevel);
        for (int i = 0; i < m_maxLevel; i++) {
            m_headNode->next[i] = m_tailNode;
        }
        // 缓存前驱节点
        m_forwardNodes = new node_pointer_type[m_maxLevel];
    }
    ~skiplist() {
        while (m_headNode != m_tailNode) {
            auto x = m_headNode->next[0];
            delete m_headNode;
            m_headNode = x;
        }
        delete m_tailNode;
        delete[] m_forwardNodes;
    }

    class iterator {
       public:
        explicit iterator(node_pointer_type x) : it(x) {}

        operator element_type() { return it->element; }
        element_type& operator*() { return (it->element); }
        element_type* operator->() { return &it->element; }

        iterator& operator++() {
            it = it->next[0];
            return *this;
        }
        iterator operator++(int) {
            iterator old(it);
            it = it->next[0];
            return old;
        }
        bool operator==(const iterator& iter) { return iter.it == it; }
        bool operator!=(const iterator& iter) { return iter.it != it; }

       private:
        node_pointer_type it;
    };
    iterator begin() { return iterator(m_headNode->next[0]); }
    iterator end() { return iterator(m_tailNode); }
    using const_iterator = iterator;

    node_pointer_type insert(const_key_type key,
                             const_reference_value_type value) {
        // 随机生成索引节点层数 1<=level<=m_maxLevel
        int level = random_level();
        if (level > m_curMaxLevel) {
            m_curMaxLevel = level;
        }
        node_pointer_type pNode = search(key);
        if (pNode->element.first == key) {
            pNode->element.second = value;
            return pNode;
        }
        // 此时已经保存了合适的前驱节点m_forwardNodes
        // 创建一个具有level层的节点
        node_pointer_type pNewNode = new node_type(key, value, level);

        // 建立索引节点
        for (int i = level - 1; i >= 0; --i) {
            pNewNode->next[i] = m_forwardNodes[i]->next[i];
            m_forwardNodes[i]->next[i] = pNewNode;
        }
        m_size++;
        return pNewNode;
    }
    node_pointer_type insert(element_const_reference_type element) {
        return insert(element.first, element.second);
    }

    bool erase(const_key_type key) {
        // 不符合的key
        if (key > m_tailKey) return false;
        node_pointer_type pNode = search(key);
        // 不存在
        if (pNode->element.first != key) return false;
        // 更新跳表链表结构
        for (int i = m_curMaxLevel - 1; i >= 0; --i) {
            // 此处 m_forwardNodes[i]->next[i] 可能不是 pNode
            if (m_forwardNodes[i]->next[i] == pNode)
                m_forwardNodes[i]->next[i] = pNode->next[i];
        }
        // 维护当前最大层级数
        // 当删除一个具有最大层级的节点时，可能会导致
        // m_headNode->next[m_maxLevel-1]=m_tailNode，那么此时需要降低层级
        while (m_curMaxLevel - 1 > 0 &&
               m_headNode->next[m_curMaxLevel - 1] == m_tailNode) {
            m_curMaxLevel--;
        }
        delete pNode;
        m_size--;
        return true;
    }

    node_pointer_type find_node(const_key_type key) {
        node_pointer_type x = search(key);
        if (x)
            return x->element.first == key ? x : nullptr;
        else
            return nullptr;
    }
    // 返回一个迭代器
    iterator find(const_key_type key) {
        auto x = find_node(key);
        if (x == nullptr) return end();
        return iterator(x);
    }

    element_pointer_type find_element(const_key_type key) {
        node_pointer_type x = find_node(key);
        if (!x) return nullptr;
        return &x->element;
    }

    /*  搜索并把遇到的最后一个节点保存下来 */
    node_pointer_type search(const_key_type key) {
        node_pointer_type forwardNode = m_headNode;
        // 外层循环: 不断的指向下一层
        for (int i = m_curMaxLevel - 1; i >= 0; --i) {
            // 内层循环: 指向当前层链表的下一个节点
            while (forwardNode->next[i] != m_tailNode &&
                   forwardNode->next[i]->element.first < key) {
                forwardNode = forwardNode->next[i];
            }
            // 保存前驱节点指针
            m_forwardNodes[i] = forwardNode;
        }
        // 最终回到第0层
        return forwardNode->next[0];
    }

    int size() { return m_size; }

    void output() {
        if (m_size <= 0) return;
        for (int i = m_curMaxLevel - 1; i >= 0; i--) {
            node_pointer_type cur = m_headNode->next[i];
            cout << "head" << i << " => ";
            while (cur != m_tailNode) {
                if (cur == m_headNode->next[i]) {
                    cout << cur->element.first << ":" << cur->element.second;
                } else {
                    cout << "->" << cur->element.first << ":"
                         << cur->element.second;
                }
                cur = cur->next[i];
            }
            cout << endl;
        }
    }

    void output_bottom() {
        node_pointer_type x = m_headNode->next[0];
        while (x != m_tailNode) {
            cout << "->" << x->element.first << ":" << x->element.second;
            x = x->next[0];
        }
        cout << endl;
    }

    const_reference_value_type operator[](const_key_type key) const {
        return find_node(key)->element.second;
    }
    reference_type operator[](const_key_type key) {
        auto x = find_node(key);
        if (x == nullptr) return insert(key, value_type())->element.second;
        return x->element.second;
    }

   protected:
    // 简单随机生成索引节点层数 1<=level<=m_maxLevel
    // int random_level(){
    // 	int level = rand() % m_maxLevel + 1;
    // 	return level;
    // }
    int random_level() {
        int level = 1;
        while (((double)(rand()) / RAND_MAX) < (m_prob) && level < m_maxLevel) {
            level++;
        }
        return level;
    }
    int MaxLevel(int numberOfnode, float prob) {
        return (int)(ceil(logf((float)numberOfnode) / logf((float)1 / prob)));
    }

   private:
    // 随机概率
    float m_prob;
    key_type m_tailKey;
    // 最大索引层数
    int m_maxLevel;
    // 当前某个节点的最大索引层数 1 <= m_curMaxLevel <= m_maxLevel
    int m_curMaxLevel;
    int m_size;
    // 头结点
    node_pointer_type m_headNode;
    // 尾节点/哨兵节点
    node_pointer_type m_tailNode;
    // 用于保存前驱节点
    node_pointer_type* m_forwardNodes;
};
