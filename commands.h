#pragma once
#include <mysql.h>
#include "utility.h"

void cmdHelp();
void cmdStats(MYSQL* conn, Player& currentPlayer, const std::string& targetUser);
void cmdBeg(MYSQL* conn, Player& currentPlayer);
void cmdFish(MYSQL* conn, Player& currentPlayer);
void cmdRob(MYSQL* conn, Player& currentPlayer, const std::string& targetUser);
void cmdDeposit(MYSQL* conn, Player& currentPlayer, const std::string& amountStr);
void cmdWithdraw(MYSQL* conn, Player& currentPlayer, const std::string& amountStr);