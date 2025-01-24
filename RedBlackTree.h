#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
# include <stdlib.h>
using namespace std;
namespace fs = filesystem;// Helper to write a node to a file
void writeNodeToFile(const fs::path& filepath, int key, char color, const fs::path& parent, const fs::path& left, const fs::path& right) {
    ofstream file(filepath);
    if (file.is_open()) {
        cout << "Writing to file: " << filepath << endl;
        file << "Key: " << key << endl;
        cout << "Key: " << key << endl;
        file << "Color: " << color << endl;
        cout << "Color: " << color << endl;
        file << "ParentPath: " << (parent.empty() ? "NULL" : parent.string()) << endl;
        cout << "ParentPath: " << (parent.empty() ? "NULL" : parent.string()) << endl;
        file << "LeftChildPath: " << (left.empty() ? "NULL" : left.string()) << endl;
        cout << "LeftChildPath: " << (left.empty() ? "NULL" : left.string()) << endl;
        file << "RightChildPath: " << (right.empty() ? "NULL" : right.string()) << endl;
        cout << "RightChildPath: " << (right.empty() ? "NULL" : right.string()) << endl;
        file.close();
        cout << "File write successful." << endl;
    }
    else {
        cout << "Error: Unable to write to file " << filepath << endl;
    }
}
// Helper to read a node from a file
bool readNodeFromFile(const fs::path& filepath, int& key, char& color, fs::path& parent, fs::path& left, fs::path& right) {
    if (!fs::exists(filepath)) return false;

    ifstream file(filepath);
    if (!file.is_open()) return false;

    char line[256];
    while (file.getline(line, sizeof(line))) {
        char temp[256];
        if (findSubstring(line, "Key:") == 0) {
            extractSubstring(temp, line, 5, stringLength(line) - 5);
            key = convertStringToInt(temp);
        }
        else if (findSubstring(line, "Color:") == 0)
        {
            extractSubstring(temp, line, 7, 1); // Extract only one character for Color
            color = temp[0]; // Store the first character into color
        }
        else if (findSubstring(line, "ParentPath:") == 0)
        {
            extractSubstring(temp, line, 12, stringLength(line) - 12);
            parent = (compareStrings(temp, "NULL") == 0) ? filesystem::path() : filesystem::path(temp);
        }
        else if (findSubstring(line, "LeftChildPath:") == 0)
        {
            extractSubstring(temp, line, 15, stringLength(line) - 15);
            left = (compareStrings(temp, "NULL") == 0) ? filesystem::path() : filesystem::path(temp);
        }
        else if (findSubstring(line, "RightChildPath:") == 0)
        {
            extractSubstring(temp, line, 16, stringLength(line) - 16);
            right = (compareStrings(temp, "NULL") == 0) ? filesystem::path() : filesystem::path(temp);
        }

    }
    file.close();
    return true;
}

struct Node {
    fs::path filePath;
    int key;
    fs::path parent;
    fs::path left;
    fs::path right;
    char color;
};



class RedBlackTree {
private:
    fs::path rootFile; // Path to the root node's file
    bool ll = false, rr = false, lr = false, rl = false;


public:
    RedBlackTree() : rootFile("") {}
    fs::path getRoot() {
        return rootFile;
    }
    void colorRootBlack() {
        int key;
        char color;
        fs::path parentPath, leftPath, rightPath;
        readNodeFromFile(rootFile, key, color, parentPath, leftPath, rightPath);
        writeNodeToFile(rootFile, key, 'B', parentPath, leftPath, rightPath);
    }
    // Custom function to compare two fs::path objects for equality
    bool arePathsEqual(const fs::path& path1, const fs::path& path2) {
        //if either is empty or does not exist return false. Both empty = true
        if ((path1.empty() || !fs::exists(path1)) && (path2.empty() || !fs::exists(path2)))
            return true;
        if ((path1.empty() || !fs::exists(path1)) || (path2.empty() || !fs::exists(path2)))
            return false;
        auto it1 = path1.begin();
        auto it2 = path2.begin();

        // Compare each component of the path
        while (it1 != path1.end() && it2 != path2.end()) {
            if (*it1 != *it2) {
                return false; // Components are not the same
            }
            ++it1;
            ++it2;
        }

        // Ensure both paths are fully traversed
        return it1 == path1.end() && it2 == path2.end();
    }

    // Custom function to compare two fs::path objects for inequality
    bool arePathsNotEqual(const fs::path& path1, const fs::path& path2) {
        return !arePathsEqual(path1, path2);

    }
    void printTree(const fs::path& nodeFile, int space = 0) const {
        if (nodeFile.empty() || !fs::exists(nodeFile)) return;

        int key;
        char color;
        fs::path parentPath, leftPath, rightPath;
        readNodeFromFile(nodeFile, key, color, parentPath, leftPath, rightPath);

        space += 10;
        printTree(rightPath, space);

        for (int i = 10; i < space; i++) cout << " ";
        cout << key << "(" << color << ")" << endl;

        printTree(leftPath, space);
    }

    void printTree() const {
        printTree(rootFile);
    }
    //    Left rotation

    fs::path leftRotate(const fs::path& nodeFile) {
        std::cout << "Starting left rotation on node file: " << nodeFile << std::endl;

        // Read the current node (let's call it 'node')
        int nodeKey;
        char nodeColor;
        fs::path nodeParent, nodeLeft, nodeRight;

        if (!readNodeFromFile(nodeFile, nodeKey, nodeColor, nodeParent, nodeLeft, nodeRight)) {
            throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
        }
        std::cout << "Read node: key=" << nodeKey << ", color=" << nodeColor << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;



        // Read the right child of the current node (let's call it 'x')
        if (nodeRight.empty()) {
            std::cerr << "Node has no right child, cannot perform left rotation." << std::endl;
            return nodeFile; // Cannot rotate left if there is no right child
        }
        fs::path originalParent = nodeParent;   // (to be changed at the end)




        //// read the node parent and store its left
        // if parent left exist and grandparent exists, parent left becomes grandparent right
        fs::path originalLeft = nodeLeft;

        int nodeParentKey;
        char nodeParentColor;
        fs::path nodeParentParent, nodeParentLeft, nodeParentRight;

        if (!nodeParent.empty() || fs::exists(nodeParent)) {
            if (!readNodeFromFile(nodeParent, nodeParentKey, nodeParentColor, nodeParentParent, nodeParentLeft, nodeParentRight)) {
                throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
            }
            std::cout << "Read node Parent : key=" << nodeKey << ", color=" << nodeColor << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;
            originalLeft = nodeParentLeft;

            cout << endl << "Original Left of " << nodeParent << endl;
        }


        int xKey;
        char xColor;
        fs::path xFile = nodeRight; // x is the right child of node
        fs::path xLeft, xRight;
        fs::path curr = nodeFile;
        if (!readNodeFromFile(xFile, xKey, xColor, curr, xLeft, xRight)) {
            throw std::runtime_error("Failed to read right child node from file: " + xFile.string());
        }
        std::cout << "Read right child (x): key=" << xKey << ", color=" << xColor << ", parent=" << nodeFile << ", left=" << xLeft << ", right=" << xRight << std::endl;

        if (arePathsEqual(rootFile, nodeFile)) {
            rootFile = nodeRight;
        }
        xColor = 'B';
        nodeColor = 'R';



        // Perform the left rotation
        std::cout << "Performing left rotation..." << std::endl;

        // Step 1: x->left = node
        writeNodeToFile(xFile, xKey, xColor, nodeParent, nodeFile, xRight); // x becomes the new parent of node

        // Step 2 & 3: node->right = xLeft (if xLeft exists) then update Parents
        if (!xLeft.empty()) {
            writeNodeToFile(nodeFile, nodeKey, nodeColor, xFile, nodeLeft, xLeft); // node's right is now x's left
        }
        else {
            writeNodeToFile(nodeFile, nodeKey, nodeColor, xFile, nodeLeft, ""); // node's right is now nullptr
        }



        // Step 4: Update the left child of the original parent (node 10)
        if (!nodeParent.empty()) {
            // Read the parent node to update its left child
            int parentKey;
            char parentColor;
            fs::path parentFile = nodeParent;
            fs::path parentLeft, parentRight;

            if (!readNodeFromFile(parentFile, parentKey, parentColor, parentFile, parentLeft, parentRight)) {
                cerr << "Failed to read parent node from file: " << parentFile.string() << endl;
            }

            // cout << "ORIGINAL LEFT == " << originalLeft << endl;
            writeNodeToFile(originalParent, parentKey, 'R', parentFile, originalLeft, xFile); // Update right child to x, left to original
        }
        else {
            std::cerr << "Error: Parent node path is empty." << std::endl;
        }
        colorRootBlack();
        std::cout << "Left rotation completed successfully." << std::endl;
        return xFile; // Return the new root of the subtree
    }



    fs::path rightRotate(const fs::path& nodeFile) {
        std::cout << "Starting right rotation on node file: " << nodeFile << std::endl;

        // Read the current node (let's call it 'node')
        int nodeKey;
        char nodeColor;
        fs::path nodeParent, nodeLeft, nodeRight;

        if (!readNodeFromFile(nodeFile, nodeKey, nodeColor, nodeParent, nodeLeft, nodeRight)) {
            throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
        }
        std::cout << "Read node: key=" << nodeKey << ", color=" << nodeColor << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;

        // Read the left child of the current node (let's call it 'y')
        if (nodeLeft.empty()) {
            std::cerr << "Node has no left child, cannot perform right rotation." << std::endl;
            return nodeFile; // Cannot rotate right if there is no left child
        }
        fs::path originalParent = nodeParent; // (to be changed at the end)

        fs::path originalRight = nodeRight;

        int nodeParentKey;
        char nodeParentColor;
        fs::path nodeParentParent, nodeParentLeft, nodeParentRight;

        if (!nodeParent.empty() || fs::exists(nodeParent)) {
            if (!readNodeFromFile(nodeParent, nodeParentKey, nodeParentColor, nodeParentParent, nodeParentLeft, nodeParentRight)) {
                throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
            }
            std::cout << "Read node Parent : key=" << nodeKey << ", color=" << nodeColor << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;
            originalRight = nodeParentRight;

            cout << endl << "Original Right of " << nodeParent << endl;
        }




        int yKey;
        char yColor;
        fs::path yFile = nodeLeft; // y is the left child of node
        fs::path yLeft, yRight;
        fs::path curr = nodeFile;
        if (!readNodeFromFile(yFile, yKey, yColor, curr, yLeft, yRight)) {
            throw std::runtime_error("Failed to read left child node from file: " + yFile.string());
        }
        std::cout << "Read left child (y): key=" << yKey << ", color=" << yColor << ", parent=" << nodeFile << ", left=" << yLeft << ", right=" << yRight << std::endl;

        // Perform the right rotation
        std::cout << "Performing right rotation..." << std::endl;
        if (arePathsEqual(rootFile, nodeFile)) {
            rootFile = nodeLeft;
        }
        yColor = 'B';
        nodeColor = 'R';

        // Step 1: y->right = node
        writeNodeToFile(yFile, yKey, yColor, originalParent, yLeft, nodeFile); // y becomes the new parent of node

        // Step 2: node->left = yRight (if yRight exists)
        // Step 3: Update the parent pointers
        if (!yRight.empty()) {
            writeNodeToFile(nodeFile, nodeKey, nodeColor, yFile, yRight, nodeRight); // node's left is now y's right
        }
        else {
            writeNodeToFile(nodeFile, nodeKey, nodeColor, yFile, "", nodeRight); // node's left is now nullptr
        }


        // Step 4: Update the right child of the original parent (node 10)
        if (!originalParent.empty()) {
            // Read the parent node (node 10) to update its right child
            int parentKey;
            char parentColor;
            fs::path parentFile = originalParent;
            fs::path parentLeft, parentRight;

            if (!readNodeFromFile(parentFile, parentKey, parentColor, parentFile, parentLeft, parentRight)) {
                std::cerr << "Failed to read parent node from file: " << parentFile.string() << std::endl;
            }

            // Update the right child of the parent (node 10) to point to the new child (node 5)
            writeNodeToFile(originalParent, parentKey, parentColor, parentFile, yFile, originalRight); // Update right child to y (node 5)
            std::cout << "Updated parent " << originalParent << " node's right child to point to new child (node 5)." << std::endl;
        }
        else {
            std::cerr << "Error: Parent node path is empty." << std::endl;
        }
        colorRootBlack();
        std::cout << "Right rotation completed successfully." << std::endl;
        return yFile; // Return the new root of the subtree
    }




    void insert(int key) {
        std::cout << "Inserting key: " << key << std::endl;
        fs::path newNodeFile = "Node_" + std::to_string(key) + ".txt";
        writeNodeToFile(newNodeFile, key, 'R', "", "", ""); // New nodes are red by default
        std::cout << "Created new node file: " << newNodeFile << " with color Red." << std::endl;

        if (rootFile.empty()) {
            // If the tree is empty, set the new node as the root and color it black
            rootFile = newNodeFile;
            writeNodeToFile(rootFile, key, 'B', "", "", ""); // Root is always black
            std::cout << "Tree was empty. Set " << newNodeFile << " as root and colored it Black." << std::endl;
            return;
        }

        fs::path currentFile = rootFile;
        fs::path parentFile;
        Node currentNode, parentNode;

        // Iteratively find the correct position for the new node
        while (true) {
            readNodeFromFile(currentFile, currentNode.key, currentNode.color, currentNode.parent, currentNode.left, currentNode.right);
            parentFile = currentFile;

            if (key < currentNode.key) {
                std::cout << "Going left from node " << currentNode.key << std::endl;
                if (currentNode.left.empty()) {
                    currentNode.left = newNodeFile;
                    std::cout << "Inserted " << newNodeFile << " as left child of " << currentNode.key << std::endl;
                    // Update parent pointer for the new node
                    writeNodeToFile(newNodeFile, key, 'R', parentFile, "", "");
                    break;
                }
                currentFile = currentNode.left;
            }
            else {
                std::cout << "Going right from node " << currentNode.key << std::endl;
                if (currentNode.right.empty()) {
                    currentNode.right = newNodeFile;
                    std::cout << "Inserted " << newNodeFile << " as right child of " << currentNode.key << std::endl;
                    // Update parent pointer for the new node
                    writeNodeToFile(newNodeFile, key, 'R', parentFile, "", "");
                    break;
                }
                currentFile = currentNode.right;
            }
        }

        // Write the parent node with the new child
        writeNodeToFile(parentFile, currentNode.key, currentNode.color, currentNode.parent, currentNode.left, currentNode.right);
        std::cout << "Updated parent node " << currentNode.key << " with new child." << std::endl;

        // Now handle the conflict if the parent is red
        handleConflict(newNodeFile, parentFile);

    }
    void handleConflict(const fs::path& newNodeFile, const fs::path& parentFile) {
        cout << "NEW NODE : " << newNodeFile << endl;
        cout << "PARENT FILE : " << parentFile << endl;
        Node newNode, parentNode;
        readNodeFromFile(newNodeFile, newNode.key, newNode.color, newNode.parent, newNode.left, newNode.right);
        readNodeFromFile(parentFile, parentNode.key, parentNode.color, parentNode.parent, parentNode.left, parentNode.right);

        // Check if the parent is red
        if (parentNode.color == 'R' && newNode.color == 'R') {
            std::cout << "Conflict detected: Parent " << parentNode.key << " is Red." << std::endl;

            // Find the grandparent
            fs::path grandParentFile = parentNode.parent;
            std::cout << "Grandparent found: " << grandParentFile << std::endl;

            Node grandParentNode;
            if (!grandParentFile.empty()) {
                readNodeFromFile(grandParentFile, grandParentNode.key, grandParentNode.color, grandParentNode.parent, grandParentNode.left, grandParentNode.right);
                std::cout << "Grandparent found: " << grandParentNode.key << std::endl;
            }
            std::cout << "Grandparent found: " << parentNode.parent << std::endl;

            // Determine the sibling
            fs::path siblingFile;
            if (arePathsEqual(grandParentNode.left, parentFile)) {
                siblingFile = grandParentNode.right;

            }
            else {
                siblingFile = grandParentNode.left;
            }
            Node siblingNode;
            bool siblingExists = !siblingFile.empty();
            if (siblingExists) {
                readNodeFromFile(siblingFile, siblingNode.key, siblingNode.color, siblingNode.parent, siblingNode.left, siblingNode.right);
                std::cout << "Sibling found: " << siblingNode.key << " with color " << siblingNode.color << std::endl;
            }
            else {
                // If sibling does not exist, treat it as black
                siblingNode.color = 'B'; // Treat as black
                std::cout << "Sibling does not exist. Treating as Black." << std::endl;
            }

            // Case 1: Sibling is red
            if (siblingExists && siblingNode.color == 'R') {
                std::cout << "Sibling " << siblingNode.key << " is Red. Recoloring parent and sibling to Black, and grandparent to Red." << std::endl;
                writeNodeToFile(parentFile, parentNode.key, 'B', parentNode.parent, parentNode.left, parentNode.right);
                writeNodeToFile(siblingFile, siblingNode.key, 'B', siblingNode.parent, siblingNode.left, siblingNode.right);
                char grandParentColor = 'R';
                if (arePathsEqual(rootFile, grandParentFile)) {
                    grandParentColor = 'B';
                }
                if (!grandParentFile.empty()) {
                    writeNodeToFile(grandParentFile, grandParentNode.key, grandParentColor, grandParentNode.parent, grandParentNode.left, grandParentNode.right);
                }
            }
            else {
                // Case 2: Sibling is black (or null)
                std::cout << "Sibling " << (siblingExists ? siblingNode.key : 0) << " is Black. Performing rotations." << std::endl;

                // Determine the structure of the tree to decide which rotation(s) to perform
                bool isLeftChild = (arePathsEqual(parentNode.left, newNodeFile));
                bool isParentLeftChild = (arePathsEqual(grandParentNode.left, parentFile));


                std::cout << "Checking relationships: "
                    << "\n  Parent Left: " << parentNode.left
                    << "\n  New Node File: " << newNodeFile
                    << "\n  Grandparent Left: " << grandParentNode.left
                    << "\n  Parent File: " << parentFile << std::endl;
                std::cout << "Parent Node Left: " << parentNode.left
                    << ", Right: " << parentNode.right
                    << ", New Node: " << newNodeFile << std::endl;


                if (isLeftChild && isParentLeftChild) {
                    // LL case
                    std::cout << "LL case detected. Performing right rotation on grandparent " << grandParentNode.key << std::endl;
                    rightRotate(grandParentFile);
                }
                else if (!isLeftChild && !isParentLeftChild) {
                    // RR case
                    std::cout << "RR case detected. Performing left rotation on grandparent " << grandParentNode.key << std::endl;
                    leftRotate(grandParentFile);
                }
                else if (isLeftChild && !isParentLeftChild) {
                    // LR case
                    std::cout << "LR case detected. Performing left rotation on parent " << parentNode.key << " followed by right rotation on grandparent " << grandParentNode.key << std::endl;
                    leftRotate(parentFile);
                    rightRotate(grandParentFile);
                }
                else if (!isLeftChild && isParentLeftChild) {
                    // RL case
                    std::cout << "RL case detected. Performing right rotation on parent " << parentNode.key << " followed by left rotation on grandparent " << grandParentNode.key << std::endl;
                    rightRotate(parentFile);
                    leftRotate(grandParentFile);
                }


            }

        }
        // Call the new function to check for conflicts
        checkForConflicts(newNodeFile);
    }

    void checkForConflicts(const fs::path& newNodeFile) {
        fs::path conflictNodeFile = newNodeFile;
        Node conflictNode;
        Node CParentNode;
        // Loop to check for conflicts up to the root
        while (!conflictNodeFile.empty()) {
            readNodeFromFile(conflictNodeFile, conflictNode.key, conflictNode.color, conflictNode.parent, conflictNode.left, conflictNode.right);

            // Call handleConflict for the current node and its parent
            if (!conflictNode.parent.empty()) {
                // read the parent
                readNodeFromFile(conflictNode.parent, CParentNode.key, CParentNode.color, CParentNode.parent, CParentNode.left, CParentNode.right);
                if (conflictNode.color == 'R' && CParentNode.color == 'R')
                    handleConflict(conflictNodeFile, conflictNode.parent);
                // print tree again
               // printTree();
            }

            // Move up to the parent
            conflictNodeFile = conflictNode.parent;
        }
    }




};
