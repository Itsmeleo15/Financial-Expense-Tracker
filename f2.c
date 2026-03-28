/*
 * ============================================================
 *   Personal Expense Tracker
 *   C Console System Development
 * ============================================================
 *   Features:
 *   - Set budget, Add, View, Edit, Delete expenses (CRUD)
 *   - Search by category with subtotal
 *   - Input validation (negative, zero, letters, overflow)
 *   - File I/O: auto-save and auto-load (expenses.dat)
 *   - Clear screen between menus for clean UX
 * ============================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Constants ────────────────────────────────────────────── */
#define MAX       100
#define FILENAME  "expenses.dat"

/* ── ANSI Color Codes ─────────────────────────────────────── */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[1;37m"
#define YELLOW  "\033[1;33m"
#define MAGENTA "\033[1;35m"

/* ── Data Structure ───────────────────────────────────────── */
struct Expense {
    int   id;
    char  category[50];
    float amount;
};

/* ── Global Variables ─────────────────────────────────────── */
struct Expense expenses[MAX];
int   count           = 0;
float budget          = 0;
float remainingBudget = 0;

/* ============================================================
 *  UTILITY FUNCTIONS
 * ============================================================ */

/* Clears leftover characters from stdin buffer */
void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Clears the terminal screen (cross-platform) */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Pauses until the user presses Enter */
void pressEnter() {
    printf(WHITE "\n  Press Enter to continue..." RESET);
    flushInput();
}

/* Returns 1 if an expense with the given ID already exists */
int idExists(int id) {
    int i;
    for (i = 0; i < count; i++) {
        if (expenses[i].id == id) return 1;
    }
    return 0;
}

/* Prints a styled section divider */
void printDivider() {
    printf(CYAN "  ------------------------------------------------\n" RESET);
}

/* ============================================================
 *  FILE I/O  (saves and loads all data to/from expenses.dat)
 * ============================================================ */

/* Save budget + all expenses to binary file */
void saveData() {
    FILE *fp = fopen(FILENAME, "wb");
    if (fp == NULL) {
        printf(RED "\n  Warning: Could not save data to file.\n" RESET);
        return;
    }
    /* Write budget values first, then expense records */
    fwrite(&budget,          sizeof(float),          1,     fp);
    fwrite(&remainingBudget, sizeof(float),          1,     fp);
    fwrite(&count,           sizeof(int),            1,     fp);
    fwrite(expenses,         sizeof(struct Expense), count, fp);
    fclose(fp);
}

/* Load previously saved data on program start */
void loadData() {
    FILE *fp = fopen(FILENAME, "rb");
    if (fp == NULL) return; /* No save file yet — normal on first run */

    /* Read header values */
    if (fread(&budget,          sizeof(float), 1, fp) != 1 ||
        fread(&remainingBudget, sizeof(float), 1, fp) != 1 ||
        fread(&count,           sizeof(int),   1, fp) != 1) {
        /* Corrupted file — reset to defaults */
        budget = remainingBudget = 0;
        count  = 0;
        fclose(fp);
        return;
    }

    /* Safety check on count before reading records */
    if (count < 0 || count > MAX) {
        count = 0;
        fclose(fp);
        return;
    }

    /* Read expense records */
    if ((int)fread(expenses, sizeof(struct Expense), count, fp) != count) {
        count = 0; /* Partial read — discard */
    }

    fclose(fp);
}

/* ============================================================
 *  MENU FEATURES
 * ============================================================ */

/* ── 1. Set Budget ──────────────────────────────────────────── */
void setBudget() {
    if (count > 0) {
        printf(RED "\n  Cannot reset budget while expenses are recorded.\n"
                   "  Please delete all expenses first.\n\n" RESET);
        return;
    }

    printf(WHITE "\n  Enter Total Budget: " RESET YELLOW);
    if (scanf("%f", &budget) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid input. Please enter a number.\n\n" RESET);
        budget = 0;
        return;
    }
    flushInput();

    /* Validate: budget must be a positive number */
    if (budget <= 0) {
        printf(RESET RED "\n  Budget must be greater than zero.\n\n" RESET);
        budget = 0;
        return;
    }

    remainingBudget = budget;
    saveData(); /* Persist immediately */

    printf(GREEN "\n  Budget set successfully!\n" RESET);
    printf(WHITE "  Current Budget : " YELLOW "%.2f\n\n" RESET, budget);
}

/* ── 2. Add Expense ─────────────────────────────────────────── */
void addExpense() {
    if (count >= MAX) {
        printf(RED "\n  Expense list is full (max %d).\n\n" RESET, MAX);
        return;
    }
    if (budget <= 0) {
        printf(RED "\n  Please set a budget first.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Add New Expense\n" RESET);
    printDivider();

    /* --- ID --- */
    int newId;
    printf(WHITE "  Expense ID  : " RESET YELLOW);
    if (scanf("%d", &newId) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid ID. Must be a number.\n\n" RESET);
        return;
    }
    flushInput();

    if (newId <= 0) {
        printf(RESET RED "\n  ID must be a positive number.\n\n" RESET);
        return;
    }
    if (idExists(newId)) {
        printf(RESET RED "\n  Expense with ID %d already exists.\n\n" RESET, newId);
        return;
    }
    expenses[count].id = newId;

    /* --- Category (supports spaces via fgets) --- */
    printf(RESET WHITE "  Category    : " RESET YELLOW);
    if (fgets(expenses[count].category, sizeof(expenses[count].category), stdin) == NULL) {
        printf(RESET RED "\n  Invalid category.\n\n" RESET);
        return;
    }
    expenses[count].category[strcspn(expenses[count].category, "\n")] = '\0';
    if (strlen(expenses[count].category) == 0) {
        printf(RESET RED "\n  Category cannot be empty.\n\n" RESET);
        return;
    }

    /* --- Amount --- */
    printf(RESET WHITE "  Amount      : " RESET YELLOW);
    if (scanf("%f", &expenses[count].amount) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid amount.\n\n" RESET);
        return;
    }
    flushInput();

    if (expenses[count].amount <= 0) {
        printf(RED "\n  Amount must be greater than zero.\n\n" RESET);
        return;
    }
    if (expenses[count].amount > remainingBudget) {
        printf(RED "\n  Not enough budget! Available: %.2f\n\n" RESET, remainingBudget);
        return;
    }

    remainingBudget -= expenses[count].amount;
    count++;
    saveData();

    printf(RESET GREEN "\n  Expense added successfully!\n" RESET);
    printf(WHITE "  Remaining Budget : " YELLOW "%.2f\n\n" RESET, remainingBudget);
}

/* ── 3. View Expenses ───────────────────────────────────────── */
void viewExpenses() {
    int i;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  ============= EXPENSE LIST =============\n\n" RESET);
    printf(CYAN "  %-5s | %-22s | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();

    for (i = 0; i < count; i++) {
        printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
               expenses[i].id,
               expenses[i].category,
               expenses[i].amount);
    }

    printDivider();
    printf(WHITE "  Total Expenses : " YELLOW "%d record(s)\n\n" RESET, count);
}

/* ── 4. Total / Summary ─────────────────────────────────────── */
void totalExpenses() {
    int   i;
    float total   = 0.0f;
    float percent = 0.0f;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    for (i = 0; i < count; i++) total += expenses[i].amount;
    if (budget > 0) percent = (total / budget) * 100.0f;

    printf(YELLOW "\n  =========== EXPENSE SUMMARY ============\n\n" RESET);
    printf(WHITE "  Total Budget   : " YELLOW "%.2f\n"      RESET, budget);
    printf(WHITE "  Total Spent    : " YELLOW "%.2f\n"      RESET, total);
    printf(WHITE "  Remaining      : " YELLOW "%.2f\n"      RESET, remainingBudget);
    printf(WHITE "  Budget Used    : " YELLOW "%.1f%%\n\n"  RESET, percent);

    /* Visual budget bar (20 blocks) */
    int filled = (int)(percent / 5.0f);
    if (filled > 20) filled = 20;
    printf(WHITE "  [" RESET);
    int b;
    for (b = 0; b < 20; b++) {
        if (b < filled)
            printf(percent >= 90 ? RED "█" RESET : GREEN "█" RESET);
        else
            printf(CYAN "░" RESET);
    }
    printf(WHITE "] " YELLOW "%.1f%%\n\n" RESET, percent);
}

/* ── 5. Search by Category ──────────────────────────────────── */
void searchExpense() {
    char  search[50];
    int   i, found  = 0;
    float catTotal  = 0.0f;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Search by Category\n" RESET);
    printDivider();
    printf(WHITE "  Enter category : " RESET YELLOW);
    if (fgets(search, sizeof(search), stdin) == NULL) {
        printf(RESET RED "\n  Invalid input.\n\n" RESET);
        return;
    }
    search[strcspn(search, "\n")] = '\0';
    printf(RESET "\n");

    printf(CYAN "  %-5s | %-22s | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();

    for (i = 0; i < count; i++) {
        if (strcmp(expenses[i].category, search) == 0) {
            printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
                   expenses[i].id,
                   expenses[i].category,
                   expenses[i].amount);
            catTotal += expenses[i].amount;
            found = 1;
        }
    }

    if (!found) {
        printf(RED "  No expenses found for \"%s\".\n\n" RESET, search);
    } else {
        printDivider();
        printf(WHITE "  Category Total : " YELLOW "%.2f\n\n" RESET, catTotal);
    }
}

/* ── 6. Edit Expense ────────────────────────────────────────── */
void editExpense() {
    int   id, i;
    float newAmount;
    char  newCategory[50];

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Edit Expense\n" RESET);
    printDivider();
    printf(WHITE "  Enter ID to edit : " RESET YELLOW);
    if (scanf("%d", &id) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    for (i = 0; i < count; i++) {
        if (expenses[i].id == id) {

            /* Show current values */
            printf(RESET CYAN "\n  Current Category : " WHITE "%s\n" RESET, expenses[i].category);
            printf(CYAN       "  Current Amount   : " WHITE "%.2f\n\n" RESET, expenses[i].amount);

            /* Temporarily free up the old amount */
            remainingBudget += expenses[i].amount;

            /* --- New Category --- */
            printf(WHITE "  New Category     : " RESET YELLOW);
            if (fgets(newCategory, sizeof(newCategory), stdin) == NULL) {
                printf(RESET RED "\n  Invalid input. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            newCategory[strcspn(newCategory, "\n")] = '\0';
            if (strlen(newCategory) == 0) {
                printf(RESET RED "\n  Category cannot be empty. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }

            /* --- New Amount --- */
            printf(RESET WHITE "  New Amount       : " RESET YELLOW);
            if (scanf("%f", &newAmount) != 1) {
                flushInput();
                printf(RESET RED "\n  Invalid amount. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            flushInput();

            if (newAmount <= 0) {
                printf(RED "\n  Amount must be greater than zero. Edit cancelled.\n\n" RESET);
                remainingBudget -= expenses[i].amount;
                return;
            }
            if (newAmount > remainingBudget) {
                printf(RED "\n  Not enough budget! Available: %.2f\n  Edit cancelled.\n\n" RESET, remainingBudget);
                remainingBudget -= expenses[i].amount;
                return;
            }

            /* Apply changes */
            strncpy(expenses[i].category, newCategory, sizeof(expenses[i].category) - 1);
            expenses[i].category[sizeof(expenses[i].category) - 1] = '\0';
            expenses[i].amount  = newAmount;
            remainingBudget    -= newAmount;
            saveData();

            printf(RESET GREEN "\n  Expense updated successfully!\n" RESET);
            printf(WHITE "  Remaining Budget : " YELLOW "%.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

    printf(RED "\n  Expense with ID %d not found.\n\n" RESET, id);
}

/* ── 7. Delete One Expense ──────────────────────────────────── */
void deleteExpense() {
    int id, i, j;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Delete Expense\n" RESET);
    printDivider();
    printf(WHITE "  Enter ID to delete : " RESET YELLOW);
    if (scanf("%d", &id) != 1) {
        flushInput();
        printf(RESET RED "\n  Invalid ID.\n\n" RESET);
        return;
    }
    flushInput();

    for (i = 0; i < count; i++) {
        if (expenses[i].id == id) {
            printf(RESET CYAN "\n  Deleting: [%s] %.2f\n" RESET,
                   expenses[i].category, expenses[i].amount);

            remainingBudget += expenses[i].amount;

            /* Shift remaining records left */
            for (j = i; j < count - 1; j++) {
                expenses[j] = expenses[j + 1];
            }
            count--;
            saveData();

            printf(GREEN "  Expense deleted successfully!\n" RESET);
            printf(WHITE "  Remaining Budget : " YELLOW "%.2f\n\n" RESET, remainingBudget);
            return;
        }
    }

    printf(RED "\n  Expense with ID %d not found.\n\n" RESET, id);
}

/* ── 8. Delete All Expenses ─────────────────────────────────── */
void deleteAllExpenses() {
    char confirm[10];

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Delete ALL Expenses\n" RESET);
    printDivider();
    printf(RED "  WARNING: This will delete all %d expense(s) and clear the budget!\n" RESET, count);
    printf(WHITE "  Type 'yes' to confirm: " RESET YELLOW);
    if (fgets(confirm, sizeof(confirm), stdin) == NULL) {
        printf(RESET RED "\n  Invalid input.\n\n" RESET);
        return;
    }
    confirm[strcspn(confirm, "\n")] = '\0';
    printf(RESET);

    if (strcmp(confirm, "yes") == 0) {
        count           = 0;
        budget          = 0;
        remainingBudget = 0;
        saveData();
        printf(GREEN "\n  All expenses deleted and budget cleared.\n\n" RESET);
    } else {
        printf(YELLOW "\n  Operation cancelled.\n\n" RESET);
    }
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main() {
    int choice;

    loadData(); /* Restore previous session if save file exists */

    do {
        clearScreen();

        /* ── Header ── */
        printf(CYAN  "+==========================================+\n" RESET);
        printf(CYAN  "|" YELLOW "      Personal Expense Tracker           " CYAN "|" RESET "\n");
        printf(CYAN  "+==========================================+\n" RESET);

        /* ── Live Budget Display ── */
        printf(WHITE "\n  Budget    : " YELLOW "%.2f\n" RESET, budget);
        printf(WHITE "  Remaining : ");
        if (remainingBudget < budget * 0.2f && budget > 0)
            printf(RED   "%.2f  (!)\n" RESET, remainingBudget);
        else
            printf(YELLOW "%.2f\n" RESET, remainingBudget);
        printf(WHITE "  Expenses  : " YELLOW "%d record(s)\n\n" RESET, count);

        /* ── Menu ── */
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
            printf(RESET RED "\n  Invalid input. Enter a number from 1 to 9.\n" RESET);
            pressEnter();
            continue;
        }
        flushInput();
        printf(RESET);

        /* ── Dispatch ── */
        switch (choice) {
            case 1:  setBudget();        break;
            case 2:  addExpense();       break;
            case 3:  viewExpenses();     break;
            case 4:  totalExpenses();    break;
            case 5:  searchExpense();    break;
            case 6:  editExpense();      break;
            case 7:  deleteExpense();    break;
            case 8:  deleteAllExpenses(); break;
            case 9:
                printf(GREEN "\n  Goodbye! Stay on budget.\n\n" RESET);
                break;
            default:
                printf(RED "\n  Invalid choice. Enter a number from 1 to 9.\n" RESET);
        }

        if (choice != 9) pressEnter();

    } while (choice != 9);

    return 0;
}