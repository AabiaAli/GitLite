#include "GlobalFunctions.h"
#include "AVLTree.h"
#include "MerkleTree.h"
#include "RedBlackTree.h"
using namespace std;
/*
ZARA NOOR 23I-0681
AABIA ALI 23I-0704
INSHARAH IRFAN 23I-0615
*/

int main()
{
   
    // //beauty full interface
    create_beauty();
    AVLTree** avlTree = new AVLTree * [15];
    int index = -1;
    RedBlackTree** redBlackTree = new RedBlackTree * [15];
    int rindex = 0;
    filesystem::path merkleRootNode;
    char* merkleRootHash = new char[65] {'\0'};
    Queue<filesystem::path> leafNodes;
    //COMMANDS

    char input1[256]; // Buffer for user input
    MerkleTree merkleTree;
   // std::cout << "\n     Welcome to Team Exception Thrown's GitLite!" << endl;

    int hashChoice;
    Queue<char*> changedHashes;
    filesystem::path avlDirectory;
    filesystem::path rbDirectory;
    Queue<char*> avlhashes;
    std::cout << "\n     Welcome to Team Exception Thrown's GitLite!" << endl;
    
        while (true) {
            std::cout << " -> "; // Prompt for input
            std::cin.getline(input1, 256); // Read the entire line

            if (strcmp(input1, "exit") == 0) {
                std::cout << "Exiting the program.\n";
                //break;
            }

            int command_no = executeCommand(input1); // Parse and execute the command
            if (command_no == 1) { // It is an init command, extract the filepath
                char* useless = handleInitCommand(input1);
                // Convert char* to std::filesystem::path
                std::filesystem::path filePath(useless);
                ifstream file(filePath);
                if (!file.is_open())
                    std::cout << "Could not open the file " << filePath << "." << endl;

                char line[1024]; // Buffer to store each line
                int lineSize = 1024;
                int numFields = 0;

                // Read and tokenize column headers
                if (file.getline(line, lineSize))
                {
                    char* temporaryTokens = tokenize(line, ",");
                    while (temporaryTokens)
                    {
                        numFields++;
                        temporaryTokens = tokenize(nullptr, ",");
                    }
                }
                else
                {
                    std::cout << "Could not read column names. Please try again." << endl;
                    file.close();
                    //return 1;
                }

                // Create array for columns based on the number of columns
                char** columns = new char* [numFields];
                for (int i = 0; i < numFields; i++)
                    columns[i] = new char[500]; // Assuming a single field is 500 characters max

                // Reset file pointer to start and store field names
                file.clear();
                file.seekg(0);
                if (file.getline(line, lineSize))
                {
                    int index = 0;
                    char* token = tokenize(line, ",");
                    while (token)
                    {
                        copyIn(columns[index], token); // Copy the strings into columns
                        index++;
                        token = tokenize(nullptr, ",");
                    }
                }

                std::cout << "------COLUMN NAMES------" << endl;
                for (int i = 0; i < numFields; i++) // Output columns for the user
                    std::cout << "Column #" << i + 1 << ": " << columns[i] << endl;

                int columnNum;
                do
                {
                    std::cout << "Enter the number of the column/field you want to make a tree for: ";
                    std::cin >> columnNum;
                    columnNum -= 1; // Convert to 0-based index
                    if (columnNum < 0 || columnNum >= numFields)
                        std::cout << "Invalid field number. Please try again." << endl;
                } while (columnNum < 0 || columnNum >= numFields);
                //   ---------MERKLE TREE
                 
                do
                {
                    std::cout << "Choose hash function:\n1. Instructor Hash\n2. SHA256 (RECOMMENDED, LESS DUPLICATE VALUES)\nEnter choice: ";
                    std::cin >> hashChoice;
                    if (hashChoice != 1 && hashChoice != 2)
                        cout << "Invalid choice. Please try again." << std::endl;
                } while (hashChoice != 1 && hashChoice != 2);

                filesystem::path treeDir = "MerkleTreeFiles";
                merkleTree.hashChoice = hashChoice;
                merkleTree.directoryPath = treeDir;
                if (!filesystem::exists(merkleTree.directoryPath))//create directory if we dont in main
                    filesystem::create_directory(merkleTree.directoryPath);
                //create leaf nodes queue
                leafNodes = merkleTree.createLeafNodes(filePath.string().c_str()); //convert to char array 
                if (leafNodes.is_empty())
                    cout << "No leaf nodes made for Merkle Tree" << std::endl;

                //build tree
                filesystem::path rootNode = merkleTree.buildMerkleTree(leafNodes);
                std::cout << "Merkle Tree built successfully. Root node: " << rootNode << std::endl;

                std::ifstream rootFile(rootNode);
                std::string rootHash;
                std::getline(rootFile, rootHash);
                copyIn(merkleRootHash, rootHash.substr(rootHash.find(":") + 2).c_str());
                std::cout << "Root Hash: " << rootHash.substr(rootHash.find(":") + 2) << std::endl;
                rootFile.close();

                //----------OTHER TREES
                int treeType;
                do
                {
                    std::cout << "1. AVL\n2. RED BLACK TREE\nEnter the tree you want to make: ";
                    std::cin >> treeType;
                    if (treeType < 0 || treeType > 3)
                        std::cout << "Invalid tree type, please try again." << endl;
                } while (treeType < 0 || treeType > 3);
                

                if (treeType == 1) {
                    avlTree[++index] = new AVLTree;
                  //  filesystem::path avlDirectory;
                    cout << "Enter the AVL Directory name: ";
                    cin >> avlDirectory;
                    if (!filesystem::exists(avlDirectory))
                        filesystem::create_directories(avlDirectory);


                    //save the current directory
                    filesystem::path originalDirectory = filesystem::current_path();
                    //------------------------------------------ENTERING THE AVL TREE DIRECTORY-------------------------------------------------------------
                    //go to new directoryy
                    filesystem::current_path(avlDirectory);

                    int lineNumber = 1;
                    // Process records from the CSV
                    while (file.getline(line, sizeof(line)))
                    {
                        lineNumber++;
                        char* token = tokenize(line, ",");
                        int currentFieldIndex = 0;
                        char fieldValue[500] = ""; // Adjust size as needed
                        char* rowData = new char[1024]; // Buffer for the complete row data
                        int rowDataIndex = 0;

                        // Extract the chosen field's value and construct the entire row data
                        while (token)
                        {
                            if (currentFieldIndex == columnNum)
                                copyIn(fieldValue, token);

                            // Append the token to the row data buffer
                            int tokenLength = stringLength(token);
                            copyIn(rowData + rowDataIndex, token);
                            rowDataIndex += tokenLength;
                            rowData[rowDataIndex] = ','; // Add a comma after each field
                            rowDataIndex++;

                            currentFieldIndex++;
                            token = tokenize(nullptr, ",");
                        }

                        // Remove the last comma from the row data if it exists
                        if (rowDataIndex > 0)
                            rowData[rowDataIndex - 1] = '\0';

                        if (fieldValue[0] == '\0')
                        {
                            std::cout << "Error: Field value is empty for line " << lineNumber << endl;
                            delete[] rowData; // Free the buffer
                            continue;
                        }

                        int hashKey = hashOurKey(rowData);
                        std::cout << "INSERTING: " << hashKey << " with value: " << fieldValue << endl;
                        avlTree[index][0].insert(hashKey, fieldValue, rowData);
                    }

                    //set hash choice here
                    avlTree[index][0].hashChoice = hashChoice;
                   
                    avlTree[index][0].getAllHashes(avlhashes);
                   // avlhashes.print();
                    //------------------------------------------EXITING THE AVL TREE DIRECTORY-------------------------------------------------------------
                    filesystem::current_path(originalDirectory); //return to original
                }
                else if (treeType == 2) {
                    redBlackTree[++rindex] = new RedBlackTree;
                   
                    cout << "Enter the RED BLACK Directory name: ";
                    cin >> rbDirectory;
                    if (!filesystem::exists(rbDirectory))
                        filesystem::create_directories(rbDirectory);


                    //save the current directory
                    filesystem::path originalDirectory = filesystem::current_path();
                    //------------------------------------------ENTERING THE AVL TREE DIRECTORY-------------------------------------------------------------
                    //go to new directoryy
                    filesystem::current_path(rbDirectory);

                    int lineNumber = 1;
                    // Process records from the CSV
                    while (file.getline(line, sizeof(line)))
                    {
                        lineNumber++;
                        char* token = tokenize(line, ",");
                        int currentFieldIndex = 0;
                        char fieldValue[500] = ""; // Adjust size as needed
                        char* rowData = new char[1024]; // Buffer for the complete row data
                        int rowDataIndex = 0;

                        // Extract the chosen field's value and construct the entire row data
                        while (token)
                        {
                            if (currentFieldIndex == columnNum)
                                copyIn(fieldValue, token);

                            // Append the token to the row data buffer
                            int tokenLength = stringLength(token);
                            copyIn(rowData + rowDataIndex, token);
                            rowDataIndex += tokenLength;
                            rowData[rowDataIndex] = ','; // Add a comma after each field
                            rowDataIndex++;

                            currentFieldIndex++;
                            token = tokenize(nullptr, ",");
                        }

                        // Remove the last comma from the row data if it exists
                        if (rowDataIndex > 0)
                            rowData[rowDataIndex - 1] = '\0';

                        if (fieldValue[0] == '\0')
                        {
                            std::cout << "Error: Field value is empty for line " << lineNumber << endl;
                            delete[] rowData; // Free the buffer
                            continue;
                        }

                        int hashKey = hashOurKey(rowData);
                        std::cout << "INSERTING: " << hashKey << " with value: " << fieldValue << endl;
                        char* f = fieldValue;
                        redBlackTree[rindex][0].insert(hashKey, f);
                    }

                    //set hash choice here
                    redBlackTree[rindex][0].hashChoice = hashChoice;
                    //Queue<char*> avlhashes;
                  //  redBlackTree[index][0].getAllHashes(avlhashes);
                    redBlackTree[rindex][0].printTree();
                    //------------------------------------------EXITING THE AVL TREE DIRECTORY-------------------------------------------------------------
                    filesystem::current_path(originalDirectory); //return to original
                }
            }
            else if (command_no == 2) { //create a branch
                char* branchname = handlebranchCommand(input1);
                cout << branchname;
                std::filesystem::path filePath(branchname);
                ifstream file(filePath);
                // Get the current working directory
                filesystem::path currentDir = filesystem::current_path();
                createNewFolder(branchname, avlDirectory);
            }
                else if (command_no == 3){ //commit
                    
                    filesystem::path log;
                    cout << "Enter the name of your log file: ";
                    cin >> log;

                    char* commitMsg = handleCommitCommand(input1);
                    logCommitAction(commitMsg, log);

                    merkleTree.synchronizeMerkleWithAVL(avlTree[index]);
                    cout << "Changes committed." << endl;

                   /* while (!changedHashes.is_empty())
                    {
                        char* hash = changedHashes.Front();
                        Queue<char*> merkleProof = merkleTree.generateMerkleProof(hash);
                        cout << "THE MERKLE TREE IS SEARCHING FOR CHANGES IN DATA..........\nCOMPUTING MERKLE PROOF PATH..." << endl;
                        if (!merkleTree.verifyMerkleProof(hash, merkleProof, merkleRootHash))
                        {
                            cout << "Data corruption detected. Changes were made in the tree. Repairs have begun..." << endl;
                            merkleTree.repairMerkleTree(hash);
                        }
                        else
                            cout << "Data committed. No changes were made." << endl;
                        changedHashes.dequeue();
                    }*/
             }
            else if (command_no == 4) { //checkout
                
               handleCheckoutCommand(input1);            
            }
            else if(command_no==5){ //delete branch
                handleDeleteBranchCommand(input1);
            }
            else if (command_no == 6) { //current branch
                displayCurrentBranch();
            }
            else if (command_no == 7) { //all branch
                handleBranchesCommand();
            }
            else if (command_no == 8) //delete
            {
                // cout << "Enter what you you want to delete (the data): ";
                char* data = new char[100];
                //cin.getline(data, 100);
                //cin.ignore();
                data = handleDeleteCommand(input1);
                filesystem::path nodePath = convertToFilesystemPath(data);
                bool found = false;
                fs::path currentDir = fs::current_path();
                try {
                    // Iterate over all entries in the directory
                    for (const auto& entry : fs::directory_iterator(currentDir)) {
                        // Check if the entry is a regular file and matches the target filename
                        if (fs::is_regular_file(entry.status()) && entry.path().filename() == nodePath) {
                            std::cout << "Found the file: " << entry.path() << std::endl;

                            // Assuming you have a way to get the hash and process it
                            char* hash = avlTree[index]->getHash(entry.path(), hashChoice);
                            avlTree[index]->deleteRecordsByAttribute(data);
                            changedHashes.enqueue(hash);

                            found = true;
                            break; // Exit the loop once the file is found
                        }
                    }

                    if (!found) {
                        std::cout << "The file '" << nodePath << "' was not found in the directory." << std::endl;
                    }
                }
                catch (const fs::filesystem_error& e) {
                    std::cerr << "Error accessing directory: " << e.what() << std::endl;
                }
  }
        }
        return 0;
    }
