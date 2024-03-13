#include <iostream>
#include <iomanip>
#include <sqlite3.h>

struct Item {
    std::string barcodeID;
    std::string name;
    float price;
    float cost;
};

// Callback function to handle SQLite3 errors
int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    return 0;
}

// Function to add an item to the SQLite3 database
void add_item() {
    Item newItem;
    std::cout << "Scan barcode: ";
    std::cin >> newItem.barcodeID;
    std::cin.ignore(); // Clear input buffer
    std::cout << "Enter name: ";
    std::getline(std::cin, newItem.name);
    std::cout << "Enter price: ";
    std::cin >> newItem.price;
    std::cout << "Enter cost: ";
    std::cin >> newItem.cost;

    sqlite3* db;
    int rc = sqlite3_open("items.db", &db);
    if(rc){
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    // Create items table if not exists
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS items (" \
                                 "barcode TEXT PRIMARY KEY NOT NULL," \
                                 "name TEXT NOT NULL," \
                                 "price REAL NOT NULL," \
                                 "cost REAL NOT NULL);";
    rc = sqlite3_exec(db, createTableSQL, callback, 0, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // Construct SQL query to insert item
    std::string insertSQL = "INSERT INTO items (barcode, name, price, cost) VALUES ('" +
                            newItem.barcodeID + "', '" + newItem.name + "', " +
                            std::to_string(newItem.price) + ", " +
                            std::to_string(newItem.cost) + ");";

    // Execute SQL query
    rc = sqlite3_exec(db, insertSQL.c_str(), callback, 0, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Item added successfully." << std::endl;
    }

    sqlite3_close(db);
}

// Implement scan_item() function.

// Implement sale function.


int main() {
    int choice;
    std::cout << "Enter choice: ";
    std::cin >> choice;

    if (choice == 1) {
        add_item();
    }

    return 0;
}
