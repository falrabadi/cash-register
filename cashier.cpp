#include <iostream>
#include <iomanip>
#include <sqlite3.h>
#include <string>
#include <optional>

struct Item {
    std::string barcodeID;
    std::string name;
    float price;
    float cost;
};

// Function to check if barcode exists in the database and return the corresponding item
std::optional<Item> getItemByBarcode(const std::string& barcode) {
    sqlite3* db;
    int rc = sqlite3_open("items.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    std::string sql = "SELECT name, price, cost FROM items WHERE barcode = '" + barcode + "';";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return std::nullopt;
    }

    // Step through the result set
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Item item;
        item.barcodeID = barcode;
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item.price = sqlite3_column_double(stmt, 1);
        item.cost = sqlite3_column_double(stmt, 2);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return item;
    }

    // Barcode not found
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return std::nullopt;
}



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

    // Check if barcode already exists
    if (getItemByBarcode(newItem.barcodeID).has_value()) {
        std::cerr << "Error: Barcode already exists in the database." << std::endl;
        return;
    }

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

// Implement remove_item() function.

// Implement edit_item() function.


int main() {
    int choice;
    std::cout << "Enter choice: ";
    std::cin >> choice;

    if (choice == 1) {
        add_item();
    }

    return 0;
}
