This project has been made as a submission for Software Development Fundamentals Lab - 2 Project Based Learning.

# Multinomy
A multiplayer economy based CLI-game.

# Installation and running(for Windows)
1. Install [MySQL Server](https://dev.mysql.com/downloads/mysql/).
2. Install MingW.
3. Clone this repository.
   ```bash
   git clone https://github.com/Kavoyaa/sdf-2-pbl/
   ```
5. Make sure to link the libraries with the project properly.
6. Compile & run.
   ```bash
   cd sdf-2-pbl
   ```
   ```bash
    g++ -std=c++17 main.cpp commands.cpp utility.cpp -o main.exe -I "C:/Program Files/MySQL/MySQL Server 9.7/include" -L "C:/Program Files/MySQL/MySQL Server 9.7/lib" -l libmysql
   ```
   ```bash
   ./main.exe
   ```
