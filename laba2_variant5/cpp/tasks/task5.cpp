#include "tasks.h"
#include "support.h"
#include "../str/tree.h"
#include <memory>
#include <utility>

namespace lab2 {
namespace {

struct OwnedTree {
    Tree tree = Tree_create();
    // Nodes have independent ownership, so destroying a deep input tree is iterative.
    std::vector<std::unique_ptr<TreeNode>> nodes;
};

OwnedTree parse_tree(std::istream& input) {
    OwnedTree result;
    std::vector<TreeNode**> pending{&result.tree.root};
    for (std::string token; input >> token;) {
        if (pending.empty()) throw std::runtime_error("Лишние узлы после конца дерева");
        TreeNode** slot = pending.back();
        pending.pop_back();
        if (token == "#") continue;
        const auto value = integer(token);
        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
            throw std::runtime_error("Значение узла должно помещаться в int32");
        }
        auto node = std::make_unique<TreeNode>(TreeNode{static_cast<int>(value), nullptr, nullptr});
        *slot = node.get();
        result.nodes.push_back(std::move(node));
        pending.push_back(&(*slot)->right);
        pending.push_back(&(*slot)->left);
    }
    if (!pending.empty()) throw std::runtime_error("Неполная запись дерева: используйте # для пустых детей");
    if (input.bad()) throw std::runtime_error("Ошибка чтения дерева");
    return result;
}

std::vector<TreeNode*> inorder(const Tree& tree) {
    std::vector<TreeNode*> result;
    std::vector<TreeNode*> pending;
    TreeNode* current = tree.root;
    while (current || !pending.empty()) {
        while (current) {
            pending.push_back(current);
            current = current->left;
        }
        current = pending.back();
        pending.pop_back();
        result.push_back(current);
        current = current->right;
    }
    return result;
}

bool recover(Tree& tree) {
    auto nodes = inorder(tree);
    TreeNode* first = nullptr;
    TreeNode* second = nullptr;
    for (std::size_t i = 1; i < nodes.size(); ++i) {
        if (nodes[i - 1]->data >= nodes[i]->data) {
            if (!first) first = nodes[i - 1];
            second = nodes[i];
        }
    }
    if (!first || !second) return false;
    std::swap(first->data, second->data);
    for (std::size_t i = 1; i < nodes.size(); ++i) {
        if (nodes[i - 1]->data >= nodes[i]->data) {
            std::swap(first->data, second->data);
            return false;
        }
    }
    return true;
}

} // namespace

void run_task5(std::istream& input, std::ostream& output) {
    OwnedTree owned = parse_tree(input);
    if (!recover(owned.tree)) {
        throw std::runtime_error("Дерево не исправляется обменом ровно двух различных значений");
    }
    output << "Исправленное дерево (preorder):";
    std::vector<const TreeNode*> pending{owned.tree.root};
    while (!pending.empty()) {
        const TreeNode* node = pending.back();
        pending.pop_back();
        if (!node) {
            output << " #";
            continue;
        }
        output << ' ' << node->data;
        pending.push_back(node->right);
        pending.push_back(node->left);
    }
    output << "\nInorder:";
    for (const TreeNode* node : inorder(owned.tree)) {
        output << ' ' << node->data;
    }
    output << '\n';
}

} // namespace lab2
