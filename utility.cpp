#include "utility.h"
#include <fstream>

// Helpers
static int safeStoi(const char* str, int def = 0) {
    if (!str) return def;
    try { return std::stoi(str); } catch (...) { return def; }
}
static float safeStof(const char* str, float def = 0.0f) {
    if (!str) return def;
    try { return std::stof(str); } catch (...) { return def; }
}

// Connect
MYSQL* connectDB() {
    // Read database.txt
    std::ifstream file("database.txt");
    if (!file.is_open()) {
        std::cerr << RED << "Could not open database.txt\n" << RESET;
        return nullptr;
    }

    std::string host, portStr, user, password, dbname;
    std::getline(file, host);
    std::getline(file, portStr);
    std::getline(file, user);
    std::getline(file, password);
    std::getline(file, dbname);
    file.close();

    if (host.empty() || portStr.empty() || user.empty() || password.empty() || dbname.empty()) {
        std::cerr << RED << "database.txt is missing one or more fields.\n" << RESET;
        return nullptr;
    }

    int port;
    try { port = std::stoi(portStr); }
    catch (...) {
        std::cerr << RED << "Invalid port in database.txt\n" << RESET;
        return nullptr;
    }

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) { std::cerr << "mysql_init failed\n"; return nullptr; }

    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                            dbname.c_str(), port, nullptr, 0)) {
        std::cerr << RED << "Connection failed: " << mysql_error(conn) << RESET << "\n";
        mysql_close(conn);
        return nullptr;
    }

    mysql_autocommit(conn, 1);
    return conn;
}

// Notifications 
void sendNotification(MYSQL* conn, const std::string& toUser, const std::string& type,
                      const std::string& message, const std::string& sender) {
    std::string q = "INSERT INTO NotifsData (UName, Type, Message, Sender) VALUES ('"
        + toUser + "', '" + type + "', '" + message + "', '" + sender + "')";
    mysql_query(conn, q.c_str());
}

void checkNotifications(MYSQL* conn, const std::string& uname) {
    std::string q = "SELECT Type, Message FROM NotifsData WHERE UName = '" + uname + "'";
    if (mysql_query(conn, q.c_str())) return;

    MYSQL_RES* res = mysql_store_result(conn);
    if (mysql_num_rows(res) == 0) { mysql_free_result(res); return; }

    std::cout << YELLOW << "\n[!] You have unread notifications:\n" << RESET;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
        std::cout << YELLOW << "  [" << (row[0] ? row[0] : "?") << "] "
                  << (row[1] ? row[1] : "") << "\n" << RESET;
    std::cout << "\n";
    mysql_free_result(res);

    mysql_query(conn, ("DELETE FROM NotifsData WHERE UName = '" + uname + "'").c_str());
}

// Player Constructors
Player::Player()
    : wallet(0), securityLvl(1), luckLvl(1),
      bankBal(0), bankCap(1000), intelligence(1.0f), valid(false) {}

Player::Player(MYSQL* conn, const std::string& uname) : Player() {
    std::string q =
        "SELECT UName, DName, Wallet, SecurityLvl, LuckLvl, BankBal, BankCap, Job, Intelligence"
        " FROM PlayerData WHERE UName = '" + uname + "'";

    if (mysql_query(conn, q.c_str())) return;

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW  row = mysql_fetch_row(res);

    if (row) {
        this->uname  = row[0] ? row[0] : "";
        this->dname  = row[1] ? row[1] : "";
        wallet       = safeStoi(row[2]);
        securityLvl  = safeStoi(row[3], 1);
        luckLvl      = safeStoi(row[4], 1);
        bankBal      = safeStoi(row[5]);
        bankCap      = safeStoi(row[6], 1000);
        job          = row[7] ? row[7] : "None";
        intelligence = safeStof(row[8], 1.0f);
        valid        = true;
    }
    mysql_free_result(res);
}

// Player Money Methods
bool Player::addMoney(MYSQL* conn, int amount) {
    std::string q = "UPDATE PlayerData SET Wallet = Wallet + " + std::to_string(amount)
        + " WHERE UName = '" + uname + "'";
    if (mysql_query(conn, q.c_str()) != 0) {
        std::cerr << RED << "addMoney failed: " << mysql_error(conn) << RESET << "\n";
        return false;
    }
    wallet += amount;
    return true;
}

bool Player::removeMoney(MYSQL* conn, int amount) {
    std::string q = "UPDATE PlayerData SET Wallet = Wallet - " + std::to_string(amount)
        + " WHERE UName = '" + uname + "'";
    if (mysql_query(conn, q.c_str()) != 0) {
        std::cerr << RED << "removeMoney failed: " << mysql_error(conn) << RESET << "\n";
        return false;
    }
    wallet -= amount;
    return true;
}

bool Player::deposit(MYSQL* conn, int amount) {
    if (amount > wallet) {
        std::cout << RED << "You don't have $" << amount << " in your wallet.\n" << RESET;
        return false;
    }
    int space = bankCap - bankBal;
    if (space <= 0) {
        std::cout << RED << "Your bank is full! (Capacity: $" << bankCap << ")\n" << RESET;
        return false;
    }
    if (amount > space) {
        std::cout << YELLOW << "Only $" << space << " space left. Depositing $" << space << " instead.\n" << RESET;
        amount = space;
    }

    std::string q = "UPDATE PlayerData SET Wallet = Wallet - " + std::to_string(amount)
        + ", BankBal = BankBal + " + std::to_string(amount)
        + " WHERE UName = '" + uname + "'";

    if (mysql_query(conn, q.c_str()) != 0) {
        std::cerr << RED << "Deposit failed: " << mysql_error(conn) << RESET << "\n";
        return false;
    }
    wallet  -= amount;
    bankBal += amount;
    return true;
}

bool Player::withdraw(MYSQL* conn, int amount) {
    if (amount > bankBal) {
        std::cout << RED << "You only have $" << bankBal << " in your bank.\n" << RESET;
        return false;
    }

    std::string q = "UPDATE PlayerData SET BankBal = BankBal - " + std::to_string(amount)
        + ", Wallet = Wallet + " + std::to_string(amount)
        + " WHERE UName = '" + uname + "'";

    if (mysql_query(conn, q.c_str()) != 0) {
        std::cerr << RED << "Withdraw failed: " << mysql_error(conn) << RESET << "\n";
        return false;
    }
    bankBal -= amount;
    wallet  += amount;
    return true;
}

// Player Display
void Player::display(bool isSelf) const {
    std::cout << CYAN << "\n=== " << dname << "'s Stats ===\n" << RESET;
    std::cout << "  Username     : " << uname << "\n";
    std::cout << "  Display Name : " << dname << "\n";
    std::cout << "  Job          : " << job   << "\n";
    std::cout << GREEN << "  Wallet       : $" << wallet << "\n" << RESET;
    if (isSelf)
        std::cout << GREEN << "  Bank         : $" << bankBal << " / $" << bankCap << "\n" << RESET;
    std::cout << "  Security Lvl : " << securityLvl << "\n";
    std::cout << "  Luck Lvl     : " << luckLvl << "\n";
    std::cout << "  Intelligence : " << std::fixed << std::setprecision(1) << intelligence << "\n\n";
}

// Player Static Auth
bool Player::exists(MYSQL* conn, const std::string& uname) {
    std::string q = "SELECT UName FROM UserData WHERE UName = '" + uname + "'";
    if (mysql_query(conn, q.c_str())) return false;
    MYSQL_RES* res = mysql_store_result(conn);
    bool found = mysql_num_rows(res) > 0;
    mysql_free_result(res);
    return found;
}

bool Player::login(MYSQL* conn, const std::string& uname, const std::string& password) {
    std::string q = "SELECT UName FROM UserData WHERE UName = '" + uname
        + "' AND Password = '" + password + "'";
    if (mysql_query(conn, q.c_str())) return false;
    MYSQL_RES* res = mysql_store_result(conn);
    bool ok = mysql_num_rows(res) > 0;
    mysql_free_result(res);
    return ok;
}

bool Player::registerPlayer(MYSQL* conn, const std::string& uname,
                             const std::string& dname, const std::string& password) {
    std::string q1 = "INSERT INTO UserData (UName, DName, Password) VALUES ('"
        + uname + "', '" + dname + "', '" + password + "')";
    if (mysql_query(conn, q1.c_str())) {
        std::cerr << RED << "Register failed: " << mysql_error(conn) << RESET << "\n";
        return false;
    }

    std::string q2 =
        "INSERT INTO PlayerData (UName, DName, Wallet, SecurityLvl, LuckLvl, BankBal, BankCap, Job, Intelligence)"
        " VALUES ('" + uname + "', '" + dname + "', 500, '1', 1, 0, 1000, 'None', 1.0)";
    if (mysql_query(conn, q2.c_str())) {
        std::cerr << RED << "PlayerData init failed: " << mysql_error(conn) << RESET << "\n";
        return false;
    }
    return true;
}
