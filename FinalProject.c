#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 100
#define FILENAME "expenses.dat"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"
#define CYAN    "\033[0;36m"
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

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif        
}

void pressEnter() {
    printf(WHITE "\n Press Enter to continue..." RESET);
    flushInput();
}

int idExists(int id) {
    int i;
    for (i = 0; i < count; i++){
        if(expenses[i].id == id) return 1;
    }
    return 0;
}

void printDivider() {
    printf(CYAN " ------------------------------------------------\n" RESET);
}

void saveData() {
    FILE *fp = fopen(FILENAME, "wb");
    if (fp == NULL) {
        printf(RED "\n  Warning: Could not save data to file.\n" RESET);
        return;
    }

    fwrite(&budget,          sizeof(float),          1,     fp);
    fwrite(&remainingBudget, sizeof(float),          1,     fp);
    fwrite(&count,           sizeof(int),            1,     fp);
    fwrite(expenses,         sizeof(struct Expense), count, fp);
    fclose(fp);
}

void loadData() {
    FILE *fp = fopen(FILENAME, "rb");
    if (fp == NULL) return;

    if (fread(&budget,          sizeof(float), 1, fp) != 1 ||
        fread(&remainingBudget, sizeof(float), 1, fp) != 1 ||
        fread(&count,           sizeof(int),   1, fp) != 1) {
        budget = remainingBudget = 0;
        count  = 0;
        fclose(fp);
        return;
    }
    
    if (count < 0 || count > MAX) {
        count = 0;
        fclose(fp);
        return;
    }

    if ((int)fread(expenses, sizeof(struct Expense), count, fp) != count) {
        count = 0;
    }

    fclose(fp);
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
        return;
    }
    flushInput();
    
    if (budget <= 0) {
        printf(RESET RED "\n Budget must be greater than zero.\n\n" RESET);
        budget = 0;
        return;
    }

    remainingBudget = budget;
    saveData();

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
    printDivider();
    
    int newId;
    printf(WHITE "\n  Expense ID: " RESET YELLOW);
    if(scanf("%d", &newId) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();
    
    if (newId <= 0) {
        printf(RESET RED "\n ID must be a positive number.\n\n" RESET);
        return;
    }
    if (idExists(newId)) {
        printf(RESET RED "\n  Expense with ID %d already exists.\n\n" RESET, newId);
        return;
    }

    expenses[count].id = newId;

    printf(RESET WHITE "  Category: " RESET YELLOW);
    if (fgets(expenses[count].category, sizeof(expenses[count].category), stdin) == NULL) {
        printf(RESET RED "\n Invalid category.\n\n" RESET);
        return;
    }
    expenses[count].category[strcspn(expenses[count].category, "\n")] = '\0';
    if (strlen(expenses[count].category) == 0) {
        printf(RESET RED "\n Category cannot be empty.\n\n" RESET);
        return;
    }

    printf(RESET WHITE "  Amount: " RESET YELLOW);
    if (scanf("%f", &expenses[count].amount) !=1){
        flushInput();
        printf(RESET RED "\n Invalid amount.\n\n" RESET);
        return;
    }
    flushInput();

    if (expenses[count].amount <= 0) {
        printf(RED "\n Amount must be greater than zero.\n\n" RESET);
        return;
    }
    if(expenses[count].amount > remainingBudget) {
        printf(RED "\n  Not enough budget for this expense!\n\n" RESET);
        return;
    }

    remainingBudget -= expenses[count].amount;
    count++;
    saveData();

    printf("\n" GREEN "  Expense added successfully!\n" RESET);  
    printf("\n" WHITE "  Remaining budget: "YELLOW" %.2f\n\n" RESET, remainingBudget); 
}
void viewExpenses() {
    int i;

    if (count == 0) {
        printf("\n" RED "\n  No Expenses recorded yet.\n\n" RESET "\n");
        return;
    }
    printf("\n" YELLOW "\n  ------------- EXPENSE LIST -------------\n" RESET "\n");
    printf(CYAN "  %-5s | %-20s   | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();
    
    for(i = 0; i < count; i++) {
        printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
        expenses[i].id,
        expenses[i].category,
        expenses[i].amount);
    }

    printDivider();
    printf(WHITE "  Total Expenses : " YELLOW "%d record(s)\n\n" RESET, count);

    printf("\n");
}
void totalExpenses() {
    int i;
    float total = 0.0f;
    float percent = 0.0f;
    int filled;
    int b;

    if(count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    for (i = 0; i < count; i++) total += expenses[i].amount;
    if (budget > 0) percent = (total / budget) * 100.0f;

    printf(YELLOW "\n  ----------- EXPENSE SUMMARY ------------\n\n" RESET);
    printf(WHITE "  Total Budget   : " YELLOW "%.2f\n"      RESET, budget);
    printf(WHITE "  Total Spent    : " YELLOW "%.2f\n"      RESET, total);
    printf(WHITE "  Remaining      : " YELLOW "%.2f\n"      RESET, remainingBudget);
    printf(WHITE "  Budget Used    : " YELLOW "%.1f%%\n\n"  RESET, percent);

    filled = (int)(percent / 5.0f);
    if (filled > 20) filled = 20;

    printf(WHITE "  [" RESET);
    for (b = 0; b < 20; b++) {
        if (b < filled)
            printf("%s#%s", percent >= 90 ? RED : GREEN, RESET);  
        else
            printf(CYAN "-" RESET);
    }
    printf(WHITE "] " YELLOW "%.1f%%\n\n" RESET, percent);
}
void searchExpense() {
    char search[50];
    int i, found = 0;
    float catTotal = 0.0f;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }
    printf("\n");
    printf(YELLOW"  Search by Category\n" RESET);
    printDivider();   
    printf(WHITE "  Enter category: " RESET YELLOW);
    if (fgets(search, sizeof(search), stdin) == NULL) {
        printf(RESET RED "\n  Invalid input.\n\n" RESET);
        return;
    }
    search[strcspn(search, "\n")] = '\0';
    printf(RESET "\n");

    printf("\n");

    printf(CYAN " %-5s | %-22s | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();

    for (i = 0; i < count; i++){
        if (strcmp(expenses[i].category, search) == 0) {
            printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
                   expenses[i].id,
                   expenses[i].category,
                   expenses[i].amount);
            catTotal += expenses[i].amount;
            found = 1;
        }
    }

    if (!found){
        printf(RED "  No expenses found for category \"%s\".\n\n" RESET, search);
    } else {
        printDivider();
        printf(WHITE "  Category Total : " YELLOW "%.2f\n\n" RESET, catTotal);
    }
}

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
    if(scanf("%d", &id) != 1){
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    for (i = 0; i < count; i++){
        if (expenses[i].id == id){

            printf(RESET CYAN "\n  Current Category : " WHITE "%s\n" RESET, expenses[i].category);
            printf(CYAN       "  Current Amount   : " WHITE "%.2f\n\n" RESET, expenses[i].amount);

            remainingBudget += expenses[i].amount;


            printf(WHITE "  New category    : " RESET YELLOW);
            if (fgets(newCategory, sizeof(newCategory), stdin) == NULL) {
                printf(RESET RED "\n  Invalid category. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            newCategory[strcspn(newCategory, "\n")] = '\0';
            if (strlen(newCategory) == 0) {
                printf(RESET RED "\n Category cannot be empty. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }

            printf(WHITE"  New amount    : " RESET YELLOW);
            if (scanf("%f", &newAmount) != 1){
                flushInput();
                printf(RESET RED "\n  Invalid amount. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            flushInput();
            
            if (newAmount <= 0) {
                printf(RED "\n Amount must be greater than zero. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }

            if (newAmount > remainingBudget){
                printf(RED "  Not enough budget for this amount!\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            
            strncpy(expenses[i].category, newCategory, sizeof(expenses[i].category) - 1);
            expenses[i].category[sizeof(expenses[i].category) - 1] = '\0';
            expenses[i].amount = newAmount;
            remainingBudget -= newAmount;
            saveData();

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
    printDivider();
    printf(WHITE "  Enter ID to delete: " RESET YELLOW);
    if (scanf("%d", &id) != 1) {
        flushInput();
        printf(RESET RED "\n Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    for(i = 0; i < count; i++) {
        if(expenses[i].id == id) {
            printf(RESET CYAN "\n  Deleting: [%s] %.2f\n" RESET,
                   expenses[i].category, expenses[i].amount);

            remainingBudget += expenses[i].amount;

            for(j = i; j < count - 1; j++) {
                expenses[j] = expenses[j + 1];
            }
            count--;
            saveData();
            printf(GREEN "\n  Expense deleted successfully!\n" RESET);
            printf(WHITE "  Remaining budget: "YELLOW "%.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

     printf(RED"\n  Expense with ID %d not found.\n\n" RESET, id);
}

void deleteAllExpenses() {
    char confirm[10];

    if (count == 0) {
        printf(RED "\n No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n Delete All Expenses\n" RESET);
    printDivider();
    printf(RED " WARNING: This will delete all %d expense(s) and clear the budget!\n" RESET, count);
    printf(WHITE " Type 'yes' to confirm: " RESET YELLOW);
    if (fgets(confirm, sizeof(confirm), stdin) == NULL) {
        printf(RESET RED "\n Invalid input.\n\n" RESET);
        return;
    }
    confirm[strcspn(confirm, "\n")] = '\0';
    printf(RESET);

    if (strcmp(confirm, "yes") == 0) {
        count           = 0;
        budget          = 0;
        remainingBudget = 0;
        saveData();
        printf(GREEN "\n All expenses deleted and budget cleared.\n\n" RESET);      
    } else {
        printf(YELLOW "\n Operation cancelled.\n\n" RESET);
    }
}

int main() {
    int choice;
    loadData();


    do {
        clearScreen();

        printf(CYAN "+=======================================+\n" RESET);
        printf(CYAN "|" YELLOW "       Personal Expense Tracker       " CYAN " |" RESET "\n");
        printf(CYAN "+=======================================+\n" RESET);

        printf(WHITE "\n  Budget    : " YELLOW "%.2f\n" RESET, budget);
        printf(WHITE "  Remaining : ");
        if (remainingBudget < budget * 0.2f && budget > 0)
            printf(RED   "%.2f  (!)\n" RESET, remainingBudget);
        else
            printf(YELLOW "%.2f\n" RESET, remainingBudget);
        printf(WHITE "  Expenses  : " YELLOW "%d record(s)\n\n" RESET, count);

        printf(WHITE " 1." CYAN  " Set Budget\n"       RESET);
        printf(WHITE " 2." CYAN  " Add Expense\n"      RESET);
        printf(WHITE " 3." CYAN  " View Expenses\n"    RESET);
        printf(WHITE " 4." CYAN  " Expense Summary\n"  RESET);
        printf(WHITE " 5." CYAN  " Search by Category\n" RESET);
        printf(WHITE " 6." CYAN  " Edit Expense\n"     RESET);
        printf(WHITE " 7." CYAN  " Delete Expense\n"   RESET);
        printf(WHITE " 8." CYAN  " Delete All\n"       RESET);
        printf(WHITE " 9." CYAN  " Exit\n\n"            RESET);

        printf(WHITE "  Your Choice: " RESET YELLOW);
        if (scanf("%d", &choice) != 1) {
            flushInput();
            printf(RESET RED "\n Invalid input. Please enter a number from 1 to 9.\n\n" RESET);
            pressEnter();
            continue;
        }
        flushInput();
        printf(RESET);

        switch(choice) {   
            
            case 1:  setBudget();        break;
            case 2:  addExpense();       break;
            case 3:  viewExpenses();     break;
            case 4:  totalExpenses();    break;
            case 5:  searchExpense();    break;
            case 6:  editExpense();      break;
            case 7:  deleteExpense();    break;
            case 8:  deleteAllExpenses(); break;
            case 9:
            printf("\n" GREEN "  Goodbye! Stay on budget." RESET "\n");
            break;

        default:
            printf("\n" RED "  Invalid choice. Please enter a number from 1 to 9.\n" RESET "\n");
        }

        if (choice != 9) pressEnter();

    } while(choice != 9);

    return 0;
}
