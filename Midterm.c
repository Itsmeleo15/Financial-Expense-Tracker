#include <stdio.h>
#include <string.h>

#define MAX 100
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"
#define CYAN    "\e[0;36m"
#define WHITE   "\033[1;37m"
#define YELLOW  "\033[1;33m"


struct Expense {
    int id;
    char category[50];
    float amount;

};

struct Expense expenses[MAX];
    int count = 0;
    float budget = 0;
    float remainingBudget = 0;

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int idExists(int id) {
    int i;
    for (i = 0; i < count; i++){
        if(expenses[i].id == id) return 1;
    }
    return 0;
}    

void setBudget() {
    if (count > 0){
        printf(RED "\n  Cannot reset budget while expenses are recorded.\n"
                   "  Delete all expenses first.\n\n" RESET);
        
        return;           
    }
    printf(WHITE "  Enter Total Budget: " RESET YELLOW);
    if(scanf("%f", &budget) != 1){
        flushInput();
        printf(RESET RED "\n  Invalid input. Please enter a number.\n\n" RESET);
        budget = 0;
    }
    flushInput();

    remainingBudget = budget;

    printf(GREEN "\n  Budget set successfully!\n" RESET);
    printf(WHITE "  Current Budget: "YELLOW" %.2f\n" RESET, remainingBudget);

    printf("\n");
}
void addExpense() {
    if (count >= MAX){
        printf("\n" RED "Expense list is full.\n" RESET "\n");
        return;
    }
    
    if (budget <= 0) {
        printf(RED "\n  Please set a budget first.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Add New Expense\n" RESET);
    
    int newId;
    printf(WHITE "\n  Expense ID: " RESET YELLOW);
    if(scanf("%d", &newId) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();
    if (idExists(newId)) {
        printf(RESET RED "\n  An expense with ID %d already exists.\n\n" RESET, newId);
        return;
    }

    expenses[count].id = newId;

    printf(WHITE "  Category: " RESET YELLOW);
    if (scanf("%49s", expenses[count].category) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid category.\n\n" RESET);
        return;
    }
    flushInput();

    printf(WHITE "  Amount: " RESET YELLOW);
    if (scanf("%f", &expenses[count].amount) !=1){
        flushInput();
        printf(RESET RED "\n Invalid amount.\n\n" RESET);
        return;
    }
    flushInput();

    if(expenses[count].amount > remainingBudget){
        printf(RED "\n  Not enough budget for this expense!\n\n" RESET);
        return;
    }

    remainingBudget -= expenses[count].amount;

    count++;

    printf("\n" GREEN "  Expense added successfully!\n" RESET);  
    printf("\n" WHITE "  Remaining budget: "YELLOW" %.2f\n\n" RESET, remainingBudget); 
}
void viewExpenses() {
    int i;

    if (count == 0) {
        printf("\n" RED "\n  No Expenses recorded yet.\n" RESET "\n");
        return;
    }
    printf("\n" YELLOW "\n  ------------- EXPENSE LIST -------------\n" RESET "\n");

    printf(CYAN "  %-5s | %-20s | %10s\n" RESET, "ID", "Category", "Amount");
    printf(CYAN "  ------------------------------------------\n" RESET);
    
    for(i = 0; i < count; i++) {

        printf(CYAN "  %-5d | %-20s | %10.2f\n" RESET,
        expenses[i].id,
        expenses[i].category,
        expenses[i].amount);
    }
    printf("\n");
}
void totalExpenses() {
    int i;
    float total = 0.0f;

    if(count == 0){
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    for (i = 0; i < count; i++){
        total += expenses[i].amount;
    }

    printf("\n");

    printf(YELLOW "  Expense Summary\n" RESET);

    printf(WHITE "  Total spent  : " YELLOW "%.2f\n" RESET, total);
    printf(WHITE "  Remaining    : " YELLOW "%.2f\n" RESET, remainingBudget);
    printf(WHITE "  Budget       : " YELLOW "%.2f\n\n" RESET, budget);
}
void searchExpense() {
    char search[50];
    int i, found = 0;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }
    printf("\n");
    printf(YELLOW"  Search by Category\n" RESET);
   
    printf(WHITE "  Enter category: " YELLOW);
    if (scanf("%49s", search) != 1){
        flushInput();
        printf(RESET RED "\n  Invalid input.\n\n" RESET);
        return;
    }
    flushInput();
    printf(RESET "\n");

    printf("\n");

    for (i = 0; i < count; i++){
        if (strcmp(expenses[i].category, search) == 0) {
            printf(WHITE "  ID: " YELLOW "%-6d" RESET WHITE "|  Amount: " YELLOW "%.2f\n\n" RESET,
            expenses[i].id,
            expenses[i].amount);

            found = 1;
        }
    }

    if (found == 0){
        printf(RED "  No expenses found for category \"%s\".\n\n" RESET, search);
    }
}
void editExpense() {
    int id, i;
    float newAmount;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW"  Edit Expense\n" RESET);

    printf(WHITE "  Enter ID to edit: " YELLOW);
    if(scanf("%d", &id) != 1){
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();
    printf(RESET);

    for(i = 0; i < count; i++){
        if (expenses[i].id == id){

            remainingBudget += expenses[i].amount;


            printf(WHITE "  New category    : " YELLOW);
            if (scanf("%49s", expenses[i].category) != 1){
                flushInput();
                printf(RESET RED "\n  Invalid category. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            flushInput();
            printf(RESET);

            printf(WHITE"  New amount    : " YELLOW);
            if (scanf("%f", &newAmount) != 1){
                flushInput();
                printf(RESET RED "\n  Invalid amount. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            flushInput();
            printf(RESET);

            if(newAmount > remainingBudget){
                printf(RED "  Not enough budget for this amount!\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }

            expenses[i].amount = newAmount;
            remainingBudget -= newAmount;

            printf("\n");

            printf(GREEN "  Expense updated successfully!\n" RESET);

            printf("\n");

            printf(WHITE "  Remaining budget: "YELLOW" %.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

    printf(RED "  Expense with ID %d not found.\n\n" RESET, id);
}
void deleteExpense() {
    int id, i, j;

    if(count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }
    printf("\n");
    printf(YELLOW "  Delete Expense\n" RESET);

    printf(WHITE "  Enter ID to delete: " YELLOW);
    if (scanf("%d", &id) != 1) {
        flushInput();
        printf(RESET RED "\n Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    for(i = 0; i < count; i++) {
        if(expenses[i].id == id)
        {
            remainingBudget += expenses[i].amount;

            for(j = i; j < count - 1; j++) {
                expenses[j] = expenses[j + 1];
            }


            count--;

            printf(GREEN "\n  Expense deleted successfully!\n" RESET);
            printf(WHITE "  Remaining budget: "YELLOW "%.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

     printf(RED"\n  Expense with ID %d not found.\n\n" RESET, id);
}

int main() {
    int choice;


    do {
        printf(CYAN);
        printf("+=======================================+\n");
        printf("|       Personal Expense Tracker        |\n");
        printf("+=======================================+\n");
        printf(RESET);

        printf(WHITE "\n  Budget: " YELLOW "%.2f" WHITE
           "   |   Remaining: " YELLOW "%.2f\n\n" RESET,
           budget, remainingBudget);

        printf("\n");

        printf(WHITE "1." CYAN" Set Budget\n" RESET);
        printf(WHITE "2." CYAN" Add Expense\n" RESET);
        printf(WHITE "3." CYAN" View Expenses\n" RESET);
        printf(WHITE "4." CYAN" Total Expenses\n" RESET);
        printf(WHITE "5." CYAN" Search Expense\n" RESET);
        printf(WHITE "6." CYAN" Edit Expense\n" RESET);
        printf(WHITE "7." CYAN" Delete Expense\n" RESET);
        printf(WHITE "8." CYAN" Exit\n\n" RESET);

        printf(WHITE "  Your Choice: " RESET YELLOW);
        if (scanf("%d", &choice) != 1) {
            flushInput();
            printf(RESET RED "\n Invalid input. Please enter a number from 1 to 8.\n\n" RESET);
            continue;
        }
        flushInput();

        switch(choice) {   
            
            case 1:
            setBudget();
            break;

            case 2:
            addExpense();
            break;
            
            case 3:
            viewExpenses();
            break;

            case 4:
            totalExpenses();
            break;

            case 5:
            searchExpense();
            break;

            case 6:
            editExpense();
            break;

            case 7:
            deleteExpense();
            break;

            case 8:
            if (count == 0) {
                printf("\n" RED "  Please add at least one expense before exiting.\n" RESET "\n");
            }
            else {
                printf("\n" GREEN "  Goodbye! Stay on budget." RESET "\n");
            }
            break;

        default:
            printf("\n" RED "  Invalid choice. Please enter a number from 1 to 8.\n" RESET "\n");
        }

    } while(choice != 8 || count == 0);

    return 0;
}