#pragma once
#include <mysql.h>
#include <string>
#include <iostream>
#include <iomanip>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

MYSQL* connectDB();
void sendNotification(MYSQL* conn, const std::string& toUser, const std::string& type, const std::string& message, const std::string& sender);
void checkNotifications(MYSQL* conn, const std::string& uname);

// Player Class
class Player {
public:
    std::string uname;
    std::string dname;
    std::string job;
    int wallet;
    int securityLvl;
    int luckLvl;
    int bankBal;
    int bankCap;
    float intelligence;
    bool valid;

    Player();                                          // empty player
    Player(MYSQL* conn, const std::string& uname);    // load from DB

    bool addMoney(MYSQL* conn, int amount);
    bool removeMoney(MYSQL* conn, int amount);
    bool deposit(MYSQL* conn, int amount);
    bool withdraw(MYSQL* conn, int amount);
    void display(bool isSelf) const;

    // Static auth methods
    static bool exists(MYSQL* conn, const std::string& uname);
    static bool login(MYSQL* conn, const std::string& uname, const std::string& password);
    static bool registerPlayer(MYSQL* conn, const std::string& uname,
                               const std::string& dname, const std::string& password);
};
