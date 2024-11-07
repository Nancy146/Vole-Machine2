#include "volemachine.h"

using namespace std;
int main() {
    Memory mem;
    MainUI mainUI;  // Create an instance of MainUI

    // Infinite loop to continuously display the menu and get user input
    while (true) {
        mainUI.displayMenu();

        int command;
        cout << "Enter your choice: ";
        cin >> command;

        mainUI.handleUserChoice(command);  // Process the user's choice
    }

    return 0;
}