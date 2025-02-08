#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_PARKING 5
#define MAX_VEHICLES 100

// Colors :
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

enum vehicle_type
{
    motorcycles = 1,
    sedans = 2,
    vans = 4
};

typedef struct
{
    char license_Plate[7];
    char model[20];
    int Time_of_use;
    int Checked; // 0 :means it's not in the parking , 1 :means it's in the parking.
    char Parking_space[6];
    enum vehicle_type type;
} Vehicle;

void find_EmptySpaces(char filename[50], int floor, char emptySpaces[MAX_PARKING][6], int *count_empty)
{
    *count_empty = 0;
    int occupied[MAX_PARKING] = {0};

    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("Error opening file\n");
        return;
    }

    Vehicle temp;
    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (temp.Parking_space[1] - '0' == floor && temp.Checked == 1)
        {
            int space_Num = temp.Parking_space[4] - '0';
            if (space_Num >= 1 && space_Num <= MAX_PARKING)
            {
                occupied[space_Num - 1] = 1;
            }
        }
    }

    fclose(file);

    for (int i = 0; i < MAX_PARKING; i++)
    {
        if (occupied[i] == 0)
        {
            sprintf(emptySpaces[*count_empty], "f%d_0%d", floor, i + 1);
            (*count_empty)++;
        }
    }
}

void add_vehicle(char filename[50], Vehicle *CAR)
{
    srand(time(NULL));
    FILE *file = fopen(filename, "a+");

    if (!file)
    {
        printf("Error opening file.\n");
        return;
    }
    Vehicle temp;
    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (strcmp(temp.license_Plate, CAR->license_Plate) == 0 && temp.Checked == 1)
        {
            printf(RED "This license plate has already been entered.\n" RESET);
            fclose(file);
            return;
        }
    }

    if (CAR->Checked == 0)
    {
        char emptySpaces[MAX_PARKING][6];
        int count_empty = 0;
        int floor;
        int floors_to_check[2];

        switch (CAR->type)
        {
        case motorcycles:
            floor = 1;
            floors_to_check[0] = 1;
            break;
        case sedans:
            if (rand() % 2 == 0)
            {
                floors_to_check[0] = 2;
                floors_to_check[1] = 3;
            }
            else
            {
                floors_to_check[0] = 3;
                floors_to_check[1] = 2;
            }
            break;
        case vans:
            floor = 4;
            floors_to_check[0] = 4;
            break;
        default:
            printf(RED BOLD "There is no such vehicle type.\n" RESET);
            fclose(file);
            return;
        }

        for (int i = 0; i < (CAR->type == sedans ? 2 : 1); i++)
        {
            floor = floors_to_check[i];
            find_EmptySpaces(filename, floor, emptySpaces, &count_empty);
            if (count_empty > 0)
            {
                CAR->Checked = 1;
                int space = rand() % count_empty;
                strcpy(CAR->Parking_space, emptySpaces[space]);
                fprintf(file, "%s,%s,%d,%d,%s,%d\n", CAR->license_Plate, CAR->model, CAR->Time_of_use, CAR->Checked, CAR->Parking_space, CAR->type);
                break;
            }
        }

        if (CAR->Checked == 0)
        {
            printf(RED BOLD "Parking capacity is full for this vehicle type in the assigned floors.\n" RESET);
        }
    }

    fclose(file);

    if (CAR->Checked == 1)
    {
        printf(YELLOW "|---Parking entrance receipt---|\n" RESET);
        printf(GREEN "Car space in the parking lot: %s\n", CAR->Parking_space);
        printf("License Plate: %s\n", CAR->license_Plate);
        printf("Time of use: %d\n", CAR->Time_of_use);
        printf("Status: %s\n", CAR->Checked == 1 ? "check_in" : "check_out" RESET);
        Sleep(5000);
    }
}

void remove_vehicle(char filename[50], char license_plate[7])
{
    FILE *file = fopen(filename, "r");
    FILE *temp_file = fopen("temp.csv", "w");

    if (!file || !temp_file)
    {
        printf(RED "Error opening file\n" RESET);
        return;
    }

    Vehicle temp;
    int found = 0;
    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (strcmp(temp.license_Plate, license_plate) == 0)
        {
            found = 1;
            if (temp.Checked == 1)
            {
                temp.Checked = 0;
                fprintf(temp_file, "%s,%s,%d,%d,%s,%d\n", temp.license_Plate, temp.model, temp.Time_of_use, temp.Checked, temp.Parking_space, temp.type);
                printf(CYAN "Vehicle with license plate %s removed successfully.\n" RESET, license_plate);
            }
            else
                printf(RED "Vehicle with license plate %s is not currently in the parking lot.\n" RESET, license_plate);
        }
        else
        {
            fprintf(temp_file, "%s,%s,%d,%d,%s,%d\n", temp.license_Plate, temp.model, temp.Time_of_use, temp.Checked, temp.Parking_space, temp.type);
        }
    }
    fclose(file);
    fclose(temp_file);
    if (found)
    {
        remove(filename);
        rename("temp.csv", filename);
    }
    else
    {
        printf(RED "No vehicle with this license plate was found!\n" RESET);
        remove("temp.csv");
    }
}
void search_vehicle(char filename[50], char license_plate[7])
{
    Vehicle temp;
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("Error opening file\n");
        return;
    }
    int found = 0;
    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (strcmp(temp.license_Plate, license_plate) == 0)
        {
            found = 1;
            printf(CYAN "\n\t|----------------------------------|\n\n" RESET);
            printf(MAGENTA "\tlicense plate :%s\n", temp.license_Plate);
            printf("\tmodel :%s\n", temp.model);
            printf("\tCar space in the parking lot :%s\n", temp.Parking_space);
            printf("\tTime of use :%d\n", temp.Time_of_use);
            printf("\tStatus: %s\n", temp.Checked == 1 ? "Check_in" : "Check_out" RESET);
            switch (temp.type)
            {
                case 1:
                {
                    printf(MAGENTA "\tType : motorcycles\n" RESET);
                    break;
                }
                case 2:
                {
                    printf(MAGENTA "\tType : sedans\n" RESET);
                    break;
                }
                case 3:
                {
                    printf(MAGENTA "\tType : vans\n" RESET);
                    break;
                }
            }
            printf(CYAN "\n\t|----------------------------------|\n" RESET);
        }
    }
    if (!found)
    {
        printf(RED "\t\t*---No vehicle with this license plate was found.---*\n" RESET);
    }
    Sleep(5000);
}

void Bubble_sort(Vehicle temp[MAX_VEHICLES], int count)
{
    int swapped;
    for (int i = 0; i < count - 1; i++)
    {
        swapped = 0;
        for (int j = 0; j < count - i - 1; j++)
        {
            if (strcmp(temp[j].Parking_space, temp[j + 1].Parking_space) > 0)
            {
                Vehicle a = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = a;
                swapped = 1;
            }
        }
    }
    if (!swapped)
    {
        return;
    }
}
void History_list(char filename[50])
{
    Vehicle temp[MAX_VEHICLES];
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf(RESET "Error: Cannot open file %s\n" RESET, filename);
        return;
    }
    int count = 0;

    printf(CYAN "\n=========== PARKING HISTORY ===========\n" RESET);
    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp[count].license_Plate, temp[count].model, &temp[count].Time_of_use, &temp[count].Checked, temp[count].Parking_space, &temp[count].type) == 6)
    {
        count++;
    }
    fclose(file);

    Bubble_sort(temp, count);

    printf(YELLOW "\n--- History List (Sorted by Parking Space) ---\n" RESET);
    for (int i = 0; i < count; i++)
    {
        printf(MAGENTA "License Plate: %s\t | Model: %s\t | Time of Use: %d\t | Status: %s\t | Parking Space: %s\n" RESET,
            temp[i].license_Plate, temp[i].model, temp[i].Time_of_use,
            temp[i].Checked == 1 ? "Check_in" : "Check_out", temp[i].Parking_space);
    }
    Sleep(5000);
}
void display_type(char filename[50], int type)
{
    FILE *file = fopen(filename, "r");

    Vehicle temp;
    int found = 0;
    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (temp.type == type)
        {
            found = 1;
            printf(MAGENTA "License Plate: %s\t | Model: %s\t | Time of Use: %d\t | Status: %s\t | Parking Space: %s\n" RESET,
                temp.license_Plate, temp.model, temp.Time_of_use,
                temp.Checked == 1 ? "Check_in" : "Check_out", temp.Parking_space);
        }
    }
    if (!found)
    {
        printf(RED "No vehicle found." RESET);
    }
    fclose(file);
    Sleep(5000);
}

void display_floor(char filename[50], int floor)
{
    FILE *file = fopen(filename, "r");

    Vehicle temp;
    int found = 0;

    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (temp.Parking_space[1] - '0' == floor && temp.Checked == 1)
        {
            found = 1;
            printf(MAGENTA "License Plate: %s\t | Model: %s\t | Time of Use: %d\t | Parking Space: %s\n" RESET,
                temp.license_Plate, temp.model, temp.Time_of_use, temp.Parking_space);
        }
    }

    if (!found)
    {
        printf(RED "No vehicle found." RESET);
    }
    fclose(file);
    Sleep(5000);
}

dispaly_model(char filename[50], char model[20])
{
    FILE *file = fopen(filename, "r");

    Vehicle temp;
    int found = 0;

    while (fscanf(file, "%[^,],%[^,],%d,%d,%[^,],%d\n", temp.license_Plate, temp.model, &temp.Time_of_use, &temp.Checked, temp.Parking_space, &temp.type) == 6)
    {
        if (strcmp(temp.model , model))
        {
            found = 1;
            printf(MAGENTA "License Plate: %s\t | Model: %s\t | Time of Use: %d\t | Parking Space: %s\n" RESET,
                temp.license_Plate, temp.model, temp.Time_of_use, temp.Parking_space);
        }
    }

    if(!found)
    {
        printf(RED "No vehicle found." RESET);
    }
    fclose(file);
    Sleep(5000);
}

display_parkingSpace_of_perFloors(char filename, int space);

int login()
{
    char UserName[MAX_USERNAME], Password[MAX_PASSWORD];
    char file_UserName[MAX_USERNAME], file_Password[MAX_PASSWORD];

    FILE *file;

    printf(BLUE "|---Login---|\n" RESET);
    while (1)
    {
        printf(CYAN "Enter username:\n");
        scanf("%s", UserName);
        printf("Enter Password:\n" RESET);
        scanf("%s", Password);

        file = fopen("userfile.csv", "r");
        if (!file)
        {
            perror(RED "Error opening file\n" RESET);
            return 0;
        }

        int logged_in = 0;

        while (fscanf(file, "%[^,],%s\n", file_UserName, file_Password) != EOF)
        {
            if (strcmp(file_UserName, UserName) == 0 && strcmp(file_Password, Password) == 0)
            {
                logged_in = 1;
                break;
            }
        }
        fclose(file);

        if (logged_in)
        {
            printf(GREEN "  Login successfully\n" RESET);
            return 1;
        }
        else
        {
            printf(RED "The username or password is incorrect. Please try again.\n" RESET);
        }
    }
}

void Main_Menu()
{
    int choice;
    Vehicle car;
    char fileName[50] = "Parking_Managment_Console.csv";
    char emptySpaces[MAX_PARKING][6];
    int count_empty = 0;

    while (1)
    {
        printf(CYAN "|---------------------|\n" RESET);
        find_EmptySpaces(fileName, 1, emptySpaces, &count_empty);
        printf(GREEN "Floor 1: %d out of 5 empty\n", count_empty);
        find_EmptySpaces(fileName, 2, emptySpaces, &count_empty);
        printf("Floor 2: %d out of 5 empty\n", count_empty);
        find_EmptySpaces(fileName, 3, emptySpaces, &count_empty);
        printf("Floor 3: %d out of 5 empty\n", count_empty);
        find_EmptySpaces(fileName, 4, emptySpaces, &count_empty);
        printf("Floor 4: %d out of 5 empty\n" RESET, count_empty);
        printf(CYAN "--- Menu ---\n" RESET);
        printf(YELLOW "1. Add vehicle\n");
        printf("2. Remove vehicle\n");
        printf("3. Vehicle Search\n");
        printf("4. History List\n");
        printf("5. display type\n");
        printf("6. display floor\n");
        printf("7. Log out\n");
        printf("if you want to close application press (ctrl + c)\n");
        printf("Please enter your choice: " RESET);
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf(CYAN "Enter license plate:\n");
            scanf("%s", car.license_Plate);
            printf("Enter Model:\n");
            scanf("%s", car.model);
            printf("Enter Time of use:\n");
            scanf("%d", &car.Time_of_use);
            car.Checked = 0;
            printf("Enter type (1: motorcycles, 2: sedans, 4: vans):\n" RESET);
            scanf("%d", (int *)&car.type);
            add_vehicle(fileName, &car);
            break;
        case 2:
            printf(CYAN "Enter license plate:\n" RESET);
            scanf("%s", car.license_Plate);
            remove_vehicle(fileName, car.license_Plate);
            break;
        case 3:
            printf(CYAN "Enter license plate:\n" RESET);
            scanf("%s", car.license_Plate);
            search_vehicle(fileName, car.license_Plate);
            break;
        case 4:
            History_list(fileName);
            break;
        case 5:
            printf(CYAN "Enter type (1: motorcycles, 2: sedans, 4: vans):\n" RESET);
            int type1;
            scanf("%d", &type1);
            display_type(fileName, type1);
            break;
        case 6:
            printf(CYAN "Enter the floor:\n" RESET);
            int floor;
            scanf("%d", &floor);
            display_floor(fileName, floor);
            break;
        case 7:
            printf(GREEN "Logging out...\n" RESET);
            return;
        default:
            printf(RED "Invalid choice.\n" RESET);
        }
    }
}

int main()
{
    while (1)
    {
        if (login())
        {
            Main_Menu();
        }
    }
    return 0;
}

