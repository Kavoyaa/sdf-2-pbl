#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mysql.h>
#include "utility.h"
#include "commands.h"

std::vector<std::string> parseInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) tokens.push_back(token);
    return tokens;
}

Player authMenu(MYSQL* conn) {
    while (true) {
        std::cout << CYAN << "\n================================\n";
        std::cout << "        MULTINOMY GAME\n";
        std::cout << "================================\n" << RESET;
        std::cout << "  [1] Login\n";
        std::cout << "  [2] Register\n";
        std::cout << "  [3] Exit\n";
        std::cout << "\nChoice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1) {
            std::string uname, password;
            std::cout << "Username : "; std::getline(std::cin, uname);
            std::cout << "Password : "; std::getline(std::cin, password);

            if (Player::login(conn, uname, password)) {
                Player p(conn, uname);
                std::cout << GREEN << "\nWelcome back, " << p.dname << "!\n" << RESET;
                return p;
            } else {
                std::cout << RED << "Incorrect username or password.\n" << RESET;
            }

        } else if (choice == 2) {
            std::string uname, dname, password;

            std::cout << "Username     (max 15 chars, no spaces) : ";
            std::getline(std::cin, uname);
            if (uname.empty() || uname.length() > 15) { std::cout << RED << "Invalid username.\n" << RESET; continue; }
            if (uname.find(' ') != std::string::npos)  { std::cout << RED << "No spaces in username.\n" << RESET; continue; }
            if (Player::exists(conn, uname))            { std::cout << RED << "Username already taken.\n" << RESET; continue; }

            std::cout << "Display Name (max 15 chars)            : ";
            std::getline(std::cin, dname);
            if (dname.empty() || dname.length() > 15) { std::cout << RED << "Invalid display name.\n" << RESET; continue; }

            std::cout << "Password     (max 20 chars)            : ";
            std::getline(std::cin, password);
            if (password.empty() || password.length() > 20) { std::cout << RED << "Invalid password.\n" << RESET; continue; }

            if (Player::registerPlayer(conn, uname, dname, password)) {
                std::cout << GREEN << "\nAccount created! You start with $500.\nWelcome, " << dname << "!\n" << RESET;
                return Player(conn, uname);
            } else {
                std::cout << RED << "Registration failed.\n" << RESET;
            }

        } else if (choice == 3) {
            std::cout << YELLOW << "Goodbye!\n" << RESET;
            mysql_close(conn);
            exit(0);
        } else {
            std::cout << RED << "Invalid choice.\n" << RESET;
        }
    }
}

int main() {
    srand((unsigned int)time(nullptr));

    MYSQL* conn = connectDB();
    if (!conn) return 1;

    Player currentPlayer = authMenu(conn);
    std::cout << "Type " << BOLD << "help" << RESET << " to see all commands.\n";

    while (true) {
        std::cout << CYAN << "\n> " << RESET;
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) continue;

        auto tokens = parseInput(input);
        if (tokens.empty()) continue;

        std::string cmd = tokens[0];

        if      (cmd == "help")     { cmdHelp(); }
        else if (cmd == "stats")    { cmdStats   (conn, currentPlayer, tokens.size() > 1 ? tokens[1] : ""); }
        else if (cmd == "beg")      { cmdBeg     (conn, currentPlayer); }
        else if (cmd == "fish")     { cmdFish    (conn, currentPlayer); }
        else if (cmd == "rob")      { cmdRob     (conn, currentPlayer, tokens.size() > 1 ? tokens[1] : ""); }
        else if (cmd == "deposit")  { cmdDeposit (conn, currentPlayer, tokens.size() > 1 ? tokens[1] : ""); }
        else if (cmd == "withdraw") { cmdWithdraw(conn, currentPlayer, tokens.size() > 1 ? tokens[1] : ""); }
        else if (cmd == "logout") {
            std::cout << YELLOW << "Logged out.\n" << RESET;
            currentPlayer = authMenu(conn);
            std::cout << "Type " << BOLD << "help" << RESET << " to see all commands.\n";
        }
        else if (cmd == "exit" || cmd == "quit") {
            std::cout << YELLOW << "Goodbye!\n" << RESET;
            break;
        }
        else {
            std::cout << RED << "Unknown command '" << cmd << "'. Type 'help'.\n" << RESET;
        }

        checkNotifications(conn, currentPlayer.uname);
    }

    mysql_close(conn);
    return 0;
}