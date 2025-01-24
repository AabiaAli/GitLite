#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Queue.h"
#include "SHA256.h" //not my library
using namespace std;
namespace fs = filesystem;

// Custom implementation of a safe string copy function
void copyS(char* dest, size_t destSize, const char* src, size_t maxCount) {
    if (dest == nullptr || src == nullptr) {
        return; // Handle null pointers gracefully
    }

    // Ensure we do not copy more than the destination size minus one for null termination
    size_t copyCount = (maxCount < destSize - 1) ? maxCount : (destSize - 1);

    // Copy the content from src to dest up to the copyCount or until we reach the null character
    for (size_t i = 0; i < copyCount; ++i) {
        dest[i] = src[i];
        if (src[i] == '\0') {
            break; // Stop copying if we reach the end of the source string
        }
    }

    // Null-terminate the destination buffer
    dest[copyCount] = '\0';
}
// Custom function to compare two strings up to 'num' characters
int strcompare(const char* str1, const char* str2, size_t num) {
    size_t i = 0;

    while (i < num) {
        // If we reach the end of either string, return the difference
        if (str1[i] == '\0' || str2[i] == '\0') {
            return static_cast<int>(str1[i]) - static_cast<int>(str2[i]);
        }

        // If characters are different, return their difference
        if (str1[i] != str2[i]) {
            return static_cast<int>(str1[i]) - static_cast<int>(str2[i]);
        }

        ++i;
    }

    // If we reach here, the first 'num' characters are equal
    return 0;
}
// Custom function to find the first occurrence of a character in a string
const char* strChar(const char* str, char ch) {
    while (*str != '\0') {
        if (*str == ch) {
            return str;  // Return a pointer to the first occurrence
        }
        ++str;
    }

    // If the character wasn't found, return nullptr
    return nullptr;
}
//calculate the length of a char array
int stringLength(char* str)
{
    int length = 0;
    while (str[length] != '\0') 
        length++;
    return length;
}
//overloaded
int stringLength(const char* str)
{
    int length = 0;
    while (str[length] != '\0')
        length++;
    return length;
}

//copy two char arrays
void copyIn(char* copy, const char* paste) 
{
    int i = 0;
    while (paste[i] != '\0')
    {
        copy[i] = paste[i];
        i++;
    }
    copy[i] = '\0'; //null terminate it
}

//comapre two char arrays
int compareStrings(const char* arr1, const char* arr2) 
{
    int i = 0;
    while (arr1[i] != '\0' && arr2[i] != '\0') 
    {
        if (arr1[i] != arr2[i]) 
            return arr1[i] - arr2[i];
        i++;
    }
    return arr1[i] - arr2[i];
}

//concatenate two arrays
char* concatenate(const char* first, const char* second)
{
    int length1 = stringLength(first);
    int length2 = stringLength(second);
    char* result = new char[length1 + length2 + 1]; // +1 for null terminator

    //copy the first in
    int i = 0;
    while (i < length1)
    {
        result[i] = first[i];
        i++;
    }

    //copy second in
    int j = 0;
    while (j < length2)
    {
        result[i] = second[j];
        i++;
        j++;
    }
    //null-terminate the result
    result[i] = '\0';
    return result;
}
//tokenize an array
char* tokenize(char* arr, const char* delimiter)
{
    static char* nextToken = nullptr;
    if (arr)
        nextToken = arr;
    if (!nextToken || *nextToken == '\0')
        return nullptr;

    char* tokenStart = nextToken;
    bool inQuotes = false;

    while (*nextToken != '\0')
    {
        if (*nextToken == '"')
        {
            inQuotes = !inQuotes; //toggle quotes
        }
        else if (!inQuotes)
        {
            for (int i = 0; delimiter[i] != '\0'; i++)
            {
                if (*nextToken == delimiter[i])
                {
                    *nextToken = '\0'; //replace the delimiter with null character
                    nextToken++;
                    //skip spaces after the delimiter
                    while (*nextToken == ' ')
                        nextToken++;
                    return tokenStart;
                }
            }
        }
        nextToken++;
    }

    nextToken = nullptr;
    return tokenStart;
}

//convert string to integer
int convertStringToInt(const char* str) 
{
    int num = 0, i = 0;
    bool negativeNum = false;

    if (str[0] == '-') //negative num
    { 
        negativeNum = true;
        i++;
    }

    while (str[i] != '\0')
    {
        num = num * 10 + (str[i] - '0'); //convert char to int
        i++;
    }

    if (negativeNum)
        return -num;
    else
        return num;
}

//convert an integer to a character array
void convertIntToString(int n, char* result) 
{
    int i = 0;
    bool negativeNum = false;

    if (n < 0) 
    {
        negativeNum = true;
        n = -n;
    }

    do 
    {
        result[i++] = (n % 10) + '0';
        n /= 10;
    } 
    while (n > 0);

    if (negativeNum) 
        result[i++] = '-';

    result[i] = '\0';

    //reverse result
    for (int j = 0, k = i - 1; j < k; j++, k--)
    {
        char temp = result[j];
        result[j] = result[k];
        result[k] = temp;
    }
}

//compute max
int maxValue(int one, int two)
{
    if (one > two)
        return one;
    return two;
}


//extract substring from character array
void extractSubstring(char* substring, const char* source, int start, int length) 
{
    int i = 0;
    while (i < length && source[start + i] != '\0') 
    {
        substring[i] = source[start + i];
        i++;
    }
    substring[i] = '\0';
}

//find substring but return a char pointer
char* findNeedle(const char* stringFindingIn, const char* targetToFind)
{
    if (!stringFindingIn || !targetToFind || *targetToFind == '\0')
        return nullptr; 

    for (const char* h = stringFindingIn; *h != '\0'; ++h)
    {
        const char* hay = h;
        const char* need = targetToFind;

        while (*hay != '\0' && *need != '\0' && *hay == *need)
        {
            ++hay;
            ++need;
        }

        if (*need == '\0') //match found
            return const_cast<char*>(h); //cast to non-const
    }

    return nullptr;
}


//find substring
int findSubstring(const char* str, const char* substring)
{
    int length = stringLength(str);
    int subLength = stringLength(substring);

    for (int i = 0; i <= length - subLength; i++) 
    {
        bool match = true;
        for (int j = 0; j < subLength; j++) 
        {
            if (str[i + j] != substring[j]) 
            {
                match = false;
                break;
            }
        }
        if (match)
            return i; //return index of the first occurrence
    }
    return -1; //not found
}

//helper function to remove consecutive duplicates for instructor hash
void removeConsecutiveDuplicates(const char* input, char* output)
{
    int outputIndex = 0;
    for (int i = 0; input[i] != '\0'; i++)
    {
        if (i == 0 || input[i] != input[i - 1])  // copy current characterif different from prev
            output[outputIndex++] = input[i];
    }
    output[outputIndex] = '\0'; //null-terminate
}

//convert a const char array into a filesystem object
filesystem::path convertToFilesystemPath(char* filename)
{
    char* newFileName = concatenate(filename, ".txt"); //append .txt
    //convert
    std::filesystem::path newPath(newFileName);
    //free the allocated memory
    delete[] newFileName;
    return newPath;
}

//convert a string to a char array for sha 256)
char* convertStringToCharArray(string i)
{
    int length = 0;
    while (i[length] != '\0')
        length++;
    char* arr = new char[length + 1]; //for null
    int index = 0;
    while (i[index] != '\0')
    {
        arr[index] = i[index];
        index++;
    }
    arr[index] = '\0'; //null terminate it
    return arr;
}

//instructor hash any char array
int instructorHash(char* str) 
{
    int hash = str[0];
    int i = 1;
    while (str[i] != '\0') 
    {
        hash = hash * str[i];
        hash %= 29; //take modulo to prevent incredibly long numbers
        i++;
    }
    if (hash < 0)
        hash *= -1;
    return hash;
}

//polynomial rolling hash used for our keys for consistency!
int hashOurKey(char* s) 
{
    int p = 31; // commonly used small prime number for p
    //long long m = 1e9 + 9; // large prime number for m (close to 32-bit limit)
    int m = 701; //largest 3 digit prime
    int hashValue = 0;
    int pPower = 1; // p^0 initially

    for (int i = 0; s[i] != '\0'; ++i)
    {
        hashValue = (hashValue + (s[i] * pPower) % m) % m;
        pPower = (pPower * p) % m; // Update p^i for the next character
    }

    return hashValue;
}

//int to char array i.e. 11 -> "11"
void intToCharArray(int value, char* output, int outputSize)
{
    if (outputSize <= 0)
        return; //no space

    char temp[64]; //store characters
    int i = 0;

    //0 logic diff
    if (value == 0) 
    {
        output[0] = '0';
        output[1] = '\0';
        return;
    }

    //convert integer to decimal string in reverse order
    while (value > 0)
    {
        int remainder = value % 10;
        temp[i++] = '0' + remainder; //convert to '0'-'9'
        value /= 10;
    }

    //reverse the string to get the correct order
    int j = 0;
    while (i > 0)
    {
        output[j++] = temp[--i];
    }

    //null-terminate the output
    output[j] = '\0';
}

//instructor hash function that makes a character array (like sha 256)
void instructorHash(char* str, char* hashOutput, int outputSize) 
{
    int hash = str[0];
    int i = 1;
    while (str[i] != '\0') 
    {
        hash = hash * str[i];
        hash %= 29; //take mod to prevent incredibly long numbers
        i++;
    }
    if (hash < 0)
        hash *= -1;

    intToCharArray(hash, hashOutput, outputSize); //convert
}

void computeSHA256Hash(const char* input, char* output, int outputSize)
{
    SHA256 sha256;
    string hashString = sha256((string)input); // forced to use strings for sha256 library
    char* hash = convertStringToCharArray(hashString);
    copyIn(output, hash);
    output[outputSize - 1] = '\0'; //jiust in case
    delete[] hash;
}

void computeHash(int choice, char* input, char* output, int outputSize)
{
    if (choice == 1)
        instructorHash(input, output, outputSize);
    else
        computeSHA256Hash(input, output, outputSize);
}

//function that extracts row wise data from csv file based on the field ...?
// Function to initialize the repository
void initRepository(const char* filePath) {
    std::cout << "Initializing repository with file: " << filePath << "\n";

    // Example logic: Check if file exists
    if (!filesystem::exists(filePath)) {
        std::cerr << "Error: File not found.\n";
        return;
    }

    // Add your CSV handling and tree-building logic here
    std::cout << "Repository initialized successfully!\n";
}
// Function to handle the 'init' command
char* handleInitCommand(const char* commandInput) {
    const char* prefix = "init<";
    size_t prefixLength = strlen(prefix);

    if (strcompare(commandInput, prefix, prefixLength) != 0) {
        std::cerr << "Error: Invalid 'init' command format. Use init<filename>.\n";
        return nullptr;
    }

    const char* closingBracket = strChar(commandInput + prefixLength, '>');
    if (!closingBracket) {
        std::cerr << "Error: Missing closing '>' in 'init<filename>'.\n";
        return nullptr;
    }

    size_t fileNameLength = closingBracket - (commandInput + prefixLength);
    char* filePath = new char[fileNameLength + 1]; // Allocate memory dynamically

    // Use copyS for safer copying
    copyS(filePath, fileNameLength + 1, commandInput + prefixLength, fileNameLength);

    return filePath;
}
char* handlebranchCommand(const char* commandInput) {

    const char* prefix = "branch<";
    size_t prefixLength = strlen(prefix);

    if (strcompare(commandInput, prefix, prefixLength) != 0) {
        std::cerr << "Error: Invalid 'branch<' command format. Use branch<branchname>.\n";
        return nullptr;
    }

    const char* closingBracket = strChar(commandInput + prefixLength, '>');
    if (!closingBracket) {
        std::cerr << "Error: Missing closing '>' in 'init<filename>'.\n";
        return nullptr;
    }

    size_t fileNameLength = closingBracket - (commandInput + prefixLength);
    char* filePath = new char[fileNameLength + 1]; // Allocate memory dynamically

    // Use copyS for safer copying
    copyS(filePath, fileNameLength + 1, commandInput + prefixLength, fileNameLength);

    return filePath;
}
void create_beauty() {
    cout << "                    _" << endl;
    cout << "                   _(_)_                          wWWWw   _" << endl;
    cout << "       @@@@       (_)@(_)   vVVVv     _     @@@@  (___) _(_)_" << endl;
    cout << "      @@()@@ wWWWw  (_)\\    (___)   _(_)_  @@()@@   Y  (_)@(_)" << endl;
    cout << "       @@@@  (___)      |/    Y    (_)@(_)  @@@@   \\|/   (_)\\" << endl;
    cout << "        /      Y       \\|    \\|/    /(_)    \\|      |/      |" << endl;
    cout << "     \\ |     \\ |/       | / \\ | /  \\|/       |/    \\|      \\|/" << endl;
    cout << "     \\\\|//   \\\\|///  \\\\\\|//\\\\\\|/// \\|///  \\\\\\|//  \\\\|//  \\\\\\|// " << endl;
    cout << "  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;

}
int executeCommand(const char* input) {
    int command_no = 0;

    if (strcompare(input, "init<", 5) == 0) {
        command_no = 1;
    }
    else if (strcompare(input, "branch<", 7) == 0) {
        command_no = 2;
    }
    else if (strcompare(input, "commit<", 7) == 0) {
        command_no = 3;
    }
    else if (strcompare(input, "checkout<", 9) == 0) {
        command_no = 4;
    }
    else if (strcompare(input, "delete-branch<", 14) == 0) {
        command_no = 5;
    }
    else if (strcompare(input, "current-branch", 14) == 0) {
        command_no = 6;
    }
    else if (strcompare(input, "all-branches", 12) == 0){
        command_no = 7;
    }
    else if (strcompare(input, "deletefile<", 11) == 0) {
        command_no = 8;
    }
    else {
        std::cerr << "Error: Unknown command '" << input << "'.\n";
    }

    return command_no;
}
/*  TREES FUNCTION */
// Function to create a new folder with the name extracted from handlebranchCommand
void createNewFolder(const char* branchName, const filesystem::path& currentDir) {
    filesystem::path folderPath = currentDir / branchName;

    // Check if the folder already exists
    if (filesystem::exists(folderPath)) {
        std::cerr << "Error: Folder '" << folderPath.string() << "' already exists.\n";
        return;
    }

    // Create the folder
    try {
        if (filesystem::create_directory(folderPath)) {
            std::cout << "Folder '" << folderPath.string() << "' created successfully.\n";
        }
        else {
            std::cerr << "Error: Failed to create folder '" << folderPath.string() << "'.\n";
            return;
        }

        // Copy all files from the current directory to the new folder
        for (const auto& entry : filesystem::directory_iterator(currentDir)) {
            if (entry.is_regular_file()) {
                filesystem::path destination = folderPath / entry.path().filename();
                filesystem::copy(entry.path(), destination, filesystem::copy_options::overwrite_existing);
                std::cout << "Copied: " << entry.path().filename() << " to " << destination << "\n";
            }
        }
    }
    catch (const filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
// Function to handle the 'commit' command and extract the commit message
char* handleCommitCommand(const char* commandInput) {
    const char* prefix = "commit<";
    const char* ptr = commandInput;

    // Check if the command starts with "commit<"
    while (*ptr && *prefix && *ptr == *prefix) {
        ++ptr;
        ++prefix;
    }

    if (*prefix != '\0') {
        std::cerr << "Error: Invalid 'commit' command format. Use commit<message>.\n";
        return nullptr;
    }

    // Find the closing '>'
    const char* closingBracket = ptr;
    while (*closingBracket && *closingBracket != '>') {
        ++closingBracket;
    }

    if (*closingBracket != '>') {
        std::cerr << "Error: Missing closing '>' in 'commit<message>'.\n";
        return nullptr;
    }

    // Calculate the length of the commit message
    size_t messageLength = closingBracket - ptr;

    // Allocate memory for the commit message and copy it
    char* commitMessage = new char[messageLength + 1];
    for (size_t i = 0; i < messageLength; ++i) {
        commitMessage[i] = ptr[i];
    }
    commitMessage[messageLength] = '\0'; // Null-terminate the string

    return commitMessage;
}

// Function to log the commit action
void logCommitAction(const char* commitMessage, const filesystem::path& nodePath)
{
    ofstream commitFile(nodePath, ios::app);
    if (commitFile.is_open()) {
        commitFile << "Commit Message: " << commitMessage << ", Node: " << nodePath
            << ", Timestamp: " << time(nullptr) << endl;
        commitFile.close();
    }
    else {
        std::cerr << "Error: Could not open commit log file.\n";
    }
}

void handleCheckoutCommand(const char*input1) {

    const char* prefix = "checkout<";
    size_t prefixLength = strlen(prefix);

    // Check if the input starts with 'checkout<'
    if (strcompare(input1, prefix, prefixLength) != 0) {
        std::cerr << "Error: Invalid 'checkout' command format. Use checkout<branchname>.\n";
        return;
    }

    const char* closingBracket = strChar(input1 + prefixLength, '>');
    if (!closingBracket) {
        std::cerr << "Error: Missing closing '>' in 'checkout<branchname>'.\n";
        return;
    }

    size_t branchNameLength = closingBracket - (input1 + prefixLength);
    char* branchName = new char[branchNameLength + 1]; // Allocate memory dynamically

    // Use copyS for safer copying
    copyS(branchName, branchNameLength + 1, input1 + prefixLength, branchNameLength);
    branchName[branchNameLength] = '\0'; // Null-terminate the string

    // Get the current directory and path for the branch folder
    filesystem::path currentDir = filesystem::current_path();
    filesystem::path branchPath = currentDir / branchName;

    // Check if the branch exists
    if (filesystem::exists(branchPath) && filesystem::is_directory(branchPath)) {
        try {
            filesystem::current_path(branchPath); // Switch to the branch directory
            std::cout << "Switched to branch '" << branchName << "'.\n";
        }
        catch (const filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    else {
        std::cerr << "Error: Branch '" << branchName << "' does not exist.\n";
    }
}
void handleDeleteBranchCommand(const char* commandInput) {
    const char* prefix = "delete-branch<";
    size_t prefixLength = strlen(prefix);

    // Check if the input starts with 'delete-branch<'
    if (strcompare(commandInput, prefix, prefixLength) != 0) {
        std::cerr << "Error: Invalid 'delete-branch' command format. Use delete-branch<branchname>.\n";
        return;
    }

    const char* closingBracket = strChar(commandInput + prefixLength, '>');
    if (!closingBracket) {
        std::cerr << "Error: Missing closing '>' in 'delete-branch<branchname>'.\n";
        return;
    }

    size_t branchNameLength = closingBracket - (commandInput + prefixLength);
    char* branchName = new char[branchNameLength + 1]; // Allocate memory dynamically

    // Use copyS for safer copying
    copyS(branchName, branchNameLength + 1, commandInput + prefixLength, branchNameLength);
    branchName[branchNameLength] = '\0'; // Null-terminate the string

    // Get the current directory and path for the branch folder
    fs::path currentDir = fs::current_path();
    fs::path branchPath = currentDir / branchName;

    // Check if the branch exists and delete it
    if (fs::exists(branchPath) && fs::is_directory(branchPath)) {
        try {
            fs::remove_all(branchPath); // Remove the directory and its contents
            std::cout << "Branch '" << branchName << "' deleted successfully.\n";
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    else {
        std::cerr << "Error: Branch '" << branchName << "' does not exist.\n";
    }

    // Clean up
    delete[] branchName;
}
void displayCurrentBranch() {

    // Get the current directory and path for the branch folder
    filesystem::path currentDir = filesystem::current_path();
    filesystem::path branchPath = currentDir;

    std::cout << "You are on branch: '" << branchPath << "'." << std::endl;
}
void handleBranchesCommand() {
    // Get the current directory
    fs::path currentDir = fs::current_path();
    std::cout << "Listing all branches in directory: " << currentDir << "\n";

    try {
        for (const auto& entry : fs::directory_iterator(currentDir)) {
            if (fs::is_directory(entry.status())) {
                // Print only directories, assuming each represents a branch
                std::cout << "- " << entry.path().filename().string() << "\n";
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
char* handleDeleteCommand(const char* commandInput)
{
    const char* prefix = "deletefile<";
    const char* ptr = commandInput;

    // Check if the command starts with "DELETE<"
    while (*ptr && *prefix && *ptr == *prefix)
    {
        ++ptr;
        ++prefix;
    }

    if (*prefix != '\0')
    {
        std::cerr << "Error: Invalid 'DELETE' command format. Use DELETE<data>.\n";
        return nullptr;
    }

    // Find the closing '>'
    const char* closingBracket = ptr;
    while (*closingBracket && *closingBracket != '>')
    {
        ++closingBracket;
    }

    if (*closingBracket != '>')
    {
        std::cerr << "Error: Missing closing '>' in 'DELETE<data>'.\n";
        return nullptr;
    }

    // Calculate the length of the data
    size_t dataLength = closingBracket - ptr;

    // Allocate memory for the data and copy it
    char* data = new char[dataLength + 1];
    for (size_t i = 0; i < dataLength; i++)
    {
        data[i] = ptr[i];
    }
    data[dataLength] = '\0'; // Null-terminate the string

    return data;
}