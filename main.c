#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MASTER_FILE "todo_master.txt"
#define MAX_NAME_LEN 100
#define MAX_LINE_LEN 256


#define RED     "\x1b[31m"
#define ORANGE  "\x1b[38;5;208m"
#define YELLOW  "\x1b[33m"
#define GREEN   "\x1b[32m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"
#define CYAN    "\x1b[36m"
#define MAGENTA "\x1b[35m"


void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


void pauseForUser() {
    printf("\nPress Enter to continue...");
    getchar();
}


void stripNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}


void createNewToDoList() {
    FILE *master = fopen(MASTER_FILE, "a+");
    if (!master) {
        printf(RED "Failed to open master file.\n" RESET);
        return;
    }

    char name[MAX_NAME_LEN];
    printf("Enter name for new to-do list: ");
    fgets(name, sizeof(name), stdin);
    stripNewline(name);


    int id = 1;
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), master)) {
        int tempId;
        char tempName[MAX_NAME_LEN];
        sscanf(line, "%d | %[^\n]", &tempId, tempName);
        if (tempId >= id) id = tempId + 1;
    }

    fprintf(master, "%d | %s\n", id, name);
    fclose(master);

    char filename[128];
    sprintf(filename, "todo_%d.txt", id);
    FILE *listFile = fopen(filename, "w");
    if (listFile) fclose(listFile);

    printf(GREEN "To-do list '%s' created with ID %d.\n" RESET, name, id);
}


int getListIdByName(const char *searchName) {
    FILE *master = fopen(MASTER_FILE, "r");
    if (!master) return -1;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), master)) {
        int id;
        char listName[MAX_NAME_LEN];
        sscanf(line, "%d | %[^\n]", &id, listName);
        if (strcmp(listName, searchName) == 0) {
            fclose(master);
            return id;
        }
    }
    fclose(master);
    return -1;
}


void addToDoItem() {
    char name[MAX_NAME_LEN];
    printf("Enter name of the list to add to: ");
    fgets(name, sizeof(name), stdin);
    stripNewline(name);

    int id = getListIdByName(name);
    if (id == -1) {
        printf(RED "List not found.\n" RESET);
        return;
    }

    char item[MAX_LINE_LEN];
    printf("Enter new to-do item: ");
    fgets(item, sizeof(item), stdin);
    stripNewline(item);

    char filename[128];
    sprintf(filename, "todo_%d.txt", id);
    FILE *file = fopen(filename, "a");
    if (!file) {
        printf(RED "Failed to open list file.\n" RESET);
        return;
    }

    fprintf(file, "[ ] %s\n", item);
    fclose(file);
    printf(GREEN "Item added to list '%s'.\n" RESET, name);
}


void viewAllToDoLists() {
    FILE *master = fopen(MASTER_FILE, "r");
    if (!master) {
        printf(RED "No to-do lists found.\n" RESET);
        return;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), master)) {
        int id;
        char listName[MAX_NAME_LEN];
        sscanf(line, "%d | %[^\n]", &id, listName);
        printf("\n" CYAN "List: %s (ID: %d)" RESET "\n", listName, id);

        char filename[128];
        sprintf(filename, "todo_%d.txt", id);
        FILE *list = fopen(filename, "r");
        if (!list) continue;

        char item[MAX_LINE_LEN];
        int itemNum = 1;
        while (fgets(item, sizeof(item), list)) {
            printf("  %d. %s", itemNum++, item);
        }
        fclose(list);
    }
    fclose(master);
}


void markItemAsCompleted() {
    char name[MAX_NAME_LEN];
    printf("Enter name of the list to update: ");
    fgets(name, sizeof(name), stdin);
    stripNewline(name);

    int id = getListIdByName(name);
    if (id == -1) {
        printf(RED "List not found.\n" RESET);
        return;
    }

    char filename[128];
    sprintf(filename, "todo_%d.txt", id);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf(RED "Failed to open list file.\n" RESET);
        return;
    }

    char items[100][MAX_LINE_LEN];
    int count = 0;
    while (fgets(items[count], sizeof(items[count]), file)) {
        count++;
    }
    fclose(file);

    for (int i = 0; i < count; i++) {
        printf("%d. %s", i + 1, items[i]);
    }

    int choice;
    printf("Enter number of item to mark as done: ");
    scanf("%d", &choice);
    getchar();  // clear newline

    if (choice < 1 || choice > count) {
        printf(RED "Invalid choice.\n" RESET);
        return;
    }

    if (strncmp(items[choice - 1], "[ ]", 3) == 0) {
        items[choice - 1][1] = 'X';
    }

    file = fopen(filename, "w");
    for (int i = 0; i < count; i++) {
        fputs(items[i], file);
    }
    fclose(file);

    int allDone = 1;
    for (int i = 0; i < count; i++) {
        if (strncmp(items[i], "[ ]", 3) == 0) {
            allDone = 0;
            break;
        }
    }

    if (allDone) {
        remove(filename);

        FILE *master = fopen(MASTER_FILE, "r");
        FILE *temp = fopen("temp.txt", "w");
        char line[MAX_LINE_LEN];
        while (fgets(line, sizeof(line), master)) {
            int currId;
            char listName[MAX_NAME_LEN];
            sscanf(line, "%d | %[^\n]", &currId, listName);
            if (currId != id) {
                fprintf(temp, "%s", line);
            }
        }
        fclose(master);
        fclose(temp);
        remove(MASTER_FILE);
        rename("temp.txt", MASTER_FILE);

        printf(GREEN "All items completed. List '%s' has been deleted.\n" RESET, name);
    } else {
        printf(GREEN "Item marked as completed.\n" RESET);
    }
}


void printMenu() {
    printf(MAGENTA "===== To-Do Application =====\n" RESET);
    printf(RED "1. Create New To-Do List\n" RESET);
    printf(ORANGE "2. Add New To-Do Item\n" RESET);
    printf(YELLOW "3. View All To-Do Lists\n" RESET);
    printf(GREEN "4. Mark Item as Completed\n" RESET);
    printf(CYAN "5. Exit\n" RESET);
    printf("Choose an option: ");
}


int main() {
    while (1) {
        printMenu();
        int choice;
        scanf("%d", &choice);
        getchar(); // Clear newline

        clearScreen();  // Clear before action

        switch (choice) {
            case 1:
                createNewToDoList();
                pauseForUser();
                break;
            case 2:
                addToDoItem();
                pauseForUser();
                break;
            case 3:
                viewAllToDoLists();
                pauseForUser();
                break;
            case 4:
                markItemAsCompleted();
                pauseForUser();
                break;
            case 5:
                printf(GREEN "Goodbye!\n" RESET);
                return 0;
            default:
                printf(RED "Invalid choice.\n" RESET);
                pauseForUser();
        }

        clearScreen();
    }
    return 0;
}
