#pragma once
#pragma once
#include "GlobalFunctions.h"
//#include "GlobalFunctions.h"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
# include <stdlib.h>
using namespace std;
namespace fs = filesystem;// Helper to write a node to a file

void writeNodeToFile(const fs::path& filepath, int key, char* rowData, char color, const fs::path& parent, const fs::path& left, const fs::path& right) {
    ofstream file(filepath);
    if (file.is_open()) {
        file.seekp(0, ios::beg); //go to the start of the file
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
        file << "RowData: " << rowData << endl;
        file.close();
        cout << "File write successful." << endl;
    }
    else {
        cout << "Error: Unable to write to file " << filepath << endl;
    }
}

//overloaded only for duplicates
void writeNodeToFile(const fs::path& filepath, int key, char* rowData, char color, const fs::path& parent, const fs::path& left, const fs::path& right, bool isDuplicate)
{
    ofstream file;
    //open file in append mode if it exists; otherwise, create a new one
    if (filesystem::exists(filepath))
        file.open(filepath, ios::app);
    else
        file.open(filepath);

    if (file.is_open()) //append duplicate value rows
    {
        file << "RowData: " << rowData << endl;
        file.close();
    }
    else
    {
        cout << "Unable to write to file " << filepath << endl;
    }
}

// Helper to read a node from a file
bool readNodeFromFile(const fs::path& filepath, int& key, char* rowData, char& color, fs::path& parent, fs::path& left, fs::path& right) {
    if (!fs::exists(filepath)) return false;

    ifstream file(filepath);
    if (!file.is_open()) return false;

    char line[256];
    bool isRowData = false;
    char rowDataAccumulated[10000] = ""; //combine row data for duplicates
    int currentRowDataLength = 0;  //track the length of accumulated data

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
        else if (findSubstring(line, "RowData:") == 0)
        {
            //read and append this line to the accumulated data
            extractSubstring(temp, line, 9, stringLength(line) - 9);
            currentRowDataLength = stringLength(temp);
            copyIn(rowDataAccumulated, temp);
            isRowData = true;

            //keep reading the next lines until we hit a non-RowData line or EOF
            while (file.getline(line, sizeof(line)))
            {
                if (findSubstring(line, "RowData:") == 0)
                {
                    int lineLength = stringLength(line);
                    if (currentRowDataLength + lineLength + 1 < sizeof(rowDataAccumulated)) //add it to the array
                    {
                        rowDataAccumulated[currentRowDataLength] = '\n';
                        currentRowDataLength++;
                        copyIn(rowDataAccumulated + currentRowDataLength, line);
                        currentRowDataLength += lineLength;
                    }
                    else
                    {
                        cerr << "RowData overflowed :(" << endl;
                        return false;
                    }
                }
            }
        }
    }
    file.close();
    //copy in the data if duplicates
    if (stringLength(rowDataAccumulated) > 0)
    {
        //delete[] rowData;
        rowData = new char[stringLength(rowDataAccumulated) + 1];
        copyIn(rowData, rowDataAccumulated);
    }
    return true;
}

struct Node {
    fs::path filePath;
    int key;
    fs::path parent;
    fs::path left;
    fs::path right;
    char color;
    char* rowData = new char[1000] {'\0'};
};

class RedBlackTree {
private:
    fs::path rootFile; // Path to the root node's file
    bool ll = false, rr = false, lr = false, rl = false;


public:
    int hashChoice = 2; //default to sha

    RedBlackTree() : rootFile("") {}
    fs::path getRoot() {
        return rootFile;
    }
    void colorRootBlack() {
        int key;
        char color;
        fs::path parentPath, leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(rootFile, key, rowData, color, parentPath, leftPath, rightPath);
        writeNodeToFile(rootFile, key, rowData, 'B', parentPath, leftPath, rightPath);
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
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, color, parentPath, leftPath, rightPath);

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

    //fs::path leftRotate(const fs::path& nodeFile) {
    //    std::cout << "Starting left rotation on node file: " << nodeFile << std::endl;

    //    // Read the current node (let's call it 'node')
    //    int nodeKey;
    //    char nodeColor;
    //    fs::path nodeParent, nodeLeft, nodeRight;
    //    char* rowData = new char[1000] {'\0'};
    //    if (!readNodeFromFile(nodeFile, nodeKey, rowData, nodeColor, nodeParent, nodeLeft, nodeRight)) {
    //        throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
    //    }
    //    std::cout << "Read node: key=" << nodeKey << ", color=" << nodeColor << ", row data=" << rowData << ", parent = " << nodeParent << ", left = " << nodeLeft << ", right = " << nodeRight << std::endl;

    //    // Read the right child of the current node (let's call it 'x')
    //    if (nodeRight.empty()) {
    //        std::cerr << "Node has no right child, cannot perform left rotation." << std::endl;
    //        return nodeFile; // Cannot rotate left if there is no right child
    //    }
    //    fs::path originalParent = nodeParent;   // (to be changed at the end)

    //    //// read the node parent and store its left
    //    // if parent left exist and grandparent exists, parent left becomes grandparent right
    //    fs::path originalLeft = nodeLeft;

    //    int nodeParentKey;
    //    char nodeParentColor;
    //    fs::path nodeParentParent, nodeParentLeft, nodeParentRight;
    //    char* nodeParentRowData = new char[1000] {'\0'};
    //    if (!nodeParent.empty() || fs::exists(nodeParent)) {
    //        if (!readNodeFromFile(nodeParent, nodeParentKey, nodeParentRowData, nodeParentColor, nodeParentParent, nodeParentLeft, nodeParentRight)) {
    //            throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
    //        }
    //        std::cout << "Read node Parent : key=" << nodeKey << ", color=" << nodeColor << ", row data=" << nodeParentRowData << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;
    //        originalLeft = nodeParentLeft;

    //        cout << endl << "Original Left of " << nodeParent << endl;
    //    }


    //    int xKey;
    //    char xColor;
    //    fs::path xFile = nodeRight; // x is the right child of node
    //    fs::path xLeft, xRight;
    //    fs::path curr = nodeFile;
    //    char* xRowData = new char[1000] {'\0'};
    //    if (!readNodeFromFile(xFile, xKey, xRowData, xColor, curr, xLeft, xRight)) {
    //        throw std::runtime_error("Failed to read right child node from file: " + xFile.string());
    //    }
    //    std::cout << "Read right child (x): key=" << xKey << ", color=" << xColor << ", row data=" << xRowData << ", parent=" << nodeFile << ", left=" << xLeft << ", right=" << xRight << std::endl;

    //    if (arePathsEqual(rootFile, nodeFile)) {
    //        rootFile = nodeRight;
    //    }
    //    xColor = 'B';
    //    nodeColor = 'R';


    //    // Perform the left rotation
    //    std::cout << "Performing left rotation..." << std::endl;

    //    //--------------------------------------------CHECK THE ROW DATA IM PASSING HERE, DONT KNOW WHAT GOES WHERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //    // Step 1: x->left = node
    //    writeNodeToFile(xFile, xKey, xRowData, xColor, nodeParent, nodeFile, xRight); // x becomes the new parent of node

    //    // Step 2 & 3: node->right = xLeft (if xLeft exists) then update Parents
    //    if (!xLeft.empty()) {
    //        writeNodeToFile(nodeFile, nodeKey, rowData, nodeColor, xFile, nodeLeft, xLeft); // node's right is now x's left
    //    }
    //    else {
    //        writeNodeToFile(nodeFile, nodeKey, rowData, nodeColor, xFile, nodeLeft, ""); // node's right is now nullptr
    //    }



    //    // Step 4: Update the left child of the original parent (node 10)
    //    if (!nodeParent.empty()) {
    //        // Read the parent node to update its left child
    //        int parentKey;
    //        char parentColor;
    //        fs::path parentFile = nodeParent;
    //        fs::path parentLeft, parentRight;
    //        char* parentRowData = new char[1000] {'\0'};
    //        if (!readNodeFromFile(parentFile, parentKey, parentRowData, parentColor, parentFile, parentLeft, parentRight)) {
    //            cerr << "Failed to read parent node from file: " << parentFile.string() << endl;
    //        }

    //        // cout << "ORIGINAL LEFT == " << originalLeft << endl;
    //        writeNodeToFile(originalParent, parentKey, parentRowData, 'R', parentFile, originalLeft, xFile); // Update right child to x, left to original
    //    }
    //    else {
    //        std::cerr << "Error: Parent node path is empty." << std::endl;
    //    }
    //    colorRootBlack();
    //    std::cout << "Left rotation completed successfully." << std::endl;
    //    return xFile; // Return the new root of the subtree
    //}

    //fs::path rightRotate(const fs::path& nodeFile) {
    //    std::cout << "Starting right rotation on node file: " << nodeFile << std::endl;

    //    // Read the current node (let's call it 'node')
    //    int nodeKey;
    //    char nodeColor;
    //    fs::path nodeParent, nodeLeft, nodeRight;
    //    char* nodeRowData = new char[1000] {'\0'};
    //    if (!readNodeFromFile(nodeFile, nodeKey, nodeRowData, nodeColor, nodeParent, nodeLeft, nodeRight)) {
    //        throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
    //    }
    //    std::cout << "Read node: key=" << nodeKey << ", color=" << nodeColor << ", row dta=" << nodeRowData << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;

    //    // Read the left child of the current node (let's call it 'y')
    //    if (nodeLeft.empty()) {
    //        std::cerr << "Node has no left child, cannot perform right rotation." << std::endl;
    //        return nodeFile; // Cannot rotate right if there is no left child
    //    }
    //    fs::path originalParent = nodeParent; // (to be changed at the end)

    //    fs::path originalRight = nodeRight;

    //    int nodeParentKey;
    //    char nodeParentColor;
    //    fs::path nodeParentParent, nodeParentLeft, nodeParentRight;
    //    char* nodeParentRowData = new char[1000] {'\0'};
    //    if (!nodeParent.empty() || fs::exists(nodeParent)) {
    //        if (!readNodeFromFile(nodeParent, nodeParentKey, nodeParentRowData, nodeParentColor, nodeParentParent, nodeParentLeft, nodeParentRight)) {
    //            throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
    //        }
    //        std::cout << "Read node Parent : key=" << nodeKey << ", color=" << nodeColor << ", row data=" << nodeParentRowData << ", parent=" << nodeParent << ", left=" << nodeLeft << ", right=" << nodeRight << std::endl;
    //        originalRight = nodeParentRight;

    //        cout << endl << "Original Right of " << nodeParent << endl;
    //    }




    //    int yKey;
    //    char yColor;
    //    fs::path yFile = nodeLeft; // y is the left child of node
    //    fs::path yLeft, yRight;
    //    fs::path curr = nodeFile;
    //    char* yRowData = new char[1000] {'\0'};
    //    if (!readNodeFromFile(yFile, yKey, yRowData, yColor, curr, yLeft, yRight)) {
    //        throw std::runtime_error("Failed to read left child node from file: " + yFile.string());
    //    }
    //    std::cout << "Read left child (y): key=" << yKey << ", color=" << yColor << ", parent=" << nodeFile << ", left=" << yLeft << ", right=" << yRight << std::endl;

    //    // Perform the right rotation
    //    std::cout << "Performing right rotation..." << std::endl;
    //    if (arePathsEqual(rootFile, nodeFile)) {
    //        rootFile = nodeLeft;
    //    }
    //    yColor = 'B';
    //    nodeColor = 'R';

    //    //------------------------------------ CHECK WHAT DATA IS GOING WHERE!!!!

    //    // Step 1: y->right = node
    //    writeNodeToFile(yFile, yKey, yRowData, yColor, originalParent, yLeft, nodeFile); // y becomes the new parent of node

    //    // Step 2: node->left = yRight (if yRight exists)
    //    // Step 3: Update the parent pointers
    //    if (!yRight.empty()) {
    //        writeNodeToFile(nodeFile, nodeKey, nodeRowData, nodeColor, yFile, yRight, nodeRight); // node's left is now y's right
    //    }
    //    else {
    //        writeNodeToFile(nodeFile, nodeKey, nodeRowData, nodeColor, yFile, "", nodeRight); // node's left is now nullptr
    //    }


    //    // Step 4: Update the right child of the original parent (node 10)
    //    if (!originalParent.empty()) {
    //        // Read the parent node (node 10) to update its right child
    //        int parentKey;
    //        char parentColor;
    //        fs::path parentFile = originalParent;
    //        fs::path parentLeft, parentRight;
    //        char* parentRowData = new char[1000] {'\0'};
    //        if (!readNodeFromFile(parentFile, parentKey, parentRowData, parentColor, parentFile, parentLeft, parentRight)) {
    //            std::cerr << "Failed to read parent node from file: " << parentFile.string() << std::endl;
    //        }

    //        // Update the right child of the parent (node 10) to point to the new child (node 5)
    //        writeNodeToFile(originalParent, parentKey, parentRowData, parentColor, parentFile, yFile, originalRight); // Update right child to y (node 5)
    //        std::cout << "Updated parent " << originalParent << " node's right child to point to new child (node 5)." << std::endl;
    //    }
    //    else {
    //        std::cerr << "Error: Parent node path is empty." << std::endl;
    //    }
    //    colorRootBlack();
    //    std::cout << "Right rotation completed successfully." << std::endl;
    //    return yFile; // Return the new root of the subtree
    //}




fs::path leftRotate(const fs::path& nodeFile) {
    // Read the current node
    int nodeKey;
    char nodeColor;
    char nodeRowData[1000] = { '\0' };
    fs::path nodeParent, nodeLeft, nodeRight;

    if (!readNodeFromFile(nodeFile, nodeKey, nodeRowData, nodeColor, nodeParent, nodeLeft, nodeRight)) {
        throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
    }

    // Validate the right child
    if (nodeRight.empty()) {
        std::cerr << "Node has no right child, cannot perform left rotation." << std::endl;
        return nodeFile;
    }

    // Read the right child (x)
    int xKey;
    char xColor;
    char xRowData[1000] = { '\0' };
    fs::path xLeft, xRight, nodePath;

    if (!readNodeFromFile(nodeRight, xKey, xRowData, xColor, nodePath, xLeft, xRight)) {
        throw std::runtime_error("Failed to read right child node: " + nodeRight.string());
    }

    // Update pointers
    fs::path originalParent = nodeParent;
    writeNodeToFile(nodeRight, xKey, xRowData, nodeColor, originalParent, nodeFile, xRight); // Inherit color
    writeNodeToFile(nodePath, nodeKey, nodeRowData, xColor, nodeRight, nodeLeft, xLeft);    // Inherit color from x

    // Fix parent pointers
    if (!nodeParent.empty()) {
        int parentKey;
        char parentColor;
        char parentRowData[1000] = { '\0' };
        fs::path parentLeft, parentRight, ParentParent;

        if (!readNodeFromFile(nodeParent, parentKey, parentRowData, parentColor, ParentParent, parentLeft, parentRight)) {
            throw std::runtime_error("Failed to read parent node: " + nodeParent.string());
        }

        if (arePathsEqual(parentLeft, nodePath)) {
            writeNodeToFile(nodeParent, parentKey, parentRowData, parentColor, ParentParent, nodeRight, parentRight);
        }
        else if (arePathsEqual(parentRight, nodePath)) {
            writeNodeToFile(nodeParent, parentKey, parentRowData, parentColor, ParentParent, parentLeft, nodeRight);
        }
    }
    else {
        rootFile = nodeRight; // Update root if no parent exists
    }

    // Fix colors
    writeNodeToFile(nodeRight, xKey, xRowData, 'B', originalParent, nodeFile, xRight); // New root must be black
    writeNodeToFile(nodePath, nodeKey, nodeRowData, 'R', nodeRight, nodeLeft, xLeft);  // Current node becomes red

    colorRootBlack(); // Ensure root remains black
    return nodeRight;
}


fs::path rightRotate(const fs::path& nodeFile) {
    // Read the current node
    int nodeKey;
    char nodeColor;
    char nodeRowData[1000] = { '\0' };
    fs::path nodeParent, nodeLeft, nodeRight;

    if (!readNodeFromFile(nodeFile, nodeKey, nodeRowData, nodeColor, nodeParent, nodeLeft, nodeRight)) {
        throw std::runtime_error("Failed to read node from file: " + nodeFile.string());
    }

    // Validate the left child
    if (nodeLeft.empty()) {
        std::cerr << "Node has no left child, cannot perform right rotation." << std::endl;
        return nodeFile;
    }

    // Read the left child (y)
    int yKey;
    char yColor;
    char yRowData[1000] = { '\0' };
    fs::path yLeft, yRight, nodePath;

    if (!readNodeFromFile(nodeLeft, yKey, yRowData, yColor, nodePath, yLeft, yRight)) {
        throw std::runtime_error("Failed to read left child node: " + nodeLeft.string());
    }

    // Update pointers
    fs::path originalParent = nodeParent;
    writeNodeToFile(nodeLeft, yKey, yRowData, nodeColor, originalParent, yLeft, nodeFile); // Inherit color
    writeNodeToFile(nodePath, nodeKey, nodeRowData, yColor, nodeLeft, yRight, nodeRight);  // Inherit color from y

    // Fix parent pointers
    if (!nodeParent.empty()) {
        int parentKey;
        char parentColor;
        char parentRowData[1000] = { '\0' };
        fs::path parentLeft, parentRight, ParentParent;

        if (!readNodeFromFile(nodeParent, parentKey, parentRowData, parentColor, ParentParent, parentLeft, parentRight)) {
            throw std::runtime_error("Failed to read parent node: " + nodeParent.string());
        }

        if (arePathsEqual(parentLeft, nodePath)) {
            writeNodeToFile(nodeParent, parentKey, parentRowData, parentColor, ParentParent, nodeLeft, parentRight);
        }
        else if (arePathsEqual(parentRight, nodePath)) {
            writeNodeToFile(nodeParent, parentKey, parentRowData, parentColor, ParentParent, parentLeft, nodeLeft);
        }
    }
    else {
        rootFile = nodeLeft; // Update root if no parent exists
    }

    // Fix colors
    writeNodeToFile(nodeLeft, yKey, yRowData, 'B', originalParent, yLeft, nodeFile); // New root must be black
    writeNodeToFile(nodePath, nodeKey, nodeRowData, 'R', nodeLeft, yRight, nodeRight); // Current node becomes red

    colorRootBlack(); // Ensure root remains black
    return nodeLeft;
}





    //---------------------------------------------------added data paraemeter to this insert header
    void insert(int key, char*& rowData) {
        std::cout << "Inserting key: " << key << std::endl;
        fs::path newNodeFile = "Node_" + std::to_string(key) + ".txt";
        writeNodeToFile(newNodeFile, key, rowData, 'R', "", "", ""); // New nodes are red by default
        std::cout << "Created new node file: " << newNodeFile << " with color Red." << std::endl;

        if (rootFile.empty()) {
            // If the tree is empty, set the new node as the root and color it black
            rootFile = newNodeFile;
            writeNodeToFile(rootFile, key, rowData, 'B', "", "", ""); // Root is always black
            std::cout << "Tree was empty. Set " << newNodeFile << " as root and colored it Black." << std::endl;
            return;
        }

        fs::path currentFile = rootFile;
        fs::path parentFile;
        Node currentNode, parentNode;

        // Iteratively find the correct position for the new node
        while (true) {
            readNodeFromFile(currentFile, currentNode.key, currentNode.rowData, currentNode.color, currentNode.parent, currentNode.left, currentNode.right);
            parentFile = currentFile;

            if (key < currentNode.key) {
                std::cout << "Going left from node " << currentNode.key << std::endl;
                if (currentNode.left.empty()) {
                    currentNode.left = newNodeFile;
                    std::cout << "Inserted " << newNodeFile << " as left child of " << currentNode.key << std::endl;
                    // Update parent pointer for the new node
                    writeNodeToFile(newNodeFile, key, rowData, 'R', parentFile, "", "");
                    break;
                }
                currentFile = currentNode.left;
            }
            else if (key > currentNode.key) {
                std::cout << "Going right from node " << currentNode.key << std::endl;
                if (currentNode.right.empty()) {
                    currentNode.right = newNodeFile;
                    std::cout << "Inserted " << newNodeFile << " as right child of " << currentNode.key << std::endl;
                    // Update parent pointer for the new node
                    writeNodeToFile(newNodeFile, key, rowData, 'R', parentFile, "", "");
                    break;
                }
                currentFile = currentNode.right;
            }
            else //-----------------------------------------this else condition is new for duplicates
            {
                //append duplictaes
                writeNodeToFile(newNodeFile, key, rowData, 'R', parentFile, "", ""); ///-------------------------------IDK IF I SHOULD BREAK HERE OR IF THERE SHOULD BE AN IF CONDITION!!!!!!!!!!!1
                break;
            }
        }

        // Write the parent node with the new child
        writeNodeToFile(parentFile, currentNode.key, currentNode.rowData, currentNode.color, currentNode.parent, currentNode.left, currentNode.right);
        std::cout << "Updated parent node " << currentNode.key << " with new child." << std::endl;

        // Now handle the conflict if the parent is red
        handleConflict(newNodeFile, parentFile);

    }

    void handleConflict(const fs::path& newNodeFile, const fs::path& parentFile) {
        cout << "NEW NODE : " << newNodeFile << endl;
        cout << "PARENT FILE : " << parentFile << endl;
        Node newNode, parentNode;
        readNodeFromFile(newNodeFile, newNode.key, newNode.rowData, newNode.color, newNode.parent, newNode.left, newNode.right);
        readNodeFromFile(parentFile, parentNode.key, parentNode.rowData, parentNode.color, parentNode.parent, parentNode.left, parentNode.right);

        // Check if the parent is red
        if (parentNode.color == 'R' && newNode.color == 'R') {
            std::cout << "Conflict detected: Parent " << parentNode.key << " is Red." << std::endl;

            // Find the grandparent
            fs::path grandParentFile = parentNode.parent;
            std::cout << "Grandparent found: " << grandParentFile << std::endl;

            Node grandParentNode;
            if (!grandParentFile.empty()) {
                readNodeFromFile(grandParentFile, grandParentNode.key, grandParentNode.rowData, grandParentNode.color, grandParentNode.parent, grandParentNode.left, grandParentNode.right);
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
                readNodeFromFile(siblingFile, siblingNode.key, siblingNode.rowData, siblingNode.color, siblingNode.parent, siblingNode.left, siblingNode.right);
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
                writeNodeToFile(parentFile, parentNode.key, parentNode.rowData, 'B', parentNode.parent, parentNode.left, parentNode.right);
                writeNodeToFile(siblingFile, siblingNode.key, siblingNode.rowData, 'B', siblingNode.parent, siblingNode.left, siblingNode.right);
                char grandParentColor = 'R';
                if (arePathsEqual(rootFile, grandParentFile)) {
                    grandParentColor = 'B';
                }
                if (!grandParentFile.empty()) {
                    writeNodeToFile(grandParentFile, grandParentNode.key, grandParentNode.rowData, grandParentColor, grandParentNode.parent, grandParentNode.left, grandParentNode.right);
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
                else if (!isLeftChild && isParentLeftChild) {
                    // LR case
                    std::cout << "LR case detected. Performing left rotation on parent " << parentNode.key << " followed by right rotation on grandparent " << grandParentNode.key << std::endl;
                    leftRotate(parentFile);
                    rightRotate(grandParentFile);
                }
                else if (isLeftChild && !isParentLeftChild) {
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
            readNodeFromFile(conflictNodeFile, conflictNode.key, conflictNode.rowData, conflictNode.color, conflictNode.parent, conflictNode.left, conflictNode.right);

            // Call handleConflict for the current node and its parent
            if (!conflictNode.parent.empty()) {
                // read the parent
                readNodeFromFile(conflictNode.parent, CParentNode.key, CParentNode.rowData, CParentNode.color, CParentNode.parent, CParentNode.left, CParentNode.right);
                if (conflictNode.color == 'R' && CParentNode.color == 'R')
                    handleConflict(conflictNodeFile, conflictNode.parent);
                // print tree again
               // printTree();
            }

            // Move up to the parent
            conflictNodeFile = conflictNode.parent;
        }
    }



    void deleteNode(int key) {
        Node z;
        z.filePath = search(key);
        if (z.filePath.empty()) {
            cout << "Key not found!" << endl;
            return;
        }
        cout << "Key File : " << z.filePath << endl;;
        readNodeFromFile(z.filePath, z.key, z.rowData, z.color, z.parent, z.left, z.right);

        Node y;
        y.filePath = z.filePath;
        y.color = z.color;
        y.key = z.color;
        y.left = z.left;
        y.right = z.right;
        y.parent = z.parent;
        writeNodeToFile(y.filePath, y.key, y.rowData, y.color, y.parent, y.left, y.right);
        char yOrigColor = y.color;
        cout << "Original color" << yOrigColor << endl;
        Node x;
        // Case 1: z has no left child
        if (z.left.empty()) {
            x.filePath = z.right;
            readNodeFromFile(x.filePath, x.key, x.rowData, x.color, x.parent, x.left, x.right);
            writeNodeToFile(x.filePath, x.key, x.rowData, x.color, x.parent, x.left, x.right);
            cout << "x.filepath" << x.filePath << endl;
            cout << "transplanting " << endl;
            transplant(z.filePath, z.right);

        }
        // Case 2: z has no right child
        else if (z.right.empty()) {
            x.filePath = z.left;
            readNodeFromFile(x.filePath, x.key, x.rowData, x.color, x.parent, x.left, x.right);
            writeNodeToFile(x.filePath, x.key, x.rowData, x.color, x.parent, x.left, x.right);
            transplant(z.filePath, z.left);
        }
        // Case 3: z has two children
        else {
            y.filePath = minimum(z.right);
            readNodeFromFile(y.filePath, y.key, y.rowData, y.color, y.parent, y.left, y.right);
            yOrigColor = y.color;
            x.filePath = y.right;
            Node y_right;
            y_right.filePath = y.right;
            readNodeFromFile(y_right.filePath, y_right.key, y_right.rowData, y_right.color, y_right.parent, y_right.left, y_right.right);
            writeNodeToFile(x.filePath, y_right.key, y_right.rowData, y_right.color, y_right.parent, y_right.left, y_right.right);
            if (arePathsEqual(y.parent, z.filePath)) {
                x.parent = y.filePath;
                Node x_parent;
                x_parent.filePath = x.parent;
                writeNodeToFile(x_parent.filePath, y.key, y.rowData, y.color, y.parent, y.left, y.right);
                writeNodeToFile(x.filePath, x.key, y.rowData, x.color, x.parent, x.left, x.right);
            }
            else {
                transplant(y.filePath, y.right);
                Node z_right;
                z_right.filePath = z.right;
                readNodeFromFile(z_right.filePath, z_right.key, z_right.rowData, z_right.color, z_right.parent, z_right.left, z_right.right);
                y.right = z.right;
                writeNodeToFile(y_right.filePath, z_right.key, z_right.rowData, z_right.color, z_right.parent, z_right.left, z_right.right);
                y_right.parent = y.filePath;
                writeNodeToFile(y_right.parent, y.key, y.rowData, y.color, y.parent, y.left, y.right);

            }

            transplant(z.filePath, y.filePath);
            Node y_left;
            y_left.filePath = y.left;
            readNodeFromFile(y_left.filePath, y_left.key, y_left.rowData, y_left.color, y_left.parent, y_left.left, y_left.right);
            y.left = z.left;
            writeNodeToFile(y.filePath, y.key, y.rowData, y.color, y.parent, y.left, y.right);
            Node z_left;
            z_left.filePath = z.left;
            readNodeFromFile(z_left.filePath, z_left.key, z_left.rowData, z_left.color, z_left.parent, z_left.left, z_left.right);
            writeNodeToFile(y_left.filePath, z_left.key, z_left.rowData, z_left.color, z_left.parent, z_left.left, z_left.right);

            writeNodeToFile(y_left.parent, y.key, y.rowData, y.color, y.parent, y.left, y.right);
            y_left.parent = y.filePath;
            y.color = z.color;
            writeNodeToFile(y.filePath, y.key, y.rowData, y.color, y.parent, y.left, y.right);
        }
        remove(z.filePath);
        if (yOrigColor == 'B') {
            cout << "FIXING UP   " << endl;
            deleteFixup(x.filePath);
        }

    }
    void deleteFixup(const fs::path& xFile) {

        Node x;
        x.filePath = xFile;
        while (!x.filePath.empty() && filesystem::exists(x.filePath) && !arePathsEqual(x.filePath, rootFile)) {
            cout << "inside while " << endl;
            Node xParent;
            readNodeFromFile(x.filePath, x.key, x.rowData, x.color, x.parent, x.left, x.right);
            xParent.filePath = x.parent;
            readNodeFromFile(x.parent, xParent.key, xParent.rowData, xParent.color, xParent.filePath, xParent.left, xParent.right);

            Node xParentRight;
            xParentRight.filePath = xParent.right;
            readNodeFromFile(xParent.right, xParentRight.key, xParentRight.rowData, xParentRight.color, xParentRight.filePath, xParentRight.left, xParentRight.right);

            Node xParentLeft;
            xParentLeft.filePath = xParent.left;
            readNodeFromFile(xParent.left, xParentLeft.key, xParentLeft.rowData, xParentLeft.color, xParentLeft.filePath, xParentLeft.left, xParentLeft.right);

            // if x is left child of its parent           
            if (arePathsEqual(x.filePath, xParent.left)) {
                cout << "goinf inside" << endl;
                // w is right sibling of x
                Node w;
                w.filePath = xParent.right;
                readNodeFromFile(w.filePath, w.key, w.rowData, w.color, w.filePath, w.left, w.right);

                Node wRight;
                wRight.filePath = w.right;
                readNodeFromFile(wRight.filePath, wRight.key, wRight.rowData, wRight.color, wRight.filePath, wRight.left, wRight.right);

                Node wLeft;
                wLeft.filePath = w.left;
                readNodeFromFile(wLeft.filePath, wLeft.key, wLeft.rowData, wLeft.color, wLeft.filePath, wLeft.left, wLeft.right);

                // Type 1
                if (w.color == 'R') {
                    w.color = 'B';
                    xParent.color = 'R';
                    leftRotate(xParent.filePath);
                    // w = x->parent->right
                    writeNodeToFile(w.filePath, xParentRight.key, xParentRight.rowData, xParentRight.color, xParentRight.filePath, xParentRight.left, xParentRight.right);
                    // rename(w.filePath, xParent.right);
                     //w.filePath = xParent.right;
                }

                // Type 2
                if (wLeft.color == 'B' && wRight.color == 'B') {
                    w.color = 'R';
                    //x = x->parent;
                    writeNodeToFile(x.filePath, xParent.key, xParent.rowData, xParent.color, xParent.parent, xParent.left, xParent.right);
                    //x.filePath = xParent.filePath;
                    // reanme if needed
                }
                else {
                    // Type 3
                    if (wRight.color == 'B') {

                        wLeft.color = 'B';
                        w.color = 'R';
                        rightRotate(w.filePath);
                        // w = x->parent->right;
                        writeNodeToFile(w.filePath, xParentRight.key, xParentRight.rowData, xParentRight.color, xParentRight.filePath, xParentRight.left, xParentRight.right);
                        //  rename(w.filePath, xParent.right);
                        //  w.filePath = xParent.right;

                    }

                    // Type 4
                    w.color = xParent.color;
                    xParent.color = 'B';
                    wRight.color = 'B';
                    leftRotate(xParent.filePath);
                    //x = root;
                    // read the root
                    Node rootNode;
                    readNodeFromFile(rootFile, rootNode.key, rootNode.rowData, rootNode.color, rootNode.parent, rootNode.left, rootNode.right);
                    writeNodeToFile(x.filePath, rootNode.key, rootNode.rowData, rootNode.color, rootNode.parent, rootNode.left, rootNode.right);
                    // rename(x.filePath, rootFile);
                   // x.filePath = rootFile;
                }
            }
            else {
                cout << "else in" << endl;
                Node w;
                w.filePath = xParent.left;
                readNodeFromFile(w.filePath, w.key, w.rowData, w.color, w.filePath, w.left, w.right);

                Node wRight;
                wRight.filePath = w.right;
                readNodeFromFile(wRight.filePath, wRight.key, wRight.rowData, wRight.color, wRight.filePath, wRight.left, wRight.right);

                Node wLeft;
                wLeft.filePath = w.left;
                readNodeFromFile(wLeft.filePath, wLeft.key, wLeft.rowData, wLeft.color, wLeft.filePath, wLeft.left, wLeft.right);
                // Type 1
                if (w.color == 'R') {
                    w.color = 'B';
                    xParent.color = 'R';

                    rightRotate(xParent.filePath);
                    // w = x->parent->left;
                    writeNodeToFile(w.filePath, xParentLeft.key, xParentLeft.rowData, xParentLeft.color, xParentLeft.filePath, xParentLeft.left, xParentLeft.right);
                    // rename(w.filePath, xParent.left);
                     //w.filePath = xParent.left;
                }

                // Type 2
                if (wRight.color == 'B' && wLeft.color == 'B') {
                    w.color = 'R';
                    //x = x->parent;
                    writeNodeToFile(x.filePath, xParent.key, xParent.rowData, xParent.color, xParent.parent, xParent.left, xParent.right);
                    //x.filePath = xParent.filePath;
                    // reanme if needed
                }
                else {
                    // Type 3
                    if (wLeft.color == 'B') {
                        wRight.color = 'B';
                        w.color = 'R';
                        leftRotate(w.filePath);
                        // w =x->parent->left
                        writeNodeToFile(w.filePath, xParentLeft.key, xParentLeft.rowData, xParentLeft.color, xParentLeft.filePath, xParentLeft.left, xParentLeft.right);
                        // rename(w.filePath, xParent.left);
                         //w.filePath = xParent.left;
                    }

                    // Type 4
                    w.color = xParent.color;
                    xParent.color = 'B';
                    //  writeNodeToFile(xParent.filePath, xParent.key, xParent.color, xParent.parent, xParent.left, xParent.right);
                    wLeft.color = 'B';
                    //    writeNodeToFile(wLeft.filePath, wLeft.key, wLeft.color, wLeft.filePath, wLeft.left, wLeft.right);
                    cout << "mar jaOOO" << endl;
                    rightRotate(xParent.filePath);
                    Node rootNode;
                    readNodeFromFile(rootFile, rootNode.key, rootNode.rowData, rootNode.color, rootNode.parent, rootNode.left, rootNode.right);
                    writeNodeToFile(x.filePath, rootNode.key, rootNode.rowData, rootNode.color, rootNode.parent, rootNode.left, rootNode.right);
                    // rename(x.filePath, rootFile);
                   // x.filePath = rootFile;
                }
            }
        }
        // Ensure the color of the current node is black
        int xKey;
        char xColor;
        fs::path xParent, xLeft, xRight;
        char* xRowData = new char[1000] {'\0'};
        readNodeFromFile(x.filePath, xKey, xRowData, xColor, xParent, xLeft, xRight);
        xColor = 'B';
        writeNodeToFile(x.filePath, xKey, xRowData, xColor, xParent, xLeft, xRight);
    }

    void transplant(const fs::path& uFile, const fs::path& vFile) {
        Node uParent;
        Node u;
        u.filePath = uFile;
        readNodeFromFile(uFile, u.key, u.rowData, u.color, u.parent, u.left, u.right);
        uParent.filePath = u.parent;
        readNodeFromFile(u.parent, uParent.key, uParent.rowData, uParent.color, uParent.parent, uParent.left, uParent.right);
        if (uParent.filePath.empty()) {
            cout << "ur parent is null" << endl;
            rootFile = vFile;
        }
        else if (arePathsEqual(uFile, uParent.left)) {
            cout << "Going in" << endl;
            uParent.left = vFile;
            writeNodeToFile(u.parent, uParent.key, uParent.rowData, uParent.color, uParent.parent, uParent.left, uParent.right);
        }
        else {
            cout << "going in else hehe" << endl;
            uParent.right = vFile;
            writeNodeToFile(u.parent, uParent.key, uParent.rowData, uParent.color, uParent.parent, uParent.left, uParent.right);
        }
        if (!vFile.empty()) {
            cout << "v not empty" << endl;
            int vKey;
            char vColor;
            fs::path vParent, vLeft, vRight;
            char* vRowData = new char[1000] {'\0'};
            readNodeFromFile(vFile, vKey, vRowData, vColor, vParent, vLeft, vRight);
            writeNodeToFile(vFile, vKey, vRowData, vColor, uParent.filePath, vLeft, vRight);
        }
    }

    fs::path minimum(const fs::path& xFile) {
        Node currentFile;
        currentFile.filePath = xFile;
        Node file_x;
        file_x.filePath = xFile;
        if (!xFile.empty()) {
            readNodeFromFile(xFile, file_x.key, file_x.rowData, file_x.color, file_x.parent, file_x.left, file_x.right);
            writeNodeToFile(currentFile.filePath, file_x.key, file_x.rowData, file_x.color, file_x.parent, file_x.left, file_x.right);
        }
        while (!currentFile.filePath.empty() && !currentFile.left.empty()) {
            currentFile.filePath = currentFile.left;
        }
        /*
        yahan write ni hona chahiyeh left ka content cuurent file ky andr
        */
        readNodeFromFile(currentFile.filePath, currentFile.key, currentFile.rowData, currentFile.color, currentFile.parent, currentFile.left, currentFile.right);
        writeNodeToFile(currentFile.filePath, currentFile.key, currentFile.rowData, currentFile.color, currentFile.parent, currentFile.left, currentFile.right);
        return currentFile.filePath;
    }

    fs::path search(int key) {
        Node curr;
        curr.filePath = rootFile;
        while (!curr.filePath.empty() && key != curr.key) {
            readNodeFromFile(curr.filePath, curr.key, curr.rowData, curr.color, curr.parent, curr.left, curr.right);

            if (key < curr.key) {
                cout << "less than current key : " << curr.key << endl;
                curr.filePath = curr.left;
            }
            else if (key > curr.key) {
                cout << "greater than current key : " << curr.key << endl;
                curr.filePath = curr.right;
            }
            else {
                return curr.filePath;
            }
        }
        return curr.filePath;
    }
};