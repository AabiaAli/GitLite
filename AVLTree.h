#include "GlobalFunctions.h"
using namespace std;


void writeNodeToFile(const filesystem::path& filepath, int key, char* rowData, const filesystem::path& left, const filesystem::path& right, int height)
{
    ofstream file;
    file.open(filepath);

    if (file.is_open())
    {
        file.seekp(0, ios::beg); //go to the start of the file

        file << "Key: " << key << endl;
        file << "LeftChildPath: " << (left.empty() ? "NULL" : left.string()) << endl;
        file << "RightChildPath: " << (right.empty() ? "NULL" : right.string()) << endl;
        file << "Height: " << height << endl;
        file << "RowData: " << rowData << endl;
        file.close();
    }
    else
    {
        cout << "Unable to write to file " << filepath << endl;
    }
}

//overloaded only for duplicates
void writeNodeToFile(const filesystem::path& filepath, int key, char* rowData, const filesystem::path& left, const filesystem::path& right, int height, bool isDuplicate)
{
    ofstream file;
    //open file in append mode if it exists; otherwise, create a new one
    if (filesystem::exists(filepath))
        file.open(filepath, ios::app);
    else
        file.open(filepath);

    if (file.is_open()) //append duplicate valuye rows
    {
        file << "RowData: " << rowData << endl;
        file.close();
    }
    else
    {
        cout << "Unable to write to file " << filepath << endl;
    }
}


bool readNodeFromFile(const filesystem::path& filepath, int& key, char*& rowData, filesystem::path& left, filesystem::path& right, int& height)
{
    if (!filesystem::exists(filepath))
        return false;

    ifstream file(filepath);
    if (!file.is_open())
        return false;

    char line[256];
    bool isRowData = false;
    char rowDataAccumulated[10000] = ""; //combine row data for duplicates
    int currentRowDataLength = 0;  //track the length of accumulated data

    while (file.getline(line, sizeof(line)))
    {
        char temp[256];
        if (findSubstring(line, "Key:") == 0)
        {
            extractSubstring(temp, line, 5, stringLength(line) - 5);
            key = convertStringToInt(temp);
        }
        else if (findSubstring(line, "RowData:") == 0)
        {
            /* extractSubstring(temp, line, 9, stringLength(line) - 9);
             copyIn(rowData, temp);*/

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
        else if (findSubstring(line, "Height:") == 0)
        {
            extractSubstring(temp, line, 8, stringLength(line) - 8);
            height = convertStringToInt(temp);
        }
    }
    file.close();

    //copy in the data if duplicates
    if (stringLength(rowDataAccumulated) > 0)
    {
        // delete[] rowData;
        rowData = new char[stringLength(rowDataAccumulated) + 1];
        copyIn(rowData, rowDataAccumulated);
    }

    return true;
}

class AVLTree
{
public:
    filesystem::path rootFile; // Path to the root node's file
    //filesystem::path directoryPath;
    int hashChoice = 2; //default to sha

    AVLTree() : rootFile("") {}
    /* AVLTree(filesystem::path dirP)
     {
         rootFile = "";
         directoryPath = dirP;
     }

     void makeDirectory(filesystem::path dirP)
     {
         directoryPath = dirP;
         if (!filesystem::exists(directoryPath))
             filesystem::create_directories(directoryPath);
     }*/

    void insert(int key, char* originalValue, char* data)
    {
        rootFile = insertFile(rootFile, key, originalValue, data);
    }

    filesystem::path insertFile(const filesystem::path& nodeFile, int key, char* originalValue, char*& rowData)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
        {
            //create a new file for the node
            filesystem::path newNodeFile = convertToFilesystemPath(originalValue);
            writeNodeToFile(newNodeFile, key, rowData, "", "", 1);
            return newNodeFile;
        }

        //read existing node details
        int currentNodeKey, nodeHeight;
        filesystem::path leftPath, rightPath;
        char* existingRowData = new char[1000] {'\0'};

        readNodeFromFile(nodeFile, currentNodeKey, existingRowData, leftPath, rightPath, nodeHeight);

        if (key < currentNodeKey)
        {
            leftPath = insertFile(leftPath, key, originalValue, rowData);
        }
        else if (key > currentNodeKey)
        {
            rightPath = insertFile(rightPath, key, originalValue, rowData);
        }
        else
        {
            //duplicate key so append the row data
            writeNodeToFile(nodeFile, key, rowData, leftPath, rightPath, nodeHeight, true);
            return nodeFile;
        }

        //update heights and rotate
        int leftHeight = getHeight(leftPath);
        int rightHeight = getHeight(rightPath);
        int balanceFactor = rightHeight - leftHeight;
        int maxHeight;
        if (leftHeight > rightHeight)
            maxHeight = leftHeight;
        else
            maxHeight = rightHeight;

        //rotate based on BF
        if (balanceFactor > 1) //right-skewed
        {
            if (calculateBalanceFactor(rightPath) >= 0) //same sign, single left rotation
                return leftRotate(nodeFile, rightPath);
            else //opposite signs, right-left rotation
            {
                rightPath = rightRotate(rightPath, getLeftChild(rightPath));
                return leftRotate(nodeFile, rightPath);
            }
        }
        else if (balanceFactor < -1) //left-skewed
        {
            if (calculateBalanceFactor(leftPath) <= 0) //single rights
                return rightRotate(nodeFile, leftPath);
            else //left right rotation
            {
                leftPath = leftRotate(leftPath, getRightChild(leftPath));
                return rightRotate(nodeFile, leftPath);
            }
        }

        //update the current node file
        writeNodeToFile(nodeFile, currentNodeKey, existingRowData, leftPath, rightPath, 1 + maxHeight);
        return nodeFile;
    }

    //function to calculate the balance factor
    int calculateBalanceFactor(const filesystem::path& nodeFile)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
            return -1;
        return getHeight(getRightChild(nodeFile)) - getHeight(getLeftChild(nodeFile));
    }


    int getHeight(const filesystem::path& nodeFile)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
            return 0;

        int key, height;
        filesystem::path left, right;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, left, right, height);
        return height;
    }

    int getKey(const filesystem::path& nodeFile)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
            return 0;

        int key, height;
        filesystem::path left, right;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, left, right, height);
        return key;
    }

    filesystem::path getLeftChild(const filesystem::path& nodeFile)
    {
        int key, height;
        filesystem::path left, right;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, left, right, height);
        return left;
    }

    filesystem::path getRightChild(const filesystem::path& nodeFile)
    {
        int key, height;
        filesystem::path left, right;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, left, right, height);
        return right;
    }

    filesystem::path leftRotate(const filesystem::path& current, const filesystem::path& k)
    {
        //get attrubutes
        int currentKey, kKey, currentHeight, kHeight;
        filesystem::path currentLeft, currentRight, kLeft, kRight;
        char* currRowData = new char[1000] {'\0'};
        char* kRowData = new char[1000] {'\0'};

        readNodeFromFile(current, currentKey, currRowData, currentLeft, currentRight, currentHeight);
        readNodeFromFile(k, kKey, kRowData, kLeft, kRight, kHeight);

        //set left and right
        currentRight = kLeft;
        kLeft = current;

        writeNodeToFile(current, currentKey, currRowData, currentLeft, currentRight, 1 + maxValue(getHeight(currentLeft), getHeight(currentRight)));
        writeNodeToFile(k, kKey, kRowData, kLeft, kRight, 1 + maxValue(getHeight(kLeft), getHeight(kRight)));

        return k;
    }

    filesystem::path rightRotate(const filesystem::path& current, const filesystem::path& kFile)
    {
        int kKey, currentKey, kHeight, currentHeight;
        filesystem::path kLeft, kRight, currLeft, currRight;
        char* currRowData = new char[1000] {'\0'};
        char* kRowData = new char[1000] {'\0'};

        readNodeFromFile(kFile, kKey, kRowData, kLeft, kRight, kHeight);
        readNodeFromFile(current, currentKey, currRowData, currLeft, currRight, currentHeight);

        currLeft = kRight;
        kRight = current;

        writeNodeToFile(kFile, kKey, kRowData, kLeft, kRight, 1 + maxValue(getHeight(kLeft), getHeight(kRight)));
        writeNodeToFile(current, currentKey, currRowData, currLeft, currRight, 1 + maxValue(getHeight(currLeft), getHeight(currRight)));

        return kFile;
    }

    void deleteKey(int k)
    {
        deleteFile(rootFile, k);
    }

    filesystem::path deleteFile(filesystem::path nodeFile, int key) {
        bool case3 = false;

        if (nodeFile.empty() || !filesystem::exists(nodeFile)) {
            return nodeFile; // Node not found
        }

        int currentNodeKey, nodeHeight;
        filesystem::path leftPath, rightPath;
        filesystem::path successorPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, currentNodeKey, rowData, leftPath, rightPath, nodeHeight);

        // BST deletion (find the node to delete)
        if (key < currentNodeKey) {
            leftPath = deleteFile(leftPath, key); // Continue searching in the left subtree
        }
        else if (key > currentNodeKey) {
            rightPath = deleteFile(rightPath, key); // Continue searching in the right subtree
        }
        else {
            // Node to be deleted is found
            // Case 1: Node has no children (leaf node)
            if (leftPath.empty() && rightPath.empty()) {
                if (nodeFile == rootFile)
                {
                    rootFile.clear(); // Clear root if it's being deleted
                }
                filesystem::remove(nodeFile);  // Delete the actual file
                updatePathsAfterDeletion(rootFile, nodeFile, "NULL");
                return filesystem::path(); // Return an empty path (node is deleted)
            }
            // Case 2: Node has only one child
            else if (leftPath.empty()) {
                if (nodeFile == rootFile) {
                    rootFile = rightPath; // Update root to the right child
                }
                filesystem::remove(nodeFile);  // Delete the actual file
                updatePathsAfterDeletion(rootFile, nodeFile, rightPath);
                return rightPath; // Replace node with its right child
            }
            else if (rightPath.empty()) {
                if (nodeFile == rootFile) {
                    rootFile = leftPath; // Update root to the left child
                }
                filesystem::remove(nodeFile);  // Delete the actual file
                updatePathsAfterDeletion(rootFile, nodeFile, leftPath);
                return leftPath; // Replace node with its left child
            }
            // Case 3: Node has two children
            else {
                // Find the inorder successor (smallest in the right subtree)
                case3 = true;
                successorPath = getMinNode(rightPath);
                int successorKey, successorHeight;
                filesystem::path successorLeft, successorRight;
                char* successorData = new char[1000] {'\0'};
                readNodeFromFile(successorPath, successorKey, successorData, successorLeft, successorRight, successorHeight);
                /*  cout << "Current file being updated : " << nodeFile << endl;
                  cout << "Current key: " << currentNodeKey << endl;
                  cout << "successor's key : " << successorKey << endl;
                  cout << "left path : " << leftPath << endl;
                  cout << "right path : " << rightPath << endl;
                  cout << "successor height : " << successorHeight << endl;
                  cout << "successor data: " << successorData << endl;*/
                currentNodeKey = successorKey;


                // Copy the inorder successor's key to the current node
                writeNodeToFile(nodeFile, successorKey, successorData, leftPath, rightPath, successorHeight);
                copyIn(rowData, successorData);

                rightPath = deleteFile(rightPath, successorKey); // Delete the inorder successor
            }
        }

        //read again if paths uodated during deletion
        if (!case3)
            readNodeFromFile(nodeFile, currentNodeKey, rowData, leftPath, rightPath, nodeHeight);

        int maxHeight;
        int leftHeight = getHeight(leftPath);
        int rightHeight = getHeight(rightPath);
        if (leftHeight > rightHeight)
            maxHeight = leftHeight;
        else
            maxHeight = rightHeight;

        // Update the current node file after deletion
        writeNodeToFile(nodeFile, currentNodeKey, rowData, leftPath, rightPath, maxHeight + 1);

        if (case3) {
            try {
                updatePathsAfterDeletion(rootFile, nodeFile, successorPath);
                filesystem::rename(nodeFile, successorPath);
                // Update rootFile if the root node is being deleted and replaced
                if (nodeFile == rootFile) {
                    rootFile = successorPath;
                }
            }
            catch (const filesystem::filesystem_error& e) {
                std::cerr << "Error renaming file: " << e.what() << std::endl;
                // Handle the error accordingly
            }
        }

        // Balance the tree after deletion
        return balanceAfterDeletion(nodeFile, leftPath, rightPath);
    }

    void updatePathsAfterDeletion(filesystem::path root, const filesystem::path& oldPath, const filesystem::path& newPath)
    {
        if (root.empty() || !filesystem::exists(root))
            return;

        // Read the current node data
        int currentNodeKey, nodeHeight;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(root, currentNodeKey, rowData, leftPath, rightPath, nodeHeight);
        //  cout << "current left: " << leftPath << " and current right: " << rightPath << endl;
          //cout << "need to change " << oldPath << " to " << newPath << endl;
        if (leftPath == oldPath)
        {
            writeNodeToFile(root, currentNodeKey, rowData, newPath, rightPath, nodeHeight);
            leftPath = newPath;
        }
        if (rightPath == oldPath)
        {
            writeNodeToFile(root, currentNodeKey, rowData, leftPath, newPath, nodeHeight);
            rightPath = newPath;
        }

        //recur for the left and right subtrees
        updatePathsAfterDeletion(leftPath, oldPath, newPath);
        updatePathsAfterDeletion(rightPath, oldPath, newPath);

    }


    filesystem::path balanceAfterDeletion(filesystem::path& nodeFile, filesystem::path& leftPath, filesystem::path& rightPath) {
        int balanceFactor = calculateBalanceFactor(nodeFile);

        // Left heavy
        if (balanceFactor < -1) {
            if (calculateBalanceFactor(leftPath) > 0) { // Left-Right case
                leftPath = leftRotate(leftPath, getRightChild(leftPath));
            }
            return rightRotate(nodeFile, leftPath); // Left-Left case
        }

        // Right heavy
        if (balanceFactor > 1) {
            if (calculateBalanceFactor(rightPath) < 0) { // Right-Left case
                rightPath = rightRotate(rightPath, getLeftChild(rightPath));
            }
            return leftRotate(nodeFile, rightPath); // Right-Right case
        }

        return nodeFile; // No balancing needed
    }


    filesystem::path getMinNode(const filesystem::path& nodeFile)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile)) {
            return filesystem::path();
        }

        filesystem::path current = nodeFile;
        while (true) {
            filesystem::path leftChild = getLeftChild(current);
            if (leftChild.empty()) {
                return current; // Leftmost node
            }
            current = leftChild;
        }
    }

    filesystem::path returnKeyPath(int key)
    {
        return searchKey(rootFile, key);
    }

    filesystem::path searchKey(const filesystem::path& nodeFile, int key) //search function
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
            return filesystem::path();

        int currentNodeKey, height;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, currentNodeKey, rowData, leftPath, rightPath, height);

        if (key < currentNodeKey)
        {
            return searchKey(leftPath, key);
        }
        else if (key > currentNodeKey)
        {
            return searchKey(rightPath, key);
        }
        else //found, return path
        {
            return nodeFile;
        }
    }

    //hash the row in the avl file (used for data comparison
    char* getHash(const filesystem::path& nodeFile, int hashChoice)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
        {
            cout << nodeFile << " does not exist" << endl;
        }

        int key, height;
        filesystem::path left, right;
        char* rowData = new char[10000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, left, right, height);
        int index = findSubstring(rowData, "RowData:");
        if (index != -1) //duplicate values
        {
            char* firstoccurence = new char[index];
            int i;
            for (i = 0; i < index - 1; i++) //stop before newline
                firstoccurence[i] = rowData[i];
            firstoccurence[i] = '\0';
            delete[] rowData; //free the original array
            rowData = firstoccurence; //piint to the new truncated data
            // cout << "NEW DATA ABOUT TO BE HASHED: " << rowData << endl;
        }

        char hash[65]; //assume the hash will never be over 64 bits
        computeHash(hashChoice, rowData, hash, sizeof(hash));
        delete[] rowData;
        return hash;
    }

    char* getHashOfThisKey(int key)
    {
        filesystem::path nodePath = searchKey(rootFile, key);

        if (nodePath.empty())
        {
            cout << "Key " << key << " not found in the AVL tree." << endl;
            return nullptr;
        }

        return getHash(nodePath, hashChoice);
    }

    void getAllHashes(Queue<char*>& hashQueue)
    {
        filesystem::path directoryPath = filesystem::current_path();
        //check all files in the directory
        try {
            for (const auto& entry : filesystem::directory_iterator(directoryPath))
            {
                if (filesystem::is_regular_file(entry.status()))
                {
                    filesystem::path filePath = entry.path(); //get path of file

                    if (filePath.empty() || !filesystem::exists(filePath))
                    {
                        cout << filePath << " does not exist" << endl;
                    }

                    int key, height;
                    filesystem::path left, right;
                    char* rowData = new char[10000] {'\0'};
                    readNodeFromFile(filePath, key, rowData, left, right, height);

                    char target[] = "RowData: ";
                    int index = findSubstring(rowData, target);
                    int targetLength = stringLength(target);
                    char* start = rowData;
                    char* end;

                    //find occurrences of dupliactes
                    while ((end = findNeedle(start, target)) != nullptr)
                    {
                        //calculate the length of the data up to "RowData:"
                        int length = end - start;

                        //copy in
                        char* firstOccurrence = new char[length + 1];
                        int i;
                        for (i = 0; i < length; ++i)
                        {
                            firstOccurrence[i] = start[i];
                        }
                        if (i > 0 && firstOccurrence[i - 1] == '\n') //remove trailing newline when duplicates are found
                        {
                            firstOccurrence[i - 1] = '\0';
                        }
                        else
                        {
                            firstOccurrence[i] = '\0'; // nll-terminate
                        }

                        char hash[65];
                        computeHash(hashChoice, firstOccurrence, hash, sizeof(hash));
                        delete[] firstOccurrence;

                        if (hash != nullptr)
                        {
                            hashQueue.enqueue(hash);
                       //     cout << "avl: " << hash << endl;
                        }

                        //move the start pointer past the current "RowData: "
                        start = end + targetLength;
                    }

                    //process any remaining data after the last "RowData:"
                    if (*start != '\0') // Check if there is data after the last "RowData: "
                    {
                        char* remainingData = new char[stringLength(start) + 1];
                        copyIn(remainingData, start);
                        //duplicates have new lines
                        int remainingLength = stringLength(remainingData);
                        if (remainingLength > 0 && remainingData[remainingLength - 1] == '\n')
                            remainingData[remainingLength - 1] = '\0';
                        char hash[65];
                        computeHash(hashChoice, remainingData, hash, sizeof(hash));

                        delete[] remainingData;

                        if (hash != nullptr)
                        {
                            hashQueue.enqueue(hash);
                            cout << "avl: " << hash << endl;
                        }
                    }

                    delete[] rowData;

                }
            }
        }
        catch (const std::exception& e)
        {
            cerr << "Error occurred while accessing files: " << e.what() << endl;
        }
    }

    void getAllDataNodes(Queue<char*>& dataQueue)
    {
        getAllDataNodesHelper(rootFile, dataQueue);
    }

    void getAllDataNodesHelper(const filesystem::path& nodeFile, Queue<char*>& dataQueue)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
        {
            return;
        }

        int key, height;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[10000] {'\0'};

        //read the current node's data
        readNodeFromFile(nodeFile, key, rowData, leftPath, rightPath, height);

        //traverse left subtree
        getAllDataNodesHelper(leftPath, dataQueue);

        //add current node's data to the queue
        dataQueue.enqueue(rowData);

        //traverse right subtree
        getAllDataNodesHelper(rightPath, dataQueue);

        delete[] rowData;
    }

    void displayAllDataForFile(const filesystem::path& filePath)
    {
        ifstream inputFile(filePath);
        if (!inputFile.is_open())
        {
            cout << "Error: Could not open file for reading." << endl;
        }

        char line[256];
        while (inputFile.getline(line, sizeof(line)))
        {
            cout << line << endl;
        }

        inputFile.close();
    }

    void selectWithinRange(int lowerBound, int upperBound, const filesystem::path& selectedDataPath)
    {
        //store data in new file
        ofstream outputFile(selectedDataPath, ios::out | ios::trunc);
        if (!outputFile.is_open())
            cout << "Error: Could not open file for writing." << endl;

        selectWithinRangeHelper(rootFile, lowerBound, upperBound, selectedDataPath);

        outputFile.close();
    }

    void selectWithinRangeHelper(const filesystem::path& nodeFile, int lowerBound, int upperBound, const filesystem::path& selectedDataPath)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
        {
            return;
        }

        int key, height;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, leftPath, rightPath, height);

        int currentValue = convertStringToInt(nodeFile.string().c_str());

        //check if current node key is within range
        if (currentValue >= lowerBound && currentValue <= upperBound)
        {
            cout << "Within range: " << endl;
            displayAllDataForFile(nodeFile);
            writeNodeToFile(selectedDataPath, key, rowData, leftPath, rightPath, height);
        }

        //traverse
        if (key > lowerBound)
        {
            selectWithinRangeHelper(leftPath, lowerBound, upperBound, selectedDataPath);
        }
        if (key < upperBound)
        {
            selectWithinRangeHelper(rightPath, lowerBound, upperBound, selectedDataPath);
        }

        //delete[] rowData;
    }

    void deleteRecordsByAttribute( char*& target)
    {
        deleteRecordsByAttributeHelper(rootFile, target);
    }

    void deleteRecordsByAttributeHelper(const filesystem::path& nodeFile,  char*& target)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile)) {
            return;
        }

        int key, height;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, leftPath, rightPath, height);

        if (compareStrings(nodeFile.string().c_str(), target) == 0) //fike name is target, delete
        {
            cout << "Deleting all instances of " << target << "!" << endl;
            deleteFile(nodeFile, key);
            remove(nodeFile);
        }

        if (!leftPath.empty()) {
            deleteRecordsByAttributeHelper(leftPath, target);
        }
        if (!rightPath.empty()) {
            deleteRecordsByAttributeHelper(rightPath, target);
        }
    }

    void deleteFirstRecordByAttribute( char*& targetName)
    {
        deleteFirstRecordByAttributeHelper(rootFile, targetName);
    }

    void deleteFirstRecordByAttributeHelper(const filesystem::path& nodeFile,  char*& targetName)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
        {
            return;
        }

        int key, height;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, leftPath, rightPath, height);

        if (compareStrings(nodeFile.string().c_str(), targetName) == 0) //file name is target, delete
        {
            if (!(checkIfFileHasDuplicates(nodeFile))) // no duplicates so just delete the whole file
                deleteFile(nodeFile, key);
            else
            {
                //delete the first line
                cout << "Deleting the first instance of " << targetName << endl;
                int index = findSubstring(rowData, "RowData: ");
                char* newRowData = new char[stringLength(rowData) - index + 1]; //one for null
                copyIn(newRowData, rowData + index); //copy only the info after

                //delete (change the file)
                writeNodeToFile(nodeFile, key, newRowData, leftPath, rightPath, height);
                cout << "Deleted: ";
                for (int i = 0; i <= index; i++)
                    cout << rowData[i];
                cout << endl;
                delete[] newRowData;
            }
        }

        // Recur for left or right as needed
        deleteFirstRecordByAttributeHelper(leftPath, targetName);
        deleteFirstRecordByAttributeHelper(rightPath, targetName);

        delete[] rowData;
    }

    bool checkIfFileHasDuplicates(const filesystem::path& nodeFile)
    {
        if (nodeFile.empty() || !filesystem::exists(nodeFile))
            return false;

        int key, height;
        filesystem::path leftPath, rightPath;
        char* rowData = new char[1000] {'\0'};
        readNodeFromFile(nodeFile, key, rowData, leftPath, rightPath, height);

        int count = 0;
        const char* searchStr = "RowData: ";
        char* ptr = rowData;

        // Search for the substring and count occurrences
        while ((ptr = findNeedle(ptr, searchStr)) != nullptr)
        {
            count++;
            ptr += stringLength(searchStr); //move past the last found occurrence
        }

        delete[] rowData; //clean up dynamic memory

        //return true if more than one occurrence of "RowData: " is found
        return count > 0;
    }



};
