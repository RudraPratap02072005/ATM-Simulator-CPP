#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

// User class to encapsulate account data
class User {
private:
    string accountNumber;
    string pin;
    double balance;
    
public:
    User(string acc = "", string p = "", double bal = 0.0) 
        : accountNumber(acc), pin(p), balance(bal) {}
    
    // Getters
    string getAccountNumber() const { return accountNumber; }
    string getPin() const { return pin; }
    double getBalance() const { return balance; }
    
    // Setters
    void setBalance(double newBalance) { balance = newBalance; }
    
    // Check if account exists and PIN matches
    bool authenticate(const string& inputPin) const {
        return pin == inputPin;
    }
    
    // Convert user data to string for file storage
    string toString() const {
        return accountNumber + "|" + pin + "|" + to_string(balance);
    }
};

// Transaction class
class Transaction {
public:
    string accountNumber;
    string type;
    double amount;
    string timestamp;
    
    Transaction(string acc, string t, double amt, string time) 
        : accountNumber(acc), type(t), amount(amt), timestamp(time) {}
    
    string toString() const {
        return accountNumber + "|" + type + "|" + to_string(amount) + "|" + timestamp;
    }
};

class ATM {
private:
    User currentUser;
    vector<Transaction> recentTransactions;
    const int MAX_LOGIN_ATTEMPTS = 3;
    const double MIN_WITHDRAWAL = 100.0;
    const double MAX_WITHDRAWAL = 20000.0;
    
    // File paths
    const string USERS_FILE = "users.txt";
    const string TRANSACTIONS_FILE = "transactions.txt";
    
    // Hide PIN input
    string getHiddenPin() {
        string pin;
        char ch;
        cout << "Enter PIN: ";
        while ((ch = getchar()) != '\n') {
            if (ch == '\b') {
                if (!pin.empty()) {
                    pin.pop_back();
                    cout << "\b \b";
                }
            } else {
                pin += ch;
                cout << "*";
            }
        }
        cout << endl;
        return pin;
    }
    
    // Load user from file
    User loadUser(const string& accountNumber) {
        ifstream file(USERS_FILE);
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string acc, pinStr;
            double balance;
            getline(ss, acc, '|');
            getline(ss, pinStr, '|');
            ss >> balance;
            
            if (acc == accountNumber) {
                file.close();
                return User(acc, pinStr, balance);
            }
        }
        file.close();
        return User(); // Empty user if not found
    }
    
    // Save user to file
    void saveUser(const User& user) {
        vector<string> users;
        ifstream file(USERS_FILE);
        string line;
        
        // Read existing users
        while (getline(file, line)) {
            stringstream ss(line);
            string acc;
            getline(ss, acc, '|');
            if (acc != user.getAccountNumber()) {
                users.push_back(line);
            }
        }
        file.close();
        
        // Add/update current user
        users.push_back(user.toString());
        
        // Write back to file
        ofstream outFile(USERS_FILE);
        for (const string& userData : users) {
            outFile << userData << endl;
        }
        outFile.close();
    }
    
    // Load recent transactions
    void loadTransactions() {
        recentTransactions.clear();
        ifstream file(TRANSACTIONS_FILE);
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string acc, type, time;
            double amount;
            getline(ss, acc, '|');
            getline(ss, type, '|');
            ss >> amount;
            getline(ss, time, '|');
            recentTransactions.emplace_back(acc, type, amount, time);
        }
        file.close();
    }
    
    // Save transaction
    void saveTransaction(const Transaction& trans) {
        ofstream file(TRANSACTIONS_FILE, ios::app);
        file << trans.toString() << endl;
        file.close();
        loadTransactions(); // Reload to keep recent transactions updated
    }
    
    // Get current timestamp
    string getTimestamp() {
        time_t now = time(0);
        char* dt = ctime(&now);
        string timestamp(dt);
        timestamp.pop_back(); // Remove newline
        return timestamp;
    }

public:
    // Create default user if file doesn't exist
    void initializeUsers() {
        ifstream file(USERS_FILE);
        if (!file.good()) {
            ofstream outFile(USERS_FILE);
            // Sample user: Account 12345678, PIN 4321, Balance 50000
            outFile << "12345678|4321|50000.00" << endl;
            outFile.close();
            cout << "Default user created. Account: 12345678, PIN: 4321" << endl;
        }
        file.close();
    }
    
    // Authentication
    bool authenticateUser() {
        string accountNumber;
        cout << "Enter Account Number: ";
        cin >> accountNumber;
        cin.ignore();
        
        currentUser = loadUser(accountNumber);
        if (currentUser.getAccountNumber().empty()) {
            cout << "Account not found!" << endl;
            return false;
        }
        
        for (int attempts = 0; attempts < MAX_LOGIN_ATTEMPTS; attempts++) {
            string inputPin = getHiddenPin();
            if (currentUser.authenticate(inputPin)) {
                cout << "\nLogin Successful!\n" << endl;
                loadTransactions();
                return true;
            }
            cout << "Invalid PIN! Attempts remaining: " << (MAX_LOGIN_ATTEMPTS - attempts - 1) << endl;
        }
        cout << "Account locked due to too many failed attempts!" << endl;
        return false;
    }
    
    // Display main menu
    void showMainMenu() {
        cout << "\n=== ATM MENU ===" << endl;
        cout << "1. Balance Inquiry" << endl;
        cout << "2. Cash Withdrawal" << endl;
        cout << "3. Cash Deposit" << endl;
        cout << "4. Mini Statement" << endl;
        cout << "5. Exit" << endl;
        cout << "Choose option: ";
    }
    
    // Balance inquiry
    void balanceInquiry() {
        cout << "\nCurrent Balance: Rs " << fixed << setprecision(2) 
             << currentUser.getBalance() << endl;
        saveTransaction(Transaction(currentUser.getAccountNumber(), "BALANCE_INQUIRY", 
                                  currentUser.getBalance(), getTimestamp()));
    }
    
    // Cash withdrawal
    void cashWithdrawal() {
        double amount;
        cout << "Enter withdrawal amount (Min: " << MIN_WITHDRAWAL 
             << ", Max: " << MAX_WITHDRAWAL << "): Rs ";
        cin >> amount;
        
        if (amount < MIN_WITHDRAWAL || amount > MAX_WITHDRAWAL) {
            cout << "Invalid amount! Please enter between Rs " << MIN_WITHDRAWAL 
                 << " and Rs " << MAX_WITHDRAWAL << endl;
            return;
        }
        
        if (amount > currentUser.getBalance()) {
            cout << "Insufficient balance!" << endl;
            cout << "Available balance: Rs " << fixed << setprecision(2) 
                 << currentUser.getBalance() << endl;
            return;
        }
        
        currentUser.setBalance(currentUser.getBalance() - amount);
        saveUser(currentUser);
        saveTransaction(Transaction(currentUser.getAccountNumber(), "WITHDRAWAL", 
                                  amount, getTimestamp()));
        
        cout << "Please collect your cash." << endl;
        cout << "New Balance: Rs " << fixed << setprecision(2) 
             << currentUser.getBalance() << endl;
    }
    
    // Cash deposit
    void cashDeposit() {
        double amount;
        cout << "Enter deposit amount: Rs ";
        cin >> amount;
        
        if (amount <= 0) {
            cout << "Invalid amount!" << endl;
            return;
        }
        
        currentUser.setBalance(currentUser.getBalance() + amount);
        saveUser(currentUser);
        saveTransaction(Transaction(currentUser.getAccountNumber(), "DEPOSIT", 
                                  amount, getTimestamp()));
        
        cout << "Deposit successful!" << endl;
        cout << "New Balance: Rs " << fixed << setprecision(2) 
             << currentUser.getBalance() << endl;
    }
    
    // Mini statement (last 5 transactions)
    void miniStatement() {
        cout << "\n=== MINI STATEMENT ===" << endl;
        cout << "Account: " << currentUser.getAccountNumber() << endl;
        cout << "Current Balance: Rs " << fixed << setprecision(2) 
             << currentUser.getBalance() << endl;
        cout << "------------------------" << endl;
        
        int count = 0;
        for (int i = recentTransactions.size() - 1; i >= 0 && count < 5; i--) {
            if (recentTransactions[i].accountNumber == currentUser.getAccountNumber()) {
                cout << left << setw(10) << recentTransactions[i].type 
                     << " Rs " << fixed << setprecision(2) << recentTransactions[i].amount
                     << " [" << recentTransactions[i].timestamp << "]" << endl;
                count++;
            }
        }
        
        if (count == 0) {
            cout << "No transactions found." << endl;
        }
        cout << "------------------------" << endl;
    }
    
    // Main ATM loop
    void run() {
        initializeUsers();
        
        while (true) {
            cout << "\n" << string(40, '=') << endl;
            cout << "     Welcome to ATM Simulator" << endl;
            cout << string(40, '=') << endl;
            
            if (!authenticateUser()) {
                cout << "Returning to main menu..." << endl;
                continue;
            }
            
            while (true) {
                showMainMenu();
                int choice;
                cin >> choice;
                cin.ignore();
                
                switch (choice) {
                    case 1:
                        balanceInquiry();
                        break;
                    case 2:
                        cashWithdrawal();
                        break;
                    case 3:
                        cashDeposit();
                        break;
                    case 4:
                        miniStatement();
                        break;
                    case 5:
                        cout << "\nThank you for using ATM. Have a great day!" << endl;
                        return;
                    default:
                        cout << "Invalid option! Please try again." << endl;
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
            }
        }
    }
};

int main() {
    ATM atm;
    atm.run();
    return 0;
}
