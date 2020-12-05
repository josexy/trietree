#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

/*
 Trie 树支持以下操作：
* 1.对字符串进行排序：可以用 map红黑树 来存储节点而不是 unordered_map哈希表。
* 2.词频统计，在节点处添加一个 count 指示重复单词出现个数
* 3.前缀匹配，给定一个前缀字符串，然后找出所有包含该前缀字符串的所有单词
* 4.用于AC自动机的辅助数据结构
*
* 按照节点的存储结构，分为有序Trie和无序Trie
* */

template <class T, bool Sorted = true>
struct TrieNodeData {
    using self_type = TrieNodeData<T, Sorted>;
    using node_type =
        typename std::conditional<Sorted, std::map<T, self_type *>,
                                  std::unordered_map<T, self_type *>>::type;
    using iterator = typename node_type::iterator;
    using const_iterator = typename node_type::const_iterator;

    // 是否是叶子节点, 即是否是一个单词/序列
    bool isLeaf;
    // 统计重复单词/序列出现的个数
    int count;
    // 记录一个单词/序列的长度
    int length;
    // fail指针,用于构建ac自动机
    self_type *fail;
    // 孩子节点有序trie/无序trie
    node_type children;

    TrieNodeData(bool _leaf = false, int _count = 0, int _length = 0,
                 self_type *_fail = nullptr)
        : isLeaf(_leaf), count(_count), length(_length), fail(_fail) {}

    TrieNodeData(const self_type &other) {
        isLeaf = other.isLeaf;
        count = other.count;
        length = other.length;
        fail = nullptr;
    }
};

template <class T>
struct isChar {
    static constexpr bool value = false;
};
template <>
struct isChar<char> {
    static constexpr bool value = true;
};

template <class T = char, bool Sorted = true, T endMark = '\0'>
class TrieTree {
   public:
    using self_type = TrieTree<T, Sorted, endMark>;
    using self_reference_type = self_type &;
    using self_const_reference_type = const self_reference_type;

    using node_type = TrieNodeData<T, Sorted>;
    using node_pointer = node_type *;
    using node_pointer_ref = node_pointer &;
    using node_itertor = typename node_type::iterator;
    using node_const_iterator = typename node_type::const_iterator;

    using sequence_type =
        typename std::conditional<isChar<T>::value, std::string,
                                  std::vector<T>>::type;
    using const_reference_list_type = const sequence_type &;
    using reference_list_type = sequence_type &;

    using t_iterator = typename sequence_type::iterator;
    using ct_iterator = typename sequence_type::const_iterator;

    TrieTree() { root = new node_type(); }

    ~TrieTree() {
        clear(root);
        if (root) {
            delete root;
            root = nullptr;
        }
    }

    // trie树迭代器
    class iterator {
       public:
        iterator(typename std::vector<sequence_type>::iterator first) {
            it = first;
        }
        ~iterator() {}
        bool operator!=(iterator it) { return this->it != it.it; }
        iterator operator++(int) {
            iterator old(it);
            it++;
            return old;
        }
        iterator &operator++() {
            it++;
            return *this;
        }
        sequence_type &operator*() { return *it; }
        sequence_type *operator->() { return &*it; }

       private:
        typename std::vector<sequence_type>::iterator it;
    };

    iterator begin() {
        get();
        return iterator(__vcLs.begin());
    }
    iterator end() { return iterator(__vcLs.end()); }

    // clone TrieTree
    auto clone() {
        std::shared_ptr<self_type> self = std::make_shared<self_type>();
        for (auto s : get()) {
            self->insert(s);
        }
        return self;
    }

    // 将一个序列/单词插入到trie中
    node_pointer insert(ct_iterator first, ct_iterator last) {
        auto x = root;
        int length = last - first;
        for (; first != last; first++) {
            if (*first == endMark) continue;

            if (x->children.find(*first) == x->children.end()) {
                x->children[*first] = new node_type();
                // fail指针,用于构建AC自动机
                x->children[*first]->fail = root;
            }
            x = x->children[*first];
        }
        x->length = length;
        x->isLeaf = true;
        x->count++;
        return x;
    }
    template <class C>
    node_pointer insert(const C &c) {
        return insert(std::begin(c), std::end(c));
    }
    node_pointer insert(const_reference_list_type s) {
        return insert(s.begin(), s.end());
    }

    // 查找一个序列/单词是否存在
    node_pointer search(ct_iterator first, ct_iterator last) {
        if (root == nullptr) return nullptr;
        auto x = root;
        for (; first != last; first++) {
            if (*first == endMark) continue;
            if (x->children.find(*first) == x->children.end()) return nullptr;
            x = x->children[*first];
        }
        return x->isLeaf ? x : nullptr;
    }
    template <class C>
    node_pointer search(const C &c) {
        return search(std::begin(c), std::end(c));
    }

    node_pointer search(const_reference_list_type s) {
        return search(s.begin(), s.end());
    }

    // 前缀匹配
    node_pointer prefix_find(const_reference_list_type s) {
        if (root == nullptr) return nullptr;
        auto x = root;
        for (auto it = s.begin(); it != s.end(); it++) {
            if (*it == endMark) continue;
            if (x->children.find(*it) == x->children.end()) return nullptr;
            x = x->children[*it];
        }
        return x;
    }

    // 删除一个序列/单词
    bool erase(node_pointer_ref x, ct_iterator first, ct_iterator last) {
        if (!x) return false;
        if (*first != endMark) {
            if (x->children.find(*first) != x->children.end()) {
                if (erase(x->children[*first], next(first, 1), last) &&
                    x->isLeaf == false) {
                    // 没有孩子节点，则删除
                    if (!hasChildren(x) && x->count == 0) {
                        x->children.erase(*first);
                        delete x;
                        x = nullptr;
                        return true;
                    } else {
                        x->children.erase(*first);
                        return false;
                    }
                } else {
                    // 如果当前节点没有孩子节点，则从哈希表/红黑树中删除多余的
                    // 不存在的 key-value
                    if (!hasChildren(x)) {
                        x->children.erase(*first);
                    }
                }
            }
        }
        // 叶子节点
        if (*first == endMark && x->isLeaf) {
            x->count--;
            // 只有在 count=0 时才真正的删除序列/单词
            if (!hasChildren(x) && x->count == 0) {
                delete x;
                x = nullptr;
                return true;
            } else {
                if (x->count == 0) x->isLeaf = false;
                return false;
            }
        }
        return false;
    }
    bool erase(const_reference_list_type s) {
        return erase(root, s.begin(), s.end());
    }

    // 统计某个序列/单词重复出现的次数
    int count(ct_iterator first, ct_iterator last) {
        if (!root) return 0;
        auto x = root;
        for (; first != last; first++) {
            if (*first == endMark) continue;
            if (x->children.find(*first) != x->children.end()) {
                x = x->children[*first];
            } else {
                return 0;
            }
        }
        // 只有到达叶子节点才可以得到正确的count
        if (x->isLeaf) return x->count;
        return 0;
    }
    int count(const_reference_list_type c) { return count(c.begin(), c.end()); }

    // 获取当前trie树所有的单词/序列
    void get(node_pointer x, sequence_type &v) {
        if (!x) return;
        for (auto it : x->children) {
            v.push_back(it.first);
            if (it.second->isLeaf) {
                for (int i = 0; i < it.second->count; i++) __vcLs.push_back(v);
            }
            get(it.second, v);
            v.pop_back();
        }
    }
    decltype(auto) get() {
        __vcLs.clear();
        sequence_type v;
        get(root, v);
        return __vcLs;
    }
    // 所有匹配的前缀单词
    void __prefix(node_pointer x, sequence_type s,
                  std::vector<sequence_type> &words) {
        if (!x) return;
        for (auto it : x->children) {
            s.push_back(it.first);
            if (it.second->isLeaf) {
                // 如果有重复出现
                for (int i = 0; i < it.second->count; i++) words.push_back(s);
            }
            __prefix(it.second, s, words);
            s.pop_back();
        }
    }

    /**
     * @brief 获取所有的前缀单词
     * @note
     * @param  &prefix_str:
     * @retval
     */
    auto prefixWords(const_reference_list_type prefix_str) {
        auto x = prefix_find(prefix_str);
        std::vector<sequence_type> words;
        if (!x) return words;
        if (x->isLeaf) {
            for (int i = 0; i < x->count; i++) words.push_back(prefix_str);
        }
        __prefix(x, prefix_str, words);
        return words;
    }

    // 清空TrieTree
    void clear(node_pointer_ref x) {
        if (!x) return;
        for (auto &i : x->children) {
            clear(i.second);
            // 回溯过程中删除节点
            delete i.second;
            i.second = nullptr;
        }
    }
    void clear() {
        clear(root);
        root->children.clear();
    }
    node_pointer_ref get_root() { return root; }

   protected:
    // 是否有孩子节点
    bool hasChildren(node_pointer x) {
        for (auto it : x->children)
            if (it.second) return true;
        return false;
    }

   private:
    node_pointer root;
    std::vector<sequence_type> __vcLs;
};

template <class T = char, bool Sortd = true, T endMark = '\0'>
class AC_automaton {
   public:
    using node_type = TrieNodeData<T, Sortd>;
    using node_pointer = node_type *;
    using node_pointer_ref = node_pointer &;

    using sequence_type = typename TrieTree<T, Sortd, endMark>::sequence_type;
    using const_reference_list_type = const sequence_type &;
    using reference_list_type = sequence_type &;

    AC_automaton() { root = trie.get_root(); }
    ~AC_automaton() {}

    // 构建Trie树
    void buildTrieTree(const std::vector<sequence_type> &vs) {
        for (auto x : vs) trie.insert(x);
    }

    // 构建ac自动机
    void buildAC_automaton() {
        std::queue<node_pointer> q;
        q.push(root);

        while (!q.empty()) {
            node_pointer x = q.front();
            q.pop();
            // 当前节点x的子节点
            for (auto xc : x->children) {
                node_pointer xf = x->fail;
                // 沿着树向上，直到根节点的fail=nullptr
                while (xf != nullptr) {
                    if (auto xfc = xf->children.find(xc.first);
                        xfc != xf->children.end()) {
                        xc.second->fail = xfc->second;
                        break;
                    }
                    xf = xf->fail;
                }
                q.push(xc.second);
            }
        }
    }

    void start_ac_automaton(const std::string &s) {
        node_pointer x = root;

        for (int i = 0; i < s.size(); i++) {
            char c = s.at(i);

            while (x != root && x->children.find(c) == x->children.end()) {
                x = x->fail;
            }
            if (auto y = x->children.find(c); y != x->children.end())
                x = y->second;

            node_pointer z = x;

            while (z != root) {
                // 只有在叶子节点才是一个完整的单词/序列
                if (z->isLeaf) {
                    int pos = i + 1 - z->length;
                    std::cout << s.substr(pos, z->length) << "\tindex: " << pos
                              << "\tlength : " << z->length
                              << "\tcount : " << z->count << std::endl;
                }
                // 跳转到另外的分支输出匹配的字符串
                z = z->fail;
            }
        }
    }

   private:
    TrieTree<T, Sortd, endMark> trie;
    node_pointer root;
};