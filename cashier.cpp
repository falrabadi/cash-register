#include <iostream>
#include <map>
#include <vector>
#include <optional>
#include <iomanip>

struct Item {
    std::string name;
    float price;
};

// Function to check if barcode exists and return the corresponding item
std::optional<Item> getItemByBarcode(const std::map<std::string, std::vector<Item>>& itemsMap, const std::string& barcode) {
    auto it = itemsMap.find(barcode);
    if (it != itemsMap.end()) {
        // Barcode exists in the map, return the first item in the vector
        return it->second[0];
    } else {
        // Barcode does not exist in the map
        return std::nullopt;
    }
}

// Implement add_item() function.

// Implement scan_item() function.

// Implement sale function.


int main() {
    // Creating a map with string keys and vectors of items as values
    std::map<std::string, std::vector<Item>> itemsMap;

    itemsMap["083046000135"].push_back({"Water Bottle", 1.50});


    while (1)
    {
        std::string searchBarcode;
        std::cout << "Scan your barcode: ";
        std::cin >> searchBarcode;

        // Check if barcode exists and return the corresponding item
        auto item = getItemByBarcode(itemsMap, searchBarcode);
        if (item.has_value()) {
            std::cout << "Item found: " << item->name << ", Price: $" << std::fixed << std::setprecision(2) << item->price << std::endl;
        } else {
            std::cout << "Barcode not found in the map." << std::endl;
        }
    }
    
    

    return 0;
}