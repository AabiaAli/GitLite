#pragma once
#include <iostream>
#include "GlobalFunctions.h"
using namespace std;


namespace fs = std::filesystem;

class MerkleTree
{
public:
    int hashChoice;
    filesystem::path directoryPath;
public:
    MerkleTree()
    {
        hashChoice = 2; //default to sha
    }

    MerkleTree(int c)
    {
        hashChoice = c;
    }

    MerkleTree(int c, const filesystem::path& dirPath) : hashChoice(c), directoryPath(dirPath)
    {
        if (!filesystem::exists(directoryPath)) //create directory if we dont in main
            filesystem::create_directory(directoryPath);
    }

    void computeHash(char* input, char* output, int outputSize)
    {
        if (hashChoice == 1)
            instructorHash(input, output, outputSize);
        else
            computeSHA256Hash(input, output, outputSize);
    }

    void extractHash(const char* line, char* hash)
    {
        const char* hashStart = findNeedle(line, "Hash: ");
        if (hashStart)
        {
            // Skip past "Hash: " (6 characters) to the start of the hash value
            hashStart += 6;
            copyIn(hash, hashStart);
            hash[64] = '\0';
        }
        else
        {
            hash[0] = '\0';
        }
    }


    void extractKey(const filesystem::path& filepath, int& key)
    {
        if (!filesystem::exists(filepath))
            return;

        ifstream file(filepath);
        if (!file.is_open())
            return;

        char line[256];
        while (file.getline(line, sizeof(line)))
        {
            if (findSubstring(line, "Key:") == 0)
            {
                char temp[256];
                extractSubstring(temp, line, 5, stringLength(line) - 5);
                key = convertStringToInt(temp);
                break;
            }
        }
        file.close();
    }

    void writeNodeToFile(const filesystem::path& filepath, const char* hash, int key, const filesystem::path& left, const filesystem::path& right, const filesystem::path& parent)
    {
        //move file inside merkle directory
        filesystem::path fullPath = directoryPath / filepath;
        ofstream file(filepath);
        if (!file.is_open())
            cout << "Failed to open file: " << filepath << endl;

        file << "Hash: " << hash << endl;
        file << "LeftChild: " << (left.empty() ? "" : left.string()) << endl;
        file << "RightChild: " << (right.empty() == 0 ? "" : right.string()) << endl;
        file << "Key: " << key << endl;
        file << "Parent: " << (parent.empty() ? "" : parent.string()) << endl;
        file.close();
    }

    bool readNodeFromFile(const filesystem::path& filepath, char* hash, int& key, filesystem::path& left, filesystem::path& right, filesystem::path& parent)
    {
        if (!filesystem::exists(filepath))
            return false;

        ifstream file(filepath);
        if (!file.is_open())
            return false;

        char line[256];
        while (file.getline(line, sizeof(line)))
        {
            char temp[256];
            if (findSubstring(line, "Key:") == 0)
            {
                extractSubstring(temp, line, 5, stringLength(line) - 5);
                key = convertStringToInt(temp);
            }
            else if (findSubstring(line, "Hash:") == 0)
            {
                extractSubstring(temp, line, 6, stringLength(line) - 6); // Start after "Hash: "
                copyIn(hash, temp);
            }
            else if (findSubstring(line, "LeftChild:") == 0)
            {
                extractSubstring(temp, line, 11, stringLength(line) - 11); // Start after "LeftChild: "
                left = (compareStrings(temp, "") == 0) ? filesystem::path() : filesystem::path(temp);
            }
            else if (findSubstring(line, "RightChild:") == 0)
            {
                extractSubstring(temp, line, 12, stringLength(line) - 12); // Start after "RightChild: "
                right = (compareStrings(temp, "") == 0) ? filesystem::path() : filesystem::path(temp);
            }
            else if (findSubstring(line, "Parent:") == 0)
            {
                extractSubstring(temp, line, 8, stringLength(line) - 8); // Start after "Parent: "
                parent = (compareStrings(temp, "") == 0) ? filesystem::path() : filesystem::path(temp);
            }
        }
        file.close();
        return true;
    }



    //helper function for making file path
    filesystem::path generateNodeFilePath(const char* nodeName)
    {
        char filePath[512];
        snprintf(filePath, sizeof(filePath), "%s.txt", nodeName); // file name is node 
        return  directoryPath / filePath; //combine directory path with name so file ends up in right directory
    }


    //build the tree
    filesystem::path buildMerkleTree(Queue<filesystem::path>& leafNodes)
    {
        while (leafNodes.size() > 1)
        {
            filesystem::path left = leafNodes.Front();
            leafNodes.dequeue();
            filesystem::path right = leafNodes.Front();
            leafNodes.dequeue();

            //read hashes
            char leftHash[65] = { 0 }, rightHash[65] = { 0 }; //64 characters and one null
            char temp[256];
            int leftKey = -1, rightKey = -1;
            ifstream leftFile(left), rightFile(right);
            if (leftFile.getline(temp, sizeof(temp)))
            {
                extractHash(temp, leftHash);
                extractKey(left, leftKey);
            }
            if (rightFile.getline(temp, sizeof(temp)))
            {
                extractHash(temp, rightHash);
                extractKey(right, rightKey); // Custom function to extract key from the node file
            }
            leftFile.close();
            rightFile.close();

            //combine hashes to generate parent
            char combinedInput[130] = { 0 }, combinedHash[65] = { 0 };
            snprintf(combinedInput, sizeof(combinedInput), "%s%s", leftHash, rightHash);
            computeHash(combinedInput, combinedHash, sizeof(combinedHash));

            int parentKey = -1;
            filesystem::path parentFile = generateNodeFilePath(combinedHash);
            writeNodeToFile(parentFile, combinedHash, parentKey, left, right, filesystem::path()); //empty parent for every "root" produced

            leafNodes.enqueue(parentFile);

            // Update child files with parent path
            updateParentPath(left, parentFile);
            updateParentPath(right, parentFile);
        }
        return leafNodes.Front(); //return the root
    }

    void updateParentPath(const filesystem::path& childFile, const filesystem::path& parentFile)
    {
        char hash[65] = { 0 };
        filesystem::path left, right, parent;
        int key = -1;
        if (!readNodeFromFile(childFile, hash, key, left, right, parent))
        {
            cout << "Failed to read child file: " << childFile << endl;
            return;
        }
        writeNodeToFile(childFile, hash, key, left, right, parentFile);
    }

    //read csv file to create the lowest level of the tree
    Queue<filesystem::path> createLeafNodes(const char* csvFilePath)
    {
        Queue<filesystem::path> leafNodes;
        ifstream csvFile(csvFilePath);

        if (!csvFile.is_open())
            cout << "Failed to open CSV file: " << csvFilePath << endl;

        char line[256];
        while (csvFile.getline(line, sizeof(line)))
        {
            char hash[65];
            computeHash(line, hash, sizeof(hash)); //row wise hashing
            filesystem::path leafFile = generateNodeFilePath(hash);
            int key = hashOurKey(line);
            writeNodeToFile(leafFile, hash, key, filesystem::path(), filesystem::path(), filesystem::path()); //empty leaf node paths
            leafNodes.enqueue(leafFile); //add leaf to queue
        }
        csvFile.close();
        return leafNodes;
    }

    char* getHash(const filesystem::path& filePath)
    {
        if (!filesystem::exists(filePath) || filePath.empty())
        {
            cout << "File does not exist" << endl;
            return nullptr;
        }

        std::ifstream file(filePath);
        char line[256];
        if (file.is_open())
        {
            file.getline(line, sizeof(line));
        }

        file.close();

        // extract hash
        char hash[65];
        extractHash(line, hash);
        return hash;
    }

    filesystem::path findNodeFile(const std::string& hash)
    {
        try
        {
            for (const auto& entry : fs::directory_iterator(directoryPath))
            {
                if (entry.is_regular_file())
                {
                    if (entry.path().filename().string().find(hash) != std::string::npos)
                    {
                        return entry.path(); //pagth found
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "General error: " << e.what() << std::endl;
        }

        return fs::path();
    }


    void getAllHashes(Queue<char*>& hashList)
    {
        for (const auto& entry : filesystem::directory_iterator(directoryPath))
        {
            if (filesystem::is_regular_file(entry.status()))
            {
                char hash[65] = { 0 };
                char line[256];
                ifstream file(entry.path());
                if (file.is_open() && file.getline(line, sizeof(line)))
                {
                    extractHash(line, hash);
                    hashList.enqueue(hash);
                    //  cout << "current merkle hash: " << hash << endl;
                }
                file.close();
            }
        }
    }

    //generate the merkle proof path based on a certain hash in the leaf node
    Queue<char*> generateMerkleProof(const char* targetHash)
    {
        Queue<char*> proofQueue;

        filesystem::path currentPath; //find the leaf node
        for (const auto& entry : filesystem::directory_iterator(directoryPath))
        {
            if (filesystem::is_regular_file(entry.status()))
            {
                char hash[65] = { 0 };
                char line[256];
                ifstream file(entry.path());
                if (file.is_open() && file.getline(line, sizeof(line)))
                {
                    extractHash(line, hash);
                    if (compareStrings(hash, targetHash) == 0)
                    {
                        currentPath = entry.path();
                        break; //found, start making the path
                    }
                }
                file.close();
            }
        }

        if (currentPath.empty())
        {
            cout << "Target hash not found in the leaf nodes." << endl;
            return proofQueue; //return empty proof if hash is not found
        }

        while (!currentPath.empty()) //go to root and collect sublings
        {
            char hash[65] = { 0 };
            filesystem::path left, right, parent;
            int key;
            if (readNodeFromFile(currentPath, hash, key, left, right, parent))
            {
                if (!parent.empty())
                {
                    //check which child we are and add the sibling's hash to the proof
                    if (currentPath == left)
                    {
                        char siblingHash[65] = { 0 };
                        if (readNodeFromFile(right, siblingHash, key, left, right, parent))
                        {
                            proofQueue.enqueue(siblingHash); // enqueue sibling hash
                        }
                    }
                    else if (currentPath == right)
                    {
                        char siblingHash[65] = { 0 };
                        if (readNodeFromFile(left, siblingHash, key, left, right, parent))
                        {
                            proofQueue.enqueue(siblingHash); //ennqueue sibling hash
                        }
                    }
                }
                currentPath = parent; //move up to the parent
            }
            else
            {
                cout << "Failed to read node from file: " << currentPath << endl;
                break;
            }
        }

        return proofQueue;
    }

    //compute the hashes to chek against the root
    bool verifyMerkleProof(const char* targetHash, Queue<char*>& proofPath, const char* rootHash)
    {
        char currentHash[65];
        copyIn(currentHash, targetHash); // Start with the target hash

        while (!proofPath.is_empty())
        {
            char* siblingHash = proofPath.Front();
            proofPath.dequeue();

            //concatenate hashes
            char combinedInput[130] = { 0 };
            if (compareStrings(currentHash, siblingHash) < 0) //go in the correct order
            {
                snprintf(combinedInput, sizeof(combinedInput), "%s%s", currentHash, siblingHash);
            }
            else
            {
                snprintf(combinedInput, sizeof(combinedInput), "%s%s", siblingHash, currentHash);
            }

            //compute hash
            computeHash(combinedInput, currentHash, sizeof(currentHash));
        }

        //compare computed with our root
        return (compareStrings(currentHash, rootHash) == 0);
    }

    //find the hash that causes the discrepency
    void locateDiscrepancy(const char* targetHash, const char* rootHash)
    {
        Queue<char*> proofPath = generateMerkleProof(targetHash); //get the proof path
        char currentHash[65];
        copyIn(currentHash, targetHash); //start with the target hash
        int key;
        filesystem::path currentPath;

        for (const auto& entry : filesystem::directory_iterator(directoryPath))
        {
            char hash[65] = { 0 };
            char line[256];
            ifstream file(entry.path());
            if (file.is_open() && file.getline(line, sizeof(line)))
            {
                extractHash(line, hash);
                if (compareStrings(hash, targetHash) == 0)
                {
                    currentPath = entry.path();
                    break;
                }
            }
            file.close();
        }

        if (currentPath.empty())
        {
            cout << "Target hash not found in the tree." << endl;
            return;
        }

        //traverse and find
        while (!proofPath.is_empty())
        {
            char* siblingHash = proofPath.Front();
            proofPath.dequeue();

            char combinedInput[130] = { 0 };
            if (compareStrings(currentHash, siblingHash) < 0)
            {
                snprintf(combinedInput, sizeof(combinedInput), "%s%s", currentHash, siblingHash);
            }
            else
            {
                snprintf(combinedInput, sizeof(combinedInput), "%s%s", siblingHash, currentHash);
            }

            char computedHash[65] = { 0 };
            computeHash(combinedInput, computedHash, sizeof(computedHash));

            //check against parent
            char storedHash[65] = { 0 };
            filesystem::path left, right, parent;
            if (!readNodeFromFile(currentPath, storedHash, key, left, right, parent) ||
                compareStrings(storedHash, computedHash) != 0)
            {
                cout << "Discrepancy found at node: " << currentPath << endl;
                break;
            }

            currentPath = parent; //move up to the parent node
            copyIn(currentHash, computedHash);
        }

        if (compareStrings(currentHash, rootHash) == 0)
        {
            cout << "Tree integrity verified up to root." << endl;
        }
        else
        {
            cout << "Root hash mismatch detected. Data corrupted!!!!" << endl;
        }
    }


    void repairMerkleTree(const char* targetHash)
    {
        filesystem::path currentPath;

        //find the leaf node
        for (const auto& entry : filesystem::directory_iterator(directoryPath))
        {
            char hash[65] = { 0 };
            char line[256];
            ifstream file(entry.path());
            if (file.is_open() && file.getline(line, sizeof(line)))
            {
                extractHash(line, hash);
                if (compareStrings(hash, targetHash) == 0)
                {
                    currentPath = entry.path();
                    break;
                }
            }
            file.close();
        }

        if (currentPath.empty())
        {
            cout << "Target hash not found in the tree." << endl;
            return;
        }

        //recompute hashes and update the tree upwards to the root
        while (!currentPath.empty())
        {
            char hash[65] = { 0 };
            filesystem::path left, right, parent;
            int key;
            if (readNodeFromFile(currentPath, hash, key, left, right, parent))
            {
                char leftHash[65] = { 0 }, rightHash[65] = { 0 };

                // Read and recompute child hashes if applicable
                if (!left.empty())
                {
                    readNodeFromFile(left, leftHash, key, left, right, parent);
                }
                if (!right.empty())
                {
                    readNodeFromFile(right, rightHash, key, left, right, parent);
                }

                //combine child hashes to compute current hash
                char combinedInput[130] = { 0 };
                snprintf(combinedInput, sizeof(combinedInput), "%s%s", leftHash, rightHash);
                computeHash(combinedInput, hash, sizeof(hash));

                //upate the current node with the new hash
                writeNodeToFile(currentPath, hash, key, left, right, parent);

                currentPath = parent; //move to the parent node
            }
            else
            {
                cout << "Failed to read node from file: " << currentPath << endl;
                break;
            }
        }
    }

    int getMerkleKey(const filesystem::path& nodeFilePath)
    {
        ifstream file(nodeFilePath);
        if (!file.is_open())
            cout << "Failed to open file: " << nodeFilePath << endl;

        char line[256];
        char hash[65] = { 0 };
        int key = -1;
        filesystem::path left, right, parent;
        readNodeFromFile(nodeFilePath, hash, key, left, right, parent);

        file.close();
        return key;
    }


    void synchronizeMerkleWithAVL(AVLTree* avlTree)
    {
        Queue<char*> avlDataNodes;
        avlTree->getAllDataNodes(avlDataNodes); // Returns data in order
        Queue<char*> merkleDataNodes;
        getAllHashes(merkleDataNodes);

        while (!avlDataNodes.is_empty())
        {
            char* dataNode = avlDataNodes.Front();
            avlDataNodes.dequeue();

            bool nodeUpdated = false; // Flag to check if the node was updated

            // Check if the current data node exists in the Merkle tree
            for (const auto& entry : filesystem::directory_iterator(directoryPath))
            {
                if (filesystem::is_regular_file(entry.status()))
                {
                    char hash[65] = { 0 };
                    char line[256];
                    ifstream file(entry.path());
                    if (file.is_open() && file.getline(line, sizeof(line)))
                    {
                        extractHash(line, hash);

                        //check if the current hash matches the AVL data node

                        if (compareStrings(hash, dataNode) == 0)
                        {
                            int leafKey = hashOurKey(dataNode);
                            // check if the key in AVL matches the key in the Merkle leaf node
                            if (leafKey == getMerkleKey(entry.path()))
                            {
                                // Get the correct hash from AVL
                                char* avlHash = avlTree->getHash(entry.path(), hashChoice);
                                if (avlHash)
                                {
                                    // Update the Merkle tree node's hash and filename to match AVL
                                    writeNodeToFile(entry.path(), avlHash, leafKey, filesystem::path(), filesystem::path(), filesystem::path());
                                    nodeUpdated = true;
                                    //  break; // Stop iterating as we've updated the node
                                }
                            }
                        }
                    }
                    file.close();
                }
            }

            if (!nodeUpdated) // If no matching node was found and updated, add it as a new leaf
            {
                char hash[65];
                computeHash(dataNode, hash, sizeof(hash));
                int leafKey = hashOurKey(dataNode);
                // Add new leaf to Merkle tree
                filesystem::path newLeaf = generateNodeFilePath(hash);
                writeNodeToFile(newLeaf, hash, leafKey, filesystem::path(), filesystem::path(), filesystem::path());
            }
        }

        // Remove deleted nodes
        while (!merkleDataNodes.is_empty())
        {
            char* hash = merkleDataNodes.Front();
            merkleDataNodes.dequeue();

            if (!avlDataNodes.contains(hash))
            {
                filesystem::path nodeFile = findNodeFile(hash);
                filesystem::remove(nodeFile); // Remove the file
            }
        }
    }



    //void synchronizeMerkleWithAVL(AVLTree* avlTree)
    //{
    //    Queue<char*> avlDataNodes;
    //    avlTree->getAllDataNodes(avlDataNodes); // Returns data in order
    //    Queue<char*> merkleDataNodes;
    //    getAllHashes(merkleDataNodes);

    //    while (!avlDataNodes.is_empty())
    //    {
    //        char* dataNode = avlDataNodes.Front();
    //        avlDataNodes.dequeue();

    //        if (!merkleDataNodes.contains(dataNode)) //new data node was inseeted
    //        {
    //            //check if their keys match, then it is an updated old data node, not a new one
    //            //keys dont match
    //            char hash[65];
    //            computeHash(dataNode, hash, sizeof(hash));
    //            int leafkey = hashOurKey(dataNode);
    //            //add new leaf to merkle tree
    //            filesystem::path newLeaf = generateNodeFilePath(hash);
    //            writeNodeToFile(newLeaf, hash, leafkey, filesystem::path(), filesystem::path(), filesystem::path());
    //            //keys match
    //        }
    //    }

    //    //remove deleted nodes
    //    while (!merkleDataNodes.is_empty())
    //    {
    //        char* hash = merkleDataNodes.Front();
    //        merkleDataNodes.dequeue();

    //        if (!avlDataNodes.contains(hash))
    //        {
    //            filesystem::path nodeFile = findNodeFile(hash); 
    //            filesystem::remove(nodeFile); // Remove the file
    //        }
    //    }

    //    //update existing leaf nodes' hashes that changed cause the data changed

    //}



};