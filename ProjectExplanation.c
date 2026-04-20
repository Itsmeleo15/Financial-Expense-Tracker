#include <stdio.h>    // Standard I/O functions (printf, scanf, fgets, fopen, etc.)
#include <string.h>   // String functions (strcspn, strlen, strcmp, strncpy)
#include <stdlib.h>   // General utilities (system())

#define MAX 100           // Maximum number of expenses that can be stored
#define FILENAME "expenses.dat"  // Binary file used to save/load expense data

// ANSI escape codes for colored terminal output
#define RED     "\033[1;31m"   // Bold red — used for errors and warnings
#define GREEN   "\033[1;32m"   // Bold green — used for success messages
#define RESET   "\033[0m"      // Resets color back to default
#define CYAN    "\033[0;36m"   // Cyan — used for dividers and table headers
#define WHITE   "\033[1;37m"   // Bold white — used for labels and prompts
#define YELLOW  "\033[1;33m"   // Yellow — used for values and user input


// Struct that represents a single expense entry
struct Expense {
    int id;            // Unique identifier for the expense
    char category[50]; // Name/label of the expense category (e.g., "Food")
    float amount;      // Monetary amount of the expense
};

struct Expense expenses[MAX]; // Array to hold all expense records (up to MAX)
int count = 0;                // Tracks how many expenses are currently stored
float budget = 0;             // The total budget set by the user
float remainingBudget = 0;    // Budget left after deducting all expenses


// Clears leftover characters in the input buffer (prevents scanf issues)
void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Read and discard until newline or end
}

// Clears the terminal screen — uses "cls" on Windows, "clear" on Linux/Mac
void clearScreen() {
    #ifdef _WIN32
        system("cls");   // Windows command
    #else
        system("clear"); // Unix/Linux/Mac command
    #endif        
}

// Pauses the program and waits for the user to press Enter before continuing
void pressEnter() {
    printf(WHITE "\n Press Enter to continue..." RESET);
    flushInput(); // Waits for the Enter key
}

// Checks if an expense with the given ID already exists in the array
// Returns 1 (true) if found, 0 (false) if not
int idExists(int id) {
    int i;
    for (i = 0; i < count; i++){
        if(expenses[i].id == id) return 1; // ID match found
    }
    return 0; // No match found
}

// Prints a horizontal divider line in cyan for visual separation
void printDivider() {
    printf(CYAN " ------------------------------------------------\n" RESET);
}

// Saves the current budget, remaining budget, count, and all expenses to a binary file
void saveData() {
    FILE *fp = fopen(FILENAME, "wb"); // Open file for writing in binary mode
    if (fp == NULL) {
        // File could not be opened — notify user but don't crash
        printf(RED "\n  Warning: Could not save data to file.\n" RESET);
        return;
    }

    // Write data fields one by one into the binary file
    fwrite(&budget,          sizeof(float),          1,     fp); // Save total budget
    fwrite(&remainingBudget, sizeof(float),          1,     fp); // Save remaining budget
    fwrite(&count,           sizeof(int),            1,     fp); // Save number of records
    fwrite(expenses,         sizeof(struct Expense), count, fp); // Save all expense entries
    fclose(fp); // Close the file after writing
}

// Loads previously saved data from the binary file into memory
void loadData() {
    FILE *fp = fopen(FILENAME, "rb"); // Open file for reading in binary mode
    if (fp == NULL) return;           // If file doesn't exist yet, skip loading

    // Read budget, remaining budget, and count — if any read fails, reset and exit
    if (fread(&budget,          sizeof(float), 1, fp) != 1 ||
        fread(&remainingBudget, sizeof(float), 1, fp) != 1 ||
        fread(&count,           sizeof(int),   1, fp) != 1) {
        budget = remainingBudget = 0; // Reset to safe defaults
        count  = 0;
        fclose(fp);
        return;
    }
    
    // Validate count to prevent out-of-bounds array access
    if (count < 0 || count > MAX) {
        count = 0; // Corrupted count — reset
        fclose(fp);
        return;
    }

    // Read the expense records; if fewer than expected are read, reset count
    if ((int)fread(expenses, sizeof(struct Expense), count, fp) != count) {
        count = 0; // Incomplete data — reset
    }

    fclose(fp); // Close file after reading
}

// Allows the user to set a total budget (only when no expenses exist)
void setBudget() {
    // Prevent budget reset if expenses are already recorded
    if (count > 0){
        printf(RED "\n  Cannot reset budget while expenses are recorded.\n"
                   "  Delete all expenses first.\n\n" RESET);
        return;           
    }

    // Prompt user for budget value
    printf(WHITE "  Enter Total Budget: " RESET YELLOW);
    if(scanf("%f", &budget) != 1){   // Validate that a valid number was entered
        flushInput();
        printf(RESET RED "\n  Invalid input. Please enter a number.\n\n" RESET);
        budget = 0; // Reset on invalid input
        return;
    }
    flushInput(); // Clear the input buffer after reading
    
    // Budget must be a positive number
    if (budget <= 0) {
        printf(RESET RED "\n Budget must be greater than zero.\n\n" RESET);
        budget = 0;
        return;
    }

    remainingBudget = budget; // Initialize remaining budget to the full budget
    saveData();               // Persist the new budget to file

    printf(GREEN "\n  Budget set successfully!\n" RESET);
    printf(WHITE "  Current Budget: "YELLOW" %.2f\n" RESET, remainingBudget);
    printf("\n");
}

// Adds a new expense entry to the expenses array
void addExpense() {
    // Check if the array is already full
    if (count >= MAX){
        printf("\n" RED "Expense list is full.\n" RESET "\n");
        return;
    }
    
    // Budget must be set before adding any expense
    if (budget <= 0) {
        printf(RED "\n  Please set a budget first.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Add New Expense\n" RESET);
    printDivider();
    
    // Read and validate the expense ID
    int newId;
    printf(WHITE "\n  Expense ID: " RESET YELLOW);
    if(scanf("%d", &newId) != 1) { // Validate numeric input
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();
    
    if (newId <= 0) { // ID must be a positive integer
        printf(RESET RED "\n ID must be a positive number.\n\n" RESET);
        return;
    }
    if (idExists(newId)) { // Prevent duplicate IDs
        printf(RESET RED "\n  Expense with ID %d already exists.\n\n" RESET, newId);
        return;
    }

    expenses[count].id = newId; // Assign the validated ID to the new entry

    // Read and validate the expense category
    printf(RESET WHITE "  Category: " RESET YELLOW);
    if (fgets(expenses[count].category, sizeof(expenses[count].category), stdin) == NULL) {
        printf(RESET RED "\n Invalid category.\n\n" RESET);
        return;
    }
    expenses[count].category[strcspn(expenses[count].category, "\n")] = '\0'; // Remove trailing newline
    if (strlen(expenses[count].category) == 0) { // Category cannot be blank
        printf(RESET RED "\n Category cannot be empty.\n\n" RESET);
        return;
    }

    // Read and validate the expense amount
    printf(RESET WHITE "  Amount: " RESET YELLOW);
    if (scanf("%f", &expenses[count].amount) !=1){
        flushInput();
        printf(RESET RED "\n Invalid amount.\n\n" RESET);
        return;
    }
    flushInput();

    if (expenses[count].amount <= 0) { // Amount must be positive
        printf(RED "\n Amount must be greater than zero.\n\n" RESET);
        return;
    }
    if(expenses[count].amount > remainingBudget) { // Cannot exceed remaining budget
        printf(RED "\n  Not enough budget for this expense!\n\n" RESET);
        return;
    }

    remainingBudget -= expenses[count].amount; // Deduct amount from remaining budget
    count++;      // Increment the expense counter
    saveData();   // Save updated data to file

    printf("\n" GREEN "  Expense added successfully!\n" RESET);  
    printf("\n" WHITE "  Remaining budget: "YELLOW" %.2f\n\n" RESET, remainingBudget); 
}

// Displays all recorded expenses in a formatted table
void viewExpenses() {
    int i;

    // Nothing to display if no expenses exist
    if (count == 0) {
        printf("\n" RED "\n  No Expenses recorded yet.\n\n" RESET "\n");
        return;
    }

    // Print table header
    printf("\n" YELLOW "\n  ------------- EXPENSE LIST -------------\n" RESET "\n");
    printf(CYAN "  %-5s | %-20s   | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();
    
    // Loop through and print each expense record in aligned columns
    for(i = 0; i < count; i++) {
        printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
        expenses[i].id,
        expenses[i].category,
        expenses[i].amount);
    }

    printDivider();
    printf(WHITE "  Total Expenses : " YELLOW "%d record(s)\n\n" RESET, count); // Print total record count
    printf("\n");
}

// Shows an expense summary: total spent, remaining budget, usage percentage, and a progress bar
void totalExpenses() {
    int i;
    float total = 0.0f;   // Accumulator for total amount spent
    float percent = 0.0f; // Percentage of budget used
    int filled;           // Number of filled blocks in the progress bar
    int b;                // Loop counter for the progress bar

    // Nothing to summarize if no expenses exist
    if(count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    // Sum up all expense amounts
    for (i = 0; i < count; i++) total += expenses[i].amount;

    // Calculate percentage of budget used (only if budget is set)
    if (budget > 0) percent = (total / budget) * 100.0f;

    // Display the summary block
    printf(YELLOW "\n  ----------- EXPENSE SUMMARY ------------\n\n" RESET);
    printf(WHITE "  Total Budget   : " YELLOW "%.2f\n"      RESET, budget);
    printf(WHITE "  Total Spent    : " YELLOW "%.2f\n"      RESET, total);
    printf(WHITE "  Remaining      : " YELLOW "%.2f\n"      RESET, remainingBudget);
    printf(WHITE "  Budget Used    : " YELLOW "%.1f%%\n\n"  RESET, percent);

    // Calculate how many blocks (out of 20) to fill — each block = 5%
    filled = (int)(percent / 5.0f);
    if (filled > 20) filled = 20; // Cap at 20 blocks maximum

    // Draw the progress bar: '#' for used, '-' for remaining
    printf(WHITE "  [" RESET);
    for (b = 0; b < 20; b++) {
        if (b < filled)
            printf("%s#%s", percent >= 90 ? RED : GREEN, RESET); // Red if >=90%, green otherwise
        else
            printf(CYAN "-" RESET); // Unfilled blocks are cyan dashes
    }
    printf(WHITE "] " YELLOW "%.1f%%\n\n" RESET, percent); // Show percentage next to the bar
}

// Searches for expenses by exact category name and shows matching results
void searchExpense() {
    char search[50]; // Holds the category name entered by the user
    int i, found = 0;    // found flag to track if any match was found
    float catTotal = 0.0f; // Accumulates total amount for matched category

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW"  Search by Category\n" RESET);
    printDivider();   
    printf(WHITE "  Enter category: " RESET YELLOW);

    // Read the search term from input
    if (fgets(search, sizeof(search), stdin) == NULL) {
        printf(RESET RED "\n  Invalid input.\n\n" RESET);
        return;
    }
    search[strcspn(search, "\n")] = '\0'; // Remove trailing newline
    printf(RESET "\n\n");

    // Print table header for search results
    printf(CYAN " %-5s | %-22s | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();

    // Loop through all expenses and print matches
    for (i = 0; i < count; i++){
        if (strcmp(expenses[i].category, search) == 0) { // Exact match check
            printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
                   expenses[i].id,
                   expenses[i].category,
                   expenses[i].amount);
            catTotal += expenses[i].amount; // Accumulate matched amounts
            found = 1; // At least one match was found
        }
    }

    if (!found){
        printf(RED "  No expenses found for category \"%s\".\n\n" RESET, search);
    } else {
        printDivider();
        printf(WHITE "  Category Total : " YELLOW "%.2f\n\n" RESET, catTotal); // Show category total
    }
}

// Allows the user to edit the category and amount of an existing expense by ID
void editExpense() {
    int id, i;
    float newAmount;
    char newCategory[50];

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW"  Edit Expense\n" RESET);
    printDivider();
    printf(WHITE "  Enter ID to edit: " RESET YELLOW);
    if(scanf("%d", &id) != 1){ // Validate ID input
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    // Search for the expense with the matching ID
    for (i = 0; i < count; i++){
        if (expenses[i].id == id){

            // Display current values before editing
            printf(RESET CYAN "\n  Current Category : " WHITE "%s\n" RESET, expenses[i].category);
            printf(CYAN       "  Current Amount   : " WHITE "%.2f\n\n" RESET, expenses[i].amount);

            // Temporarily restore the old amount to remaining budget before applying the new one
            remainingBudget += expenses[i].amount;

            // Read new category
            printf(WHITE "  New category    : " RESET YELLOW);
            if (fgets(newCategory, sizeof(newCategory), stdin) == NULL) {
                printf(RESET RED "\n  Invalid category. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount; // Undo restoration if cancelled
                return;
            }
            newCategory[strcspn(newCategory, "\n")] = '\0'; // Strip newline
            if (strlen(newCategory) == 0) {
                printf(RESET RED "\n Category cannot be empty. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount; // Undo restoration if cancelled
                return;
            }

            // Read new amount
            printf(WHITE"  New amount    : " RESET YELLOW);
            if (scanf("%f", &newAmount) != 1){
                flushInput();
                printf(RESET RED "\n  Invalid amount. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount; // Undo restoration if cancelled
                return;
            }
            flushInput();
            
            if (newAmount <= 0) { // Amount must be positive
                printf(RED "\n Amount must be greater than zero. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }

            if (newAmount > remainingBudget){ // New amount must fit within remaining budget
                printf(RED "  Not enough budget for this amount!\n\n" RESET);
                remainingBudget -= expenses[i].amount; // Undo restoration
                return;
            }
            
            // Apply the updates to the expense record
            strncpy(expenses[i].category, newCategory, sizeof(expenses[i].category) - 1);
            expenses[i].category[sizeof(expenses[i].category) - 1] = '\0'; // Ensure null termination
            expenses[i].amount = newAmount;
            remainingBudget -= newAmount; // Deduct the new amount from budget
            saveData(); // Persist changes

            printf("\n" GREEN "  Expense updated successfully!\n" RESET);
            printf("\n" WHITE "  Remaining budget: "YELLOW" %.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

    printf(RED "  Expense with ID %d not found.\n\n" RESET, id); // No matching ID found
}

// Deletes a single expense by ID and restores its amount to the remaining budget
void deleteExpense() {
    int id, i, j;

    if(count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW "  Delete Expense\n" RESET);
    printDivider();
    printf(WHITE "  Enter ID to delete: " RESET YELLOW);
    if (scanf("%d", &id) != 1) { // Validate ID input
        flushInput();
        printf(RESET RED "\n Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    // Search for the expense to delete
    for(i = 0; i < count; i++) {
        if(expenses[i].id == id) {
            // Show what is being deleted
            printf(RESET CYAN "\n  Deleting: [%s] %.2f\n" RESET,
                   expenses[i].category, expenses[i].amount);

            remainingBudget += expenses[i].amount; // Restore deleted amount to budget

            // Shift remaining elements left to fill the gap
            for(j = i; j < count - 1; j++) {
                expenses[j] = expenses[j + 1];
            }
            count--;    // Decrease the expense count
            saveData(); // Save updated data to file

            printf(GREEN "\n  Expense deleted successfully!\n" RESET);
            printf(WHITE "  Remaining budget: "YELLOW "%.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

    printf(RED"\n  Expense with ID %d not found.\n\n" RESET, id); // ID not found
}

// Deletes ALL expenses and resets the budget after user confirmation
void deleteAllExpenses() {
    char confirm[10]; // Holds the user's confirmation input

    if (count == 0) {
        printf(RED "\n No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n Delete All Expenses\n" RESET);
    printDivider();
    printf(RED " WARNING: This will delete all %d expense(s) and clear the budget!\n" RESET, count);
    printf(WHITE " Type 'yes' to confirm: " RESET YELLOW);

    // Read confirmation input
    if (fgets(confirm, sizeof(confirm), stdin) == NULL) {
        printf(RESET RED "\n Invalid input.\n\n" RESET);
        return;
    }
    confirm[strcspn(confirm, "\n")] = '\0'; // Strip newline
    printf(RESET);

    if (strcmp(confirm, "yes") == 0) { // Proceed only if user typed exactly "yes"
        count           = 0; // Reset expense count
        budget          = 0; // Clear the budget
        remainingBudget = 0; // Clear remaining budget
        saveData();          // Save the cleared state to file
        printf(GREEN "\n All expenses deleted and budget cleared.\n\n" RESET);      
    } else {
        printf(YELLOW "\n Operation cancelled.\n\n" RESET); // User did not confirm
    }
}

int main() {
    int choice; // Stores the user's menu selection
    loadData(); // Load any previously saved data from file on startup

    do {
        clearScreen(); // Clear terminal before showing the menu

        // Display the app header
        printf(CYAN "+=======================================+\n" RESET);
        printf(CYAN "|" YELLOW "       Personal Expense Tracker       " CYAN " |" RESET "\n");
        printf(CYAN "+=======================================+\n" RESET);

        // Display current budget status
        printf(WHITE "\n  Budget    : " YELLOW "%.2f\n" RESET, budget);
        printf(WHITE "  Remaining : ");

        // Show remaining budget in red with warning if below 20% of total budget
        if (remainingBudget < budget * 0.2f && budget > 0)
            printf(RED   "%.2f  (!)\n" RESET, remainingBudget); // Low budget warning
        else
            printf(YELLOW "%.2f\n" RESET, remainingBudget);

        printf(WHITE "  Expenses  : " YELLOW "%d record(s)\n\n" RESET, count);

        // Display all menu options
        printf(WHITE " 1." CYAN  " Set Budget\n"          RESET);
        printf(WHITE " 2." CYAN  " Add Expense\n"         RESET);
        printf(WHITE " 3." CYAN  " View Expenses\n"       RESET);
        printf(WHITE " 4." CYAN  " Expense Summary\n"     RESET);
        printf(WHITE " 5." CYAN  " Search by Category\n"  RESET);
        printf(WHITE " 6." CYAN  " Edit Expense\n"        RESET);
        printf(WHITE " 7." CYAN  " Delete Expense\n"      RESET);
        printf(WHITE " 8." CYAN  " Delete All\n"          RESET);
        printf(WHITE " 9." CYAN  " Exit\n\n"              RESET);

        // Prompt user for their choice
        printf(WHITE "  Your Choice: " RESET YELLOW);
        if (scanf("%d", &choice) != 1) { // Validate that a number was entered
            flushInput();
            printf(RESET RED "\n Invalid input. Please enter a number from 1 to 9.\n\n" RESET);
            pressEnter();
            continue; // Restart the loop without processing an invalid choice
        }
        flushInput();
        printf(RESET);

        // Route to the appropriate function based on the user's choice
        switch(choice) {   
            case 1:  setBudget();        break; // Set or update the budget
            case 2:  addExpense();       break; // Add a new expense
            case 3:  viewExpenses();     break; // View all expenses
            case 4:  totalExpenses();    break; // Show summary and progress bar
            case 5:  searchExpense();    break; // Search expenses by category
            case 6:  editExpense();      break; // Edit an existing expense
            case 7:  deleteExpense();    break; // Delete one expense by ID
            case 8:  deleteAllExpenses(); break; // Delete all expenses and reset budget
            case 9:
                printf("\n" GREEN "  Goodbye! Stay on budget." RESET "\n"); // Exit message
                break;
            default:
                printf("\n" RED "  Invalid choice. Please enter a number from 1 to 9.\n" RESET "\n");
        }

        if (choice != 9) pressEnter(); // Pause before refreshing the menu (except on exit)

    } while(choice != 9); // Keep looping until user chooses to exit

    return 0; // Program ends successfully
}
