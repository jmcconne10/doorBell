```
#include <iostream>
#include <string>
#include <unordered_map>

// Enum Class

enum class User {
	Rand,
    Joe,
    Ethan,
};

// Generic Print Function

void display(User user) {
    switch (user) {
        case User::Joe: std::cout << "Joe\n"; break;
        case User::Ethan: std::cout << "Ethan\n"; break;
        case User::Rand: std::cout << "Random\n"; break;
    }
}

int main() {
    // Hashmap (poorly named)
    std::unordered_map<std::string, User> user_map = {
        { "joe", User::Joe },
        { "ethan", User::Ethan },
    };
    
    std::string input = "ethan";
    
    User my_enum = (user_map.count(input) != 0) ? (user_map.at(input)) : (User::Rand);
    
    display(my_enum);
}
```
