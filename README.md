# Description
This is our final project for the course Data Structures. It is a Git-like repository system built for the purpose of handling data in a filesystem. Users can load datasets and create repositories for them based on the fields with the records being stored in tree-like structures. There is no burden placed on the RAM as the tree and its nodes is a filesystem itself.

# Functionality
Upon running the program, the user can begin to enter commands to manage versioning and data synchronization. The commands and their functionalities are as follows:
- **init<csv_filepath>**: Initializes the repository for the .csv file containing data and prompts the user to select the field and data stucture (tree type) to store the records. Records are also hashed to ensure integrity, and the user can choose the hash type.
- **branch<message>**: Creates a branch (new folder).
- **commit<message>**: Commits the changes made so far. Updates the Merkle Tree to ensure data integrity. Saves the commit details (message, time, etc.) to the log file.
- **checkout<branch_name>**: Switches to the specified branch.
- **delete-branch<branch_name>**: Deletes the specified branch.
- **current-branch**: Displays the current branch's path.
- **all-branches**: Lists all branches.
- **deletefile<filename>**: Deletes the specified file.

  The trees also have complete insertion and deletion capabilities.

  # Contributers
  - <a href=https://github.com/AabiaAli>Aabia Ali</a>
  - <a href=https://github.com/insharahn>Insharah Irfan Nazir</a>
  - <a href=https://github.com/ZaraHEREhehe>Zara Noor Qazi</a>
