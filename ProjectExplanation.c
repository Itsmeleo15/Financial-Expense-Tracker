#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX      100
#define FILENAME "expenses.csv"
#define USER_FILE "users.txt"
#define RED    "\033[1;31m"
#define GREEN  "\033[1;32m"
#define CYAN   "\033[0;36m"
#define WHITE  "\033[1;37m"
#define YELLOW "\033[1;33m"
#define RESET  "\033[0m"

/* Expense holds one record: a numeric ID, a category name, and how much was spent */
struct Expense {
    int   id;
    char  category[50];
    float amount;
};

/* Global array that stores all expense records, plus counters and budget totals */
struct Expense expenses[MAX];
int   count           = 0;
float budget          = 0;
float remainingBudget = 0;

/* 2D array that keeps the last 5 action messages the user triggered */
char history[5][100];
int  historyCount = 0;

/* Discards leftover characters in stdin so the next read starts clean */
void flushInput()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/* Clears the terminal window so the menu always appears at the top */
void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Pauses the program until the user presses Enter */
void pressEnter()
{
    printf(WHITE "\n Press Enter to continue..." RESET);
    flushInput();
}

/* Prints a horizontal line to visually separate sections on screen */
void printDivider()
{
    printf(CYAN " ------------------------------------------------\n" RESET);
}

/* Reads a line from the user and returns 1 only if every character is a digit.
   Rejects letters, symbols, and empty input so IDs are always clean numbers. */
int readInt(int *out)
{
    char buf[50];
    int  i;

    if (fgets(buf, sizeof(buf), stdin) == NULL)
        return 0;

    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) == 0)
        return 0;

    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] < '0' || buf[i] > '9')
            return 0;
    }

    *out = atoi(buf);
    return 1;
}

/* Reads a line and accepts only digits and one optional decimal point.
   Returns 1 on success so the caller knows the value is a valid number. */
int readFloat(float *out)
{
    char buf[50];
    int  i, dots = 0;

    if (fgets(buf, sizeof(buf), stdin) == NULL)
        return 0;

    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) == 0)
        return 0;

    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == '.') {
            dots++;
            if (dots > 1) return 0;  /* two decimal points means invalid input */
            continue;
        }
        if (buf[i] < '0' || buf[i] > '9')
            return 0;
    }

    *out = (float)atof(buf);
    return 1;
}

/* Scans a string character by character and returns 1 if any digit is found.
   Used to block numbers from being typed into a category name field. */
int hasDigit(const char *s)
{
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (s[i] >= '0' && s[i] <= '9')
            return 1;
    }
    return 0;
}

/* Appends a message to the rolling history log.
   When the log is full it shifts all entries left by one, dropping the oldest,
   then writes the new message at the last slot. */
void logAction(char (*log)[100], int *logCount, const char *msg)
{
    int i, j;

    if (*logCount >= 5) {
        for (i = 0; i < 4; i++)
            for (j = 0; j < 100; j++)
                log[i][j] = log[i + 1][j];  /* move each row one position earlier */
        *logCount = 4;
    }

    strncpy(log[*logCount], msg, 99);
    log[*logCount][99] = '\0';   /* guarantee null terminator even if msg is long */
    (*logCount)++;
}

/* Loops through all stored expenses and returns 1 if the given ID is already taken */
int idExists(int id)
{
    int i;
    for (i = 0; i < count; i++) {
        if (expenses[i].id == id)
            return 1;
    }
    return 0;
}

/* Writes the current budget and all expense records to the CSV file.
   The first line stores budget metadata; every following line is one expense. */
void saveData()
{
    FILE *fp;
    int   i;

    fp = fopen(FILENAME, "w");
    if (fp == NULL) {
        printf(RED "\n  Warning: Could not save data to file.\n" RESET);
        return;
    }

    fprintf(fp, "BUDGET,%.2f,%.2f,%d\n", budget, remainingBudget, count);

    for (i = 0; i < count; i++)
        fprintf(fp, "%d,%s,%.2f\n",
                expenses[i].id,
                expenses[i].category,
                expenses[i].amount);

    fclose(fp);
}

/* Reads the CSV file and restores budget and expense data from the last session.
   Silently returns if the file does not exist yet (first run). */
void loadData()
{
    FILE *fp;
    char  line[200];
    char *tok;
    int   i;

    fp = fopen(FILENAME, "r");
    if (fp == NULL)
        return;

    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return;
    }

    /* parse the header line; abort if it does not match the expected format */
    if (sscanf(line, "BUDGET,%f,%f,%d", &budget, &remainingBudget, &count) != 3) {
        budget = remainingBudget = 0;
        count  = 0;
        fclose(fp);
        return;
    }

    /* sanity-check the count before using it as a loop bound */
    if (count < 0 || count > MAX) {
        count = 0;
        fclose(fp);
        return;
    }

    for (i = 0; i < count; i++) {
        if (fgets(line, sizeof(line), fp) == NULL) {
            count = i;  /* file ended early, keep only what was read */
            break;
        }
        line[strcspn(line, "\n")] = '\0';

        /* split each CSV line by comma to extract id, category, and amount */
        tok = strtok(line, ",");
        if (tok) expenses[i].id = atoi(tok);

        tok = strtok(NULL, ",");
        if (tok) {
            strncpy(expenses[i].category, tok, sizeof(expenses[i].category) - 1);
            expenses[i].category[sizeof(expenses[i].category) - 1] = '\0';
        }

        tok = strtok(NULL, ",");
        if (tok) expenses[i].amount = (float)atof(tok);
    }

    fclose(fp);
}

/* Recursive login function. Asks for username and password, checks them against
   users.txt, and calls itself with one fewer attempt on failure.
   Stops recursing and exits when attemptsLeft reaches zero. */
int loginAttempt(int attemptsLeft)
{
    FILE *fp;
    char  username[50], password[50];
    char  line[110];
    char  storedUser[50], storedPass[50];

    if (attemptsLeft <= 0) {
        printf(RED "\n  Too many failed attempts. Exiting.\n\n" RESET);
        return 0;
    }

    printf(WHITE "  Username: " RESET YELLOW);
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf(RESET);
        return 0;
    }
    username[strcspn(username, "\n")] = '\0';
    printf(RESET);

    printf(WHITE "  Password: " RESET YELLOW);
    if (fgets(password, sizeof(password), stdin) == NULL) {
        printf(RESET);
        return 0;
    }
    password[strcspn(password, "\n")] = '\0';
    printf(RESET);

    fp = fopen(USER_FILE, "r");
    if (fp == NULL) {
        /* users.txt missing means first run, so create it with default credentials */
        fp = fopen(USER_FILE, "w");
        if (fp) {
            fprintf(fp, "admin,admin123\n");
            fclose(fp);
        }
        fp = fopen(USER_FILE, "r");
    }

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = '\0';
            /* parse each line as "username,password" and compare with what was typed */
            if (sscanf(line, "%49[^,],%49s", storedUser, storedPass) == 2) {
                if (strcmp(username, storedUser) == 0 &&
                    strcmp(password, storedPass) == 0) {
                    fclose(fp);
                    printf(GREEN "\n  Login successful! Welcome, %s.\n\n" RESET, username);
                    return 1;
                }
            }
        }
        fclose(fp);
    }

    printf(RED "\n  Invalid credentials. %d attempt(s) left.\n\n" RESET,
           attemptsLeft - 1);

    return loginAttempt(attemptsLeft - 1);  /* recursive call with one fewer attempt */
}

/* Shows the login screen header and starts the authentication process with 3 tries */
int doLogin()
{
    clearScreen();
    printf(CYAN "+=======================================+\n" RESET);
    printf(CYAN "|" YELLOW "       Personal Expense Tracker       " CYAN "|" RESET "\n");
    printf(CYAN "+=======================================+\n" RESET);
    printf(YELLOW "\n  --- LOGIN ---\n\n" RESET);
    printf(WHITE "  Default: admin / admin123\n\n" RESET);

    return loginAttempt(3);
}

/* Sets the initial budget on first use, or adds more money to an existing budget.
   Blocks negative or zero values and saves the new total to the CSV file. */
void setBudget()
{
    float addAmount;
    char  log[100];

    if (budget <= 0) {
        /* no budget yet, ask the user to set one from scratch */
        printf(WHITE "  Enter Total Budget: " RESET YELLOW);
        if (!readFloat(&budget)) {
            printf(RESET RED "\n  Invalid input. Numbers only.\n\n" RESET);
            budget = 0;
            return;
        }
        if (budget <= 0) {
            printf(RESET RED "\n  Budget must be greater than zero.\n\n" RESET);
            budget = 0;
            return;
        }
        remainingBudget = budget;  /* at start, remaining equals the full budget */
        saveData();
        logAction(history, &historyCount, "Budget set.");
        printf(GREEN "\n  Budget set successfully!\n" RESET);
        printf(WHITE "  Current Budget : " YELLOW "%.2f\n\n" RESET, budget);
        return;
    }

    /* budget already exists, so only allow topping it up */
    printf(CYAN "\n  Current Budget   : " YELLOW "%.2f\n" RESET, budget);
    printf(CYAN "  Remaining Budget : " YELLOW "%.2f\n\n" RESET, remainingBudget);
    printf(WHITE "  Enter amount to ADD to budget: " RESET YELLOW);

    if (!readFloat(&addAmount)) {
        printf(RESET RED "\n  Invalid input. Numbers only.\n\n" RESET);
        return;
    }
    if (addAmount <= 0) {
        printf(RESET RED "\n  Amount must be greater than zero.\n\n" RESET);
        return;
    }

    budget          += addAmount;
    remainingBudget += addAmount;  /* both totals grow by the same amount */
    saveData();

    sprintf(log, "Added %.2f to budget.", addAmount);
    logAction(history, &historyCount, log);

    printf(GREEN "\n  Budget updated successfully!\n" RESET);
    printf(WHITE "  New Total Budget : " YELLOW "%.2f\n" RESET, budget);
    printf(WHITE "  Remaining Budget : " YELLOW "%.2f\n\n" RESET, remainingBudget);
}

/* Collects ID, category, and amount for a new expense, validates every field,
   deducts the amount from the remaining budget, then saves to the CSV. */
void addExpense()
{
    int  newId;
    char log[100];

    if (count >= MAX) {
        printf("\n" RED "  Expense list is full.\n" RESET "\n");
        return;
    }
    if (budget <= 0) {
        printf(RED "\n  Please set a budget first.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Add New Expense\n" RESET);
    printDivider();

    printf(WHITE "\n  Expense ID: " RESET YELLOW);
    if (!readInt(&newId)) {
        printf(RESET RED "\n  Invalid ID. Numbers only.\n\n" RESET);
        return;
    }
    if (newId <= 0) {
        printf(RESET RED "\n  ID must be a positive number.\n\n" RESET);
        return;
    }
    if (idExists(newId)) {
        /* prevent duplicate IDs so each expense can be found and edited uniquely */
        printf(RESET RED "\n  Expense with ID %d already exists.\n\n" RESET, newId);
        return;
    }

    expenses[count].id = newId;

    printf(RESET WHITE "  Category: " RESET YELLOW);
    if (fgets(expenses[count].category, sizeof(expenses[count].category), stdin) == NULL) {
        printf(RESET RED "\n  Invalid category.\n\n" RESET);
        return;
    }
    expenses[count].category[strcspn(expenses[count].category, "\n")] = '\0';

    if (strlen(expenses[count].category) == 0) {
        printf(RESET RED "\n  Category cannot be empty.\n\n" RESET);
        return;
    }
    if (hasDigit(expenses[count].category)) {
        printf(RESET RED "\n  Category must contain letters only, no numbers.\n\n" RESET);
        return;
    }

    printf(RESET WHITE "  Amount: " RESET YELLOW);
    if (!readFloat(&expenses[count].amount)) {
        printf(RESET RED "\n  Invalid amount. Numbers only.\n\n" RESET);
        return;
    }
    if (expenses[count].amount <= 0) {
        printf(RED "\n  Amount must be greater than zero.\n\n" RESET);
        return;
    }
    if (expenses[count].amount > remainingBudget) {
        /* block the expense if it would push spending past the available budget */
        printf(RED "\n  Not enough budget for this expense!\n\n" RESET);
        return;
    }

    remainingBudget -= expenses[count].amount;
    count++;
    saveData();

    sprintf(log, "Added expense ID %d.", expenses[count - 1].id);
    logAction(history, &historyCount, log);

    printf("\n" GREEN "  Expense added successfully!\n" RESET);
    printf("\n" WHITE "  Remaining budget: " YELLOW " %.2f\n\n" RESET, remainingBudget);
}

/* Prints all stored expenses in a formatted table with a total record count at the bottom */
void viewExpenses()
{
    int i;

    if (count == 0) {
        printf("\n" RED "\n  No Expenses recorded yet.\n\n" RESET "\n");
        return;
    }

    printf("\n" YELLOW "\n  ------------- EXPENSE LIST -------------\n" RESET "\n");
    printf(CYAN "  %-5s | %-20s   | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();

    for (i = 0; i < count; i++) {
        printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
               expenses[i].id,
               expenses[i].category,
               expenses[i].amount);
    }

    printDivider();
    printf(WHITE "  Total Expenses : " YELLOW "%d record(s)\n\n" RESET, count);
    printf("\n");
}

/* Calculates and displays total spent, remaining budget, percentage used,
   an ASCII progress bar that turns red when over 90%, and the action history log. */
void totalExpenses()
{
    float total   = 0.0f;
    float percent = 0.0f;
    int   i, b, filled, h;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    for (i = 0; i < count; i++)
        total += expenses[i].amount;

    if (budget > 0)
        percent = (total / budget) * 100.0f;  /* how much of the budget has been used */

    printf(YELLOW "\n  ----------- EXPENSE SUMMARY ------------\n\n" RESET);
    printf(WHITE "  Total Budget   : " YELLOW "%.2f\n"     RESET, budget);
    printf(WHITE "  Total Spent    : " YELLOW "%.2f\n"     RESET, total);
    printf(WHITE "  Remaining      : " YELLOW "%.2f\n"     RESET, remainingBudget);
    printf(WHITE "  Budget Used    : " YELLOW "%.1f%%\n\n" RESET, percent);

    /* each '#' block represents 5% of the budget; bar is 20 characters wide = 100% */
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

    /* show up to 5 recent actions stored in the history 2D array */
    if (historyCount > 0) {
        printf(WHITE "  --- Recent Actions ---\n" RESET);
        for (h = 0; h < historyCount; h++)
            printf(CYAN "  [%d] %s\n" RESET, h + 1, history[h]);
        printf("\n");
    }
}

/* Asks for a category name then lists every expense that exactly matches it,
   along with a subtotal for that category. */
void searchExpense()
{
    char  search[50];
    float catTotal = 0.0f;
    int   i, found = 0;

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW "  Search by Category\n" RESET);
    printDivider();
    printf(WHITE "  Enter category: " RESET YELLOW);

    if (fgets(search, sizeof(search), stdin) == NULL) {
        printf(RESET RED "\n  Invalid input.\n\n" RESET);
        return;
    }
    search[strcspn(search, "\n")] = '\0';
    printf(RESET "\n\n");

    printf(CYAN " %-5s | %-22s | %10s\n" RESET, "ID", "Category", "Amount");
    printDivider();

    for (i = 0; i < count; i++) {
        if (strcmp(expenses[i].category, search) != 0)
            continue;  /* skip expenses that belong to a different category */
        printf(WHITE "  %-5d | %-22s | " YELLOW "%10.2f\n" RESET,
               expenses[i].id,
               expenses[i].category,
               expenses[i].amount);
        catTotal += expenses[i].amount;
        found = 1;
    }

    if (!found) {
        printf(RED "  No expenses found for category \"%s\".\n\n" RESET, search);
    } else {
        printDivider();
        printf(WHITE "  Category Total : " YELLOW "%.2f\n\n" RESET, catTotal);
    }
}

/* Finds an expense by ID, shows its current values, then lets the user replace
   the category and amount. Temporarily restores the old amount to the budget
   while editing so the budget check uses the correct available balance. */
void editExpense()
{
    int   id, i;
    float newAmount;
    char  newCategory[50];
    char  log[100];

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW "  Edit Expense\n" RESET);
    printDivider();
    printf(WHITE "  Enter ID to edit: " RESET YELLOW);

    if (!readInt(&id)) {
        printf(RESET RED "\n  Invalid ID. Numbers only.\n\n" RESET);
        return;
    }

    for (i = 0; i < count; i++) {
        if (expenses[i].id != id)
            continue;

        printf(RESET CYAN "\n  Current Category : " WHITE "%s\n"   RESET, expenses[i].category);
        printf(CYAN       "  Current Amount   : " WHITE "%.2f\n\n" RESET, expenses[i].amount);

        /* give back the old amount so the new amount is checked against the real available balance */
        remainingBudget += expenses[i].amount;

        printf(WHITE "  New category    : " RESET YELLOW);
        if (fgets(newCategory, sizeof(newCategory), stdin) == NULL) {
            printf(RESET RED "\n  Invalid category. Edit cancelled.\n\n" RESET);
            remainingBudget -= expenses[i].amount;  /* undo the temporary restore on failure */
            return;
        }
        newCategory[strcspn(newCategory, "\n")] = '\0';

        if (strlen(newCategory) == 0) {
            printf(RESET RED "\n  Category cannot be empty. Edit cancelled.\n\n" RESET);
            remainingBudget -= expenses[i].amount;
            return;
        }
        if (hasDigit(newCategory)) {
            printf(RESET RED "\n  Category must contain letters only, no numbers. Edit cancelled.\n\n" RESET);
            remainingBudget -= expenses[i].amount;
            return;
        }

        printf(WHITE "  New amount    : " RESET YELLOW);
        if (!readFloat(&newAmount)) {
            printf(RESET RED "\n  Invalid amount. Numbers only. Edit cancelled.\n\n" RESET);
            remainingBudget -= expenses[i].amount;
            return;
        }
        if (newAmount <= 0) {
            printf(RED "\n  Amount must be greater than zero. Edit cancelled.\n\n" RESET);
            remainingBudget -= expenses[i].amount;
            return;
        }
        if (newAmount > remainingBudget) {
            printf(RED "  Not enough budget for this amount!\n\n" RESET);
            remainingBudget -= expenses[i].amount;
            return;
        }

        /* apply the new values and deduct the new amount from the budget */
        strncpy(expenses[i].category, newCategory, sizeof(expenses[i].category) - 1);
        expenses[i].category[sizeof(expenses[i].category) - 1] = '\0';
        expenses[i].amount = newAmount;
        remainingBudget   -= newAmount;
        saveData();

        sprintf(log, "Edited expense ID %d.", id);
        logAction(history, &historyCount, log);

        printf("\n" GREEN "  Expense updated successfully!\n" RESET);
        printf("\n" WHITE "  Remaining budget: " YELLOW " %.2f\n\n" RESET, remainingBudget);
        return;
    }

    printf(RED "  Expense with ID %d not found.\n\n" RESET, id);
}

/* Finds an expense by ID, refunds its amount to the remaining budget,
   then shifts all later entries left by one to close the gap in the array. */
void deleteExpense()
{
    int  id, i, j;
    char log[100];

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf("\n");
    printf(YELLOW "  Delete Expense\n" RESET);
    printDivider();
    printf(WHITE "  Enter ID to delete: " RESET YELLOW);

    if (!readInt(&id)) {
        printf(RESET RED "\n  Invalid ID. Numbers only.\n\n" RESET);
        return;
    }

    for (i = 0; i < count; i++) {
        if (expenses[i].id != id)
            continue;

        printf(RESET CYAN "\n  Deleting: [%s] %.2f\n" RESET,
               expenses[i].category, expenses[i].amount);

        remainingBudget += expenses[i].amount;  /* put the money back */

        /* shift every entry after the deleted one left by one position */
        for (j = i; j < count - 1; j++)
            expenses[j] = expenses[j + 1];

        count--;
        saveData();

        sprintf(log, "Deleted expense ID %d.", id);
        logAction(history, &historyCount, log);

        printf(GREEN "\n  Expense deleted successfully!\n" RESET);
        printf(WHITE "  Remaining budget: " YELLOW "%.2f\n\n" RESET, remainingBudget);
        return;
    }

    printf(RED "\n  Expense with ID %d not found.\n\n" RESET, id);
}

/* Asks for confirmation then wipes all expense records and resets the history log.
   The budget total is kept but the remaining balance is restored to the full amount. */
void deleteAllExpenses()
{
    char confirm[10];

    if (count == 0) {
        printf(RED "\n  No expenses recorded yet.\n\n" RESET);
        return;
    }

    printf(YELLOW "\n  Delete All Expenses\n" RESET);
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
        remainingBudget = budget;  /* spending is zero again so remaining equals the full budget */
        historyCount    = 0;
        saveData();
        logAction(history, &historyCount, "All expenses deleted.");
        printf(GREEN "\n  All expenses deleted. Budget restored to %.2f\n\n" RESET, budget);
    } else {
        printf(YELLOW "\n  Operation cancelled.\n\n" RESET);
    }
}

/* Entry point. Runs the login check, loads saved data, then loops the main menu
   until the user selects Exit (option 9). */
int main()
{
    int choice;

    if (!doLogin())
        return 1;

    loadData();

    do {
        clearScreen();

        printf(CYAN "+=======================================+\n" RESET);
        printf(CYAN "|" YELLOW "       Personal Expense Tracker       " CYAN " |" RESET "\n");
        printf(CYAN "+=======================================+\n" RESET);

        printf(WHITE "\n  Budget    : " YELLOW "%.2f\n" RESET, budget);
        printf(WHITE "  Remaining : ");
        if (remainingBudget < budget * 0.2f && budget > 0)
            printf(RED "%.2f  (!)\n" RESET, remainingBudget);  /* warn when under 20% left */
        else
            printf(YELLOW "%.2f\n" RESET, remainingBudget);
        printf(WHITE "  Expenses  : " YELLOW "%d record(s)\n\n" RESET, count);

        printf(WHITE " 1." CYAN " Set / Add Budget\n"    RESET);
        printf(WHITE " 2." CYAN " Add Expense\n"         RESET);
        printf(WHITE " 3." CYAN " View Expenses\n"       RESET);
        printf(WHITE " 4." CYAN " Expense Summary\n"     RESET);
        printf(WHITE " 5." CYAN " Search by Category\n"  RESET);
        printf(WHITE " 6." CYAN " Edit Expense\n"        RESET);
        printf(WHITE " 7." CYAN " Delete Expense\n"      RESET);
        printf(WHITE " 8." CYAN " Delete All\n"          RESET);
        printf(WHITE " 9." CYAN " Exit\n\n"              RESET);

        printf(WHITE "  Your Choice: " RESET YELLOW);
        if (scanf("%d", &choice) != 1) {
            /* non-numeric input: flush the bad characters and show an error */
            flushInput();
            printf(RESET RED "\n  Invalid input. Please enter a number from 1 to 9.\n\n" RESET);
            pressEnter();
            continue;
        }
        flushInput();
        printf(RESET);

        switch (choice) {
            case 1: setBudget();         break;
            case 2: addExpense();        break;
            case 3: viewExpenses();      break;
            case 4: totalExpenses();     break;
            case 5: searchExpense();     break;
            case 6: editExpense();       break;
            case 7: deleteExpense();     break;
            case 8: deleteAllExpenses(); break;
            case 9:
                printf("\n" GREEN "  Goodbye! Stay on budget." RESET "\n");
                break;
            default:
                printf("\n" RED "  Invalid choice. Please enter a number from 1 to 9.\n" RESET "\n");
        }

        if (choice != 9)
            pressEnter();

    } while (choice != 9);

    return 0;
}
