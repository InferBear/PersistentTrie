#include <iostream>
#include <map>
#include <string>

class TrieNode;

template<class T>
using SPT = std::shared_ptr<T>;

using Children = std::unordered_map<char, SPT<const TrieNode>>;

class TrieNode {
public:
    TrieNode() = default;
    virtual ~TrieNode() = default;
    explicit TrieNode(Children children) : children(std::move(children)) {}
    bool isEndOfWord{false};
    Children children;
    virtual std::unique_ptr<TrieNode> copy() const {
        return std::make_unique<TrieNode>(children);
    }
};

template <class T>
class TrieValueNode : public TrieNode {
public:
    TrieValueNode() = delete;
    explicit TrieValueNode(SPT<T> value) : value(std::move(value)) {
        this -> isEndOfWord = true;
    }
    TrieValueNode(Children children, SPT<T> value) : TrieNode(std::move(children)), value(std::move(value)) {
        this -> isEndOfWord = true;
    }
    SPT<T> value;
    std::unique_ptr<TrieNode> copy() const override {
        return std::make_unique<TrieValueNode>(children, value);
    }
};

class Trie {
public:
    SPT<const TrieNode> root{nullptr};
    Trie() = default;

    template <class T>
    const T* search(std::string key) const;

    template <class T>
    Trie insert(std::string key, T value) const;

    Trie remove(std::string key) const;
private:
    explicit Trie(SPT<const TrieNode> root) : root(std::move(root)) {}
};

template<class T>
const T* Trie::search(std::string key) const {
    size_t size = key.size();
    auto cur = this -> root;
    for (size_t i = 0; i < size; i++) {
        char k = key[i];
        if (cur && cur -> children.find(k) != cur -> children.end()) {
            cur = cur -> children.at(k);
        } else {
            return nullptr;
        }
    }

    auto ret = dynamic_cast<const TrieValueNode<T>*>(cur.get());

    if (ret && ret -> isEndOfWord) {
        return ret -> value.get();
    }
    return nullptr;
}

template<class T>
Trie Trie::insert(std::string key, T value) const {
    size_t size = key.size();
    auto dfs = [&](auto self, size_t idx, std::shared_ptr<const TrieNode> cur) -> std::shared_ptr<const TrieNode> {
        if (idx == size) {
            auto value_ptr = std::make_shared<T>(std::move(value));
            if (cur) { // NOLINT(*-no-recursion)
                return std::make_shared<TrieValueNode<T>>(cur -> children, value_ptr);
            } else {
                return std::make_shared<TrieValueNode<T>>(value_ptr);
            }
        }
        char k = key[idx];
        auto t = cur;
        if (cur && cur -> children.find(k) != cur -> children.end()) {
            cur = cur -> children.at(k);
        } else {
            cur = nullptr;
        }

        auto next = self(self, idx + 1, cur);
        std::shared_ptr<TrieNode> ret;
        if (t) {
            ret = t -> copy();
        } else {
            ret = std::make_shared<TrieNode>();
        }
        ret -> children[k] = next;
        return ret;
    };

    auto newRoot = dfs(dfs, 0, this -> root);

    return Trie(newRoot);
}

Trie Trie::remove(std::string key) const {
    if (this -> root == nullptr) {
        return Trie(nullptr);
    }
    size_t size = key.size();

    auto dfs =
            [&](auto self, size_t idx, std::shared_ptr<const TrieNode> cur) -> std::shared_ptr<const TrieNode> {
        if (idx == size) {
            return std::make_shared<const TrieNode>(cur->children);
        }

        char k = key[idx];
        auto ret = cur->copy();
        if (cur && cur->children.find(k) != cur->children.end()) {
            cur = cur->children.at(k);
        } else {
            return nullptr;
        }

        auto next = self(self, idx + 1, cur);

        if (next == nullptr) {
            return nullptr;
        }

        if (next->children.empty() && !next->isEndOfWord) {
            ret->children.erase(k);
        } else {
            ret->children[k] = next;
        }

        return ret;
    };

    auto ret = dfs(dfs, 0, this -> root);
    if (ret && ret->children.empty() && !ret -> isEndOfWord) {
        return Trie(nullptr);
    } else if (!ret) {
        return Trie(this -> root);
    }
    return Trie(ret);
}

int main() {
    Trie trie;
    trie = trie.insert<int>("abc", 123);
    trie = trie.insert<int>("ab", 12);
    trie = trie.insert<int>("abcedfg", 888);
    trie = trie.insert<std::string>("ijk", "xyz");
    trie = trie.insert<double>("xyz", 0.888);
    std::cout << *trie.search<int>("abc") << std::endl;
    std::cout << *trie.search<int>("ab") << std::endl;
    std::cout << *trie.search<std::string>("ijk") << std::endl;
    std::cout << *trie.search<double>("xyz") << std::endl;
    trie = trie.remove("ijk");
    trie = trie.remove("ijkf");
    trie = trie.remove("ab");
    std::cout << ((trie.search<std::string>("ijk") == nullptr ? "hasNoValue" : "hasValue")) << std::endl;
    std::cout << ((trie.search<int>("abc") == nullptr ? "hasNoValue" : "hasValue")) << std::endl;
    std::cout << ((trie.search<int>("ab") == nullptr ? "hasNoValue" : "hasValue")) << std::endl;
    trie = trie.insert<std::string>("", "naughty");
    std::cout << *trie.search<std::string>("") << std::endl;
}
