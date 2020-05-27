#pragma once

#include <langutil/parser.h>

#include <nodes/error.h>

using langutil::Parser;

class Typename {
public:
    std::string name;

    bool array = false;
    bool function = false;
    bool optional = false;

    size_t paramCount = 0;

    std::vector<Typename> children;

    // more to be added, arrays or generics, for example

    bool operator==(const Typename &value) const;
    bool operator!=(const Typename &value) const;

    std::string toString() const;

    const static Typename null;
    const static Typename number;
    const static Typename string;
    const static Typename boolean;
    const static Typename any;
    const static Typename empty;

    Typename() = default;
    explicit Typename(std::string name, bool optional = false);
};

using NodeChecker = std::function<bool(class Node *)>;

class Node {
public:
    enum class Type {
        Root,
        Code,
        Enum,
        Type,
        Variable,
        Expression,
        If,
        For,
        String,
        Typename,
        Method,
        Statement,
        Page,
        Route,
        Number,
        Lambda,
        Reference,
        Array,
    };

    Type type;
    Node *parent;
    std::vector<std::shared_ptr<Node>> children;

    virtual void verify() {
        for (const auto &child : children) {
            child->verify();
        }
    }

    Node *searchThis(const NodeChecker &checker) const {
        for (const std::shared_ptr<Node> &child : children) {
            if (checker(child.get()))
                return child.get();

            child->searchThis(checker);
        }

        return nullptr;
    }

    Node *searchChildren(const NodeChecker &checker) const {
        for (const std::shared_ptr<Node> &child : children) {
            if (checker(child.get()))
                return child.get();
        }

        return nullptr;
    }

    Node *searchScope(const NodeChecker &checker) const {
        if (!parent)
            return nullptr;

        for (const std::shared_ptr<Node> &child : parent->children) {
            if (checker(child.get()))
                return child.get();
        }

        Node *parentSearch = parent->searchScope(checker);
        if (parentSearch)
            return parentSearch;

        return nullptr;
    }

    Node *searchHere(const NodeChecker &checker) const {
        if (!parent)
            return nullptr;

        for (const std::shared_ptr<Node> &child : parent->children) {
            if (checker(child.get()))
                return child.get();
        }

        return nullptr;
    }

    Node *searchParents(const NodeChecker &checker) const {
        Node *thisParent = parent;

        while (thisParent && !checker(thisParent))
            thisParent = thisParent->parent;

        return thisParent;
    }

    template <typename M>
    M *as() { return dynamic_cast<M *>(this); }

    Node(Node *parent, Type type) : parent(parent), type(type) { }
    virtual ~Node() = default;
};

std::string getName(Node *node);
