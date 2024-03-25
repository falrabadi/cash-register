#include <iostream>
#include <iomanip>
#include <sqlite3.h>
#include <string>
#include <optional>
#include <vector>
#include <ctime>
#include <cstdlib> // for std::atoi

struct Item {
    std::string barcodeID;
    std::string name;
    float price;
    float cost;
};

struct Sale {
    std::time_t dateTime;
    std::vector<Item> items;
    std::string paymentType;
    float totalPrice;
    float totalCost;
};

// Global variable for debug mode
bool debug = false;

// Function declarations
void add_item();
void delete_item();
void update_item();
Item getItemByBarcode(const std::string& barcode);
int callback(void* NotUsed, int argc, char** argv, char** azColName);
void create_sales_table(sqlite3* db);
void insert_sale(sqlite3* db, const Sale& sale);
void sales_transaction();


int main(int argc, char* argv[]) {
    // Check if debug flag is provided as a command-line argument
    if (argc > 1 && std::atoi(argv[1]) == 1) {
        debug = true;
        std::cout << "Debug mode enabled." << std::endl;
    }

    int choice;
    do {
        std::cout << "Select mode:" << std::endl;
        std::cout << "1. Manage Items" << std::endl;
        std::cout << "2. Sales Transaction" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                // Manage Items mode
                int itemChoice;
                do {
                    std::cout << "Manage Items:" << std::endl;
                    std::cout << "1. Add Item" << std::endl;
                    std::cout << "2. Delete Item" << std::endl;
                    std::cout << "3. Update Item" << std::endl;
                    std::cout << "0. Exit Manage Items mode" << std::endl;
                    std::cout << "Enter your choice: ";
                    std::cin >> itemChoice;

                    switch (itemChoice) {
                        case 1:
                            add_item();
                            break;
                        case 2:
                            delete_item();
                            break;
                        case 3:
                            update_item();
                            break;
                        case 0:
                            std::cout << "Exiting Manage Items mode..." << std::endl;
                            break;
                        default:
                            std::cout << "Invalid choice. Please try again." << std::endl;
                            break;
                    }
                } while (itemChoice != 0);
                break;
            case 2:
                // Sales Transaction mode
                sales_transaction();
                break;
            case 0:
                std::cout << "Exiting program..." << std::endl;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
                break;
        }
    } while (choice != 0);

    return 0;
}



// Function to check if barcode exists in the database and return the corresponding item
Item getItemByBarcode(const std::string& barcode) {
    sqlite3* db;
    int rc = sqlite3_open("items.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        // Return a default-constructed Item in case of error
        return Item();
    }

    std::string sql = "SELECT name, price, cost FROM items WHERE barcode = '" + barcode + "';";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        // Return a default-constructed Item in case of error
        return Item();
    }

    Item item;
    // Step through the result set
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        item.barcodeID = barcode;
        item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item.price = sqlite3_column_double(stmt, 1);
        item.cost = sqlite3_column_double(stmt, 2);

        // Check if items are properly being retrieved.
        if(debug){
            std::cout << item.barcodeID << std::endl;
            std::cout << item.name << std::endl;
            std::cout << item.price << std::endl;
            std::cout << item.cost << std::endl;
        }

    } else {
        // Barcode not found, set default values for item
        item = Item();
    }

    // Finalize statement and close database connection
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return item;
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
    Item existingItem = getItemByBarcode(newItem.barcodeID);
    if (!existingItem.barcodeID.empty()) {
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

// Function to delete an item from the SQLite3 database
void delete_item() {
    std::string barcode;
    std::cout << "Scan barcode of the item to delete: ";
    std::cin >> barcode;

    // Open database connection
    sqlite3* db;
    int rc = sqlite3_open("items.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Construct SQL query to delete item
    std::string deleteSQL = "DELETE FROM items WHERE barcode = '" + barcode + "';";

    // Execute SQL query
    char* errorMessage;
    rc = sqlite3_exec(db, deleteSQL.c_str(), nullptr, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Item with barcode " << barcode << " deleted successfully." << std::endl;
    }

    // Close database connection
    sqlite3_close(db);
}

// Function to update an item in the SQLite3 database
void update_item() {
    std::string barcode;
    std::cout << "Scan barcode of the item to update: ";
    std::cin >> barcode;

    // Open database connection
    sqlite3* db;
    int rc = sqlite3_open("items.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Check if the item with the provided barcode exists
    std::string selectSQL = "SELECT * FROM items WHERE barcode = '" + barcode + "';";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, selectSQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        std::cerr << "Item with barcode " << barcode << " not found." << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Item found, fetch its details
    std::string name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    float price = sqlite3_column_double(stmt, 2);
    float cost = sqlite3_column_double(stmt, 3);
    sqlite3_finalize(stmt);

    // Prompt user for updated information
    std::cout << "Enter new name: ";
    std::string newName;
    std::getline(std::cin >> std::ws, newName); // Read input including whitespaces
    if (newName.empty()) {
        newName = name;
    }

    std::cout << "Enter new price (or 0 to keep current): ";
    float newPrice;
    std::cin >> newPrice;
    if (newPrice == 0) {
        newPrice = price;
    }

    std::cout << "Enter new cost (or 0 to keep current): ";
    float newCost;
    std::cin >> newCost;
    if (newCost == 0) {
        newCost = cost;
    }

    // Construct SQL query to update item
    std::string updateSQL = "UPDATE items SET name = '" + newName + "', "
                            "price = " + std::to_string(newPrice) + ", "
                            "cost = " + std::to_string(newCost) + " "
                            "WHERE barcode = '" + barcode + "';";

    // Execute SQL query
    char* errorMessage;
    rc = sqlite3_exec(db, updateSQL.c_str(), nullptr, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Item with barcode " << barcode << " updated successfully." << std::endl;
    }

    // Close database connection
    sqlite3_close(db);
}


// Function to create the sales table in the database
void create_sales_table(sqlite3* db) {
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS sales (" \
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                                 "date_time DATETIME NOT NULL," \
                                 "payment_type TEXT NOT NULL," \
                                 "total_price REAL NOT NULL," \
                                 "total_cost REAL NOT NULL);";
    char* errorMessage;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }
}

// Function to insert a sale into the sales table
void insert_sale(sqlite3* db, const Sale& sale) {
    // Convert dateTime to a string in ISO 8601 format
    char dateTimeStr[20];
    std::strftime(dateTimeStr, sizeof(dateTimeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&sale.dateTime));

    std::string insertSQL = "INSERT INTO sales (date_time, payment_type, total_price, total_cost) VALUES ('" +
                            std::string(dateTimeStr) + "', '" +
                            sale.paymentType + "', " +
                            std::to_string(sale.totalPrice) + ", " +
                            std::to_string(sale.totalCost) + ");";

    if(debug){
        std::cout << sale.dateTime << std::endl;
        std::cout << sale.paymentType << std::endl;
        std::cout << sale.totalPrice << std::endl;
        std::cout << sale.totalCost << std::endl;
    }

    char* errorMessage;
    int rc = sqlite3_exec(db, insertSQL.c_str(), nullptr, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Sale recorded successfully." << std::endl;
    }
}


// Function to handle sales transactions
void sales_transaction() {
    // Open database connection
    sqlite3* db;
    int rc = sqlite3_open("sales.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Create sales table if it doesn't exist
    create_sales_table(db);

    Sale sale;
    sale.dateTime = std::time(nullptr);
    sale.totalPrice = 0.0; // Initialize totalPrice to 0.0
    sale.totalCost = 0.0; // Initialize totalCost to 0.0

    std::vector<Item> items;
    std::string barcode;
    do {
        std::cout << "Scan barcode (or enter 'done' to finish): ";
        std::cin >> barcode;

        // Retrieve item details from database based on barcode
        Item item = getItemByBarcode(barcode);

        if (!item.barcodeID.empty()) {
            // Output the name and price of the scanned item
            std::cout << "Item: " << item.name << ", Price: $" << std::fixed << std::setprecision(2) << item.price << std::endl;

            sale.items.push_back(item);
            sale.totalPrice += item.price;
            sale.totalCost += item.cost;
        } else if (barcode != "done") {
            std::cout << "Item not found." << std::endl;
        }
    } while (barcode != "done");

    // Print total cost
    std::cout << "Total: $" << std::fixed << std::setprecision(2) << sale.totalPrice << std::endl;

    // Prompt user for payment type
    std::cout << "Enter payment type (cash or card): ";
    std::cin >> sale.paymentType;

    // Insert sale into database
    insert_sale(db, sale);

    // Close database connection
    sqlite3_close(db);
}