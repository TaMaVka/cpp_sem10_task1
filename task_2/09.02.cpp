//g++ -std=c++20 -Wall -Wextra -Wpedantic 09.02.cpp -o 09.02.out

#include <iostream>
#include <memory>
#include <queue>

// Tree class representing a binary tree.
class Tree
{
public:
    // Public nested structure for a tree node.
    struct Node
    {
        int value;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;

        std::weak_ptr<Node> parent;

        explicit Node(int val) :
            value(val),
            left(nullptr),
            right(nullptr)
        {
            std::cout << "  Node(" << value << ") created." << std::endl;
        }

        ~Node()
        {
            std::cout << "  Node(" << value << ") destroyed." << std::endl;
        }
    };

    // The root of the tree is managed by a shared_ptr.
    std::shared_ptr<Node> root;

    Tree()
    {
        std::cout << "Tree created." << std::endl;
    }

    ~Tree()
    {
        std::cout << "Tree destroyed." << std::endl;
    }

    // Traverses the tree using Breadth-First Search (BFS).
    void traverse_v1() const
    {
        std::cout << "\n--- Traverse v1 (BFS) ---" << std::endl;
        if (!root)
        {
            std::cout << "Tree is empty." << std::endl;
            return;
        }

        std::queue<std::shared_ptr<Node>> nodes_to_visit;
        nodes_to_visit.push(root);

        while (!nodes_to_visit.empty())
        {
            std::shared_ptr<Node> current = nodes_to_visit.front();
            nodes_to_visit.pop();

            std::cout << current->value << " ";

            if (current->left)
            {
                nodes_to_visit.push(current->left);
            }
            if (current->right)
            {
                nodes_to_visit.push(current->right);
            }
        }
        std::cout << std::endl;
    }

    // Traverses the tree using Depth-First Search (DFS, Pre-order).
    void traverse_v2() const
    {
        std::cout << "\n--- Traverse v2 (DFS, Pre-order) ---" << std::endl;
        if (!root)
        {
            std::cout << "Tree is empty." << std::endl;
            return;
        }
        dfs_recursive(root);
        std::cout << std::endl;
    }

private:
    // Helper function for recursive DFS traversal.
    void dfs_recursive(const std::shared_ptr<Node>& node) const
    {
        if (!node)
        {
            return;
        }

        std::cout << node->value << " ";
        dfs_recursive(node->left);
        dfs_recursive(node->right);
    }
};

// --- Demonstration ---

int main()
{
    std::cout << "Entering main scope..." << std::endl;

    {
        Tree my_tree;

        // Construct the tree with 1 root, 2 children, and 4 grandchildren.
        // Level 0
        my_tree.root = std::make_shared<Tree::Node>(10);

        // Level 1
        my_tree.root->left = std::make_shared<Tree::Node>(5);
        my_tree.root->left->parent = my_tree.root;

        my_tree.root->right = std::make_shared<Tree::Node>(15);
        my_tree.root->right->parent = my_tree.root;

        // Level 2
        my_tree.root->left->left = std::make_shared<Tree::Node>(2);
        my_tree.root->left->left->parent = my_tree.root->left;

        my_tree.root->left->right = std::make_shared<Tree::Node>(7);
        my_tree.root->left->right->parent = my_tree.root->left;

        my_tree.root->right->left = std::make_shared<Tree::Node>(12);
        my_tree.root->right->left->parent = my_tree.root->right;

        my_tree.root->right->right = std::make_shared<Tree::Node>(20);
        my_tree.root->right->right->parent = my_tree.root->right;

        // Demonstrate traversal algorithms
        my_tree.traverse_v1(); // BFS
        my_tree.traverse_v2(); // DFS

        std::cout << "\nTree construction and usage complete. Exiting local scope..." << std::endl;
    } // `my_tree` and all its nodes will be destroyed here in reverse order of creation.

    std::cout << "\nExited main scope. All resources should be deallocated." << std::endl;
    return 0;
}