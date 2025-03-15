# B-Tree Implementation with GTK

## Introduction
This project implements a **B-Tree** data structure with a **Graphical User Interface (GUI)** using **GTK** in C. A B-Tree is a self-balancing tree data structure that maintains sorted data and allows searches, sequential access, insertions, and deletions in logarithmic time.

This implementation provides the following functionalities:
- Insert keys into the B-Tree
- Delete keys from the B-Tree
- Visualize the tree structure in a GTK-based UI
- Dynamic node splitting and merging

## Features
- **Efficient Insertions & Deletions:** Ensures logarithmic time complexity.
- **Interactive GUI:** Uses GTK to display the B-Tree structure.
- **Automatic Node Splitting & Merging:** Handles overflow and underflow conditions efficiently.
- **Search Functionality:** Highlights matching prefixes dynamically.

## Prerequisites
Ensure you have the following installed on your system:
- **GTK3** for GUI support (`libgtk-3-dev` for Linux)
- **GCC** or another C compiler
- **Make** (optional for build automation)

## Installation
Clone the repository:
```bash
 git clone https://github.com/zeofr/String-B-Tree.git
 cd String-B-Tree
```

## Compilation
Compile the C program using GCC:
```bash
 gcc -o btree_gui main.c `pkg-config --cflags --libs gtk+-3.0`
```

## Usage
Run the program:
```bash
 ./btree_gui
```

## File Structure
```
.
├── main.c          # Main source code implementing B-Tree with GTK
├── README.md       # Project documentation
└── assets/         # (If any) Images or icons for UI
```

## Future Improvements
- Add keyboard input for quick insertions and deletions.
- Improve UI styling for better visualization.
- Implement file I/O support to save and load trees.

## License
This project is licensed under the MIT License. Feel free to modify and distribute it.

## Contributors
- **zeofr** - [GitHub](https://github.com/zeofr)

Feel free to contribute to this project by submitting pull requests!


