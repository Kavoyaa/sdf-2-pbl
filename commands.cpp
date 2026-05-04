#include "commands.h"
#include <iostream>
#include <cstdlib>

void cmdHelp() {
    std::cout << CYAN << "\n=== ECONOMY GAME - COMMANDS ===\n" << RESET;
    std::cout << "  help              - Show this help menu\n";
    std::cout << "  stats             - View your own stats\n";
    std::cout << "  stats <user>      - View another player's stats\n";
    std::cout << "  beg               - Beg for a small amount of money\n";
    std::cout << "  fish              - Go fishing to earn money\n";
    std::cout << "  rob <user>        - Attempt to rob another player\n";
    std::cout << "  deposit <amount>  - Deposit money into your bank\n";
    std::cout << "  withdraw <amount> - Withdraw money from your bank\n";
    std::cout << "  logout            - Log out\n";
    std::cout << "  exit              - Quit the game\n\n";
}

void cmdStats(MYSQL* conn, Player& currentPlayer, const std::string& targetUser) {
    bool isSelf = targetUser.empty() || targetUser == currentPlayer.uname;

    if (isSelf) { currentPlayer.display(true); return; }

    if (!Player::exists(conn, targetUser)) {
        std::cout << RED << "Player '" << targetUser << "' not found.\n" << RESET;
        return;
    }

    Player target(conn, targetUser);
    if (!target.valid) { std::cout << RED << "Could not retrieve stats.\n" << RESET; return; }
    target.display(false);
}

void cmdBeg(MYSQL* conn, Player& currentPlayer) {
    int amount = (rand() % 91) + 10;
    const char* lines[] = {
        "A kind stranger tossed you",
        "You sat on the sidewalk and earned",
        "Someone felt sorry for you and gave you",
        "A tourist dropped",
        "You held up a sign and collected"
    };
    currentPlayer.addMoney(conn, amount);
    std::cout << GREEN << lines[rand() % 5] << " $" << amount << "!\n" << RESET;
}

void cmdFish(MYSQL* conn, Player& currentPlayer) {
    std::cout << "You cast your line into the water...\n";
    if ((rand() % 100) < 40) {
        const char* fails[] = {
            "You caught an old boot. Not worth much.",
            "The fish outsmarted you. Nothing caught.",
            "Your line snapped. Better luck next time!",
            "You caught a plastic bag. Disappointing."
        };
        std::cout << YELLOW << fails[rand() % 4] << "\n" << RESET;
        return;
    }
    int amount = (rand() % 251) + 50;
    const char* catches[] = {
        "a fat salmon", "a massive tuna", "a rare golden carp",
        "a bunch of small trout", "a surprisingly meaty bass"
    };
    currentPlayer.addMoney(conn, amount);
    std::cout << GREEN << "You caught " << catches[rand() % 5]
              << " and sold it for $" << amount << "!\n" << RESET;
}

void cmdRob(MYSQL* conn, Player& currentPlayer, const std::string& targetUser) {
    if (targetUser.empty()) { std::cout << RED << "Usage: rob <username>\n" << RESET; return; }
    if (targetUser == currentPlayer.uname) { std::cout << RED << "You can't rob yourself!\n" << RESET; return; }
    if (!Player::exists(conn, targetUser)) {
        std::cout << RED << "Player '" << targetUser << "' not found.\n" << RESET; return;
    }

    Player target(conn, targetUser);
    if (!target.valid) { std::cout << RED << "Something went wrong.\n" << RESET; return; }

    if (target.wallet <= 0) {
        std::cout << YELLOW << target.dname << "'s wallet is empty. Nothing to steal!\n" << RESET;
        return;
    }

    int successChance = 65 - (target.securityLvl * 10);
    if (successChance < 10) successChance = 10;

    std::cout << "You sneak towards " << target.dname << "'s wallet...\n";

    if ((rand() % 100) < successChance) {
        int stolen = (target.wallet * ((rand() % 31) + 20)) / 100;
        if (stolen < 1) stolen = 1;

        target.removeMoney(conn, stolen);
        currentPlayer.addMoney(conn, stolen);

        std::cout << GREEN << "Success! You robbed " << target.dname
                  << " and got away with $" << stolen << "!\n" << RESET;

        sendNotification(conn, targetUser, "ROB",
            currentPlayer.dname + " robbed you and stole $" + std::to_string(stolen) + " from your wallet!",
            currentPlayer.uname);
    } else {
        int penalty = (rand() % 76) + 25;
        if (penalty > currentPlayer.wallet) penalty = currentPlayer.wallet;

        std::cout << RED << "You got caught! " << target.dname << "'s security spotted you!\n" << RESET;
        if (penalty > 0) {
            currentPlayer.removeMoney(conn, penalty);
            std::cout << RED << "You were fined $" << penalty << " as punishment.\n" << RESET;
        } else {
            std::cout << RED << "Luckily you had no money to be fined.\n" << RESET;
        }

        sendNotification(conn, targetUser, "ROB",
            currentPlayer.dname + " tried to rob you but got caught! Your security held up.",
            currentPlayer.uname);
    }
}

void cmdDeposit(MYSQL* conn, Player& currentPlayer, const std::string& amountStr) {
    if (amountStr.empty()) { std::cout << RED << "Usage: deposit <amount>\n" << RESET; return; }
    int amount;
    try { amount = std::stoi(amountStr); } catch (...) { std::cout << RED << "Invalid amount.\n" << RESET; return; }
    if (amount <= 0) { std::cout << RED << "Amount must be greater than 0.\n" << RESET; return; }

    if (currentPlayer.deposit(conn, amount)) {
        std::cout << GREEN << "Deposited into your bank.\n" << RESET;
        std::cout << "  Wallet : $" << currentPlayer.wallet  << "\n";
        std::cout << "  Bank   : $" << currentPlayer.bankBal << " / $" << currentPlayer.bankCap << "\n";
    }
}

void cmdWithdraw(MYSQL* conn, Player& currentPlayer, const std::string& amountStr) {
    if (amountStr.empty()) { std::cout << RED << "Usage: withdraw <amount>\n" << RESET; return; }
    int amount;
    try { amount = std::stoi(amountStr); } catch (...) { std::cout << RED << "Invalid amount.\n" << RESET; return; }
    if (amount <= 0) { std::cout << RED << "Amount must be greater than 0.\n" << RESET; return; }

    if (currentPlayer.withdraw(conn, amount)) {
        std::cout << GREEN << "Withdrew from your bank.\n" << RESET;
        std::cout << "  Wallet : $" << currentPlayer.wallet  << "\n";
        std::cout << "  Bank   : $" << currentPlayer.bankBal << " / $" << currentPlayer.bankCap << "\n";
    }
}
