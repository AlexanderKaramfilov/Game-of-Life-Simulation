#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


#ifdef _WIN32
    #include <windows.h>
    void sleep_ms(int milliseconds) { Sleep(milliseconds); }
#else
    #include <unistd.h>
    void sleep_ms(int milliseconds) { usleep(milliseconds * 1000); }
#endif

// --- КОНСТАНТИ ---
#define ROWS 24      // Височина на полето (редове)
#define COLS 80      // Ширина на полето (колони)
#define ALIVE 'O'    // Символ за жива клетка
#define DEAD  '.'    // Символ за мъртва клетка

// --- ГЛОБАЛНИ ПРОМЕНЛИВИ ---
// Два масива за правилата: индексът отговаря на броя съседи (0-8)
// Ако rule_born[3] == 1, 
bool rule_born[9] = {0};
bool rule_survive[9] = {0};

// Двете решетки (текуща и следваща)
int current_grid[ROWS][COLS];
int next_grid[ROWS][COLS];

// --- ПОМОЩНИ ФУНКЦИИ ---

// Изчистване на екрана чрез ANSI кодове (работи по-гладко от system("cls"))
void clear_screen() {
    printf("\033[H\033[J");
}

// Инициализиране на правилата (Нулиране)
void reset_rules() {
    for (int i = 0; i < 9; i++) {
        rule_born[i] = false;
        rule_survive[i] = false;
    }
}

// Настройка на стандартните правила на Conway (B3/S23)
void set_conway_rules() {
    reset_rules();
    rule_born[3] = true;       
    rule_survive[2] = true;   
    rule_survive[3] = true; 
    printf(">> Rules set to Standard Conway (B3/S23)\n");
}

// Парсване на потребителски правила от низ (напр. "36" за раждане)
void parse_custom_rule(char* input, bool* rule_array) {
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] >= '0' && input[i] <= '8') {
            int neighbor_count = input[i] - '0'; // Преобразуване от char към int
            rule_array[neighbor_count] = true;
        }
    }
}

// --- ЛОГИКА НА ИГРАТА ---

// Инициализация със случайни стойности
void init_random() {
    srand(time(NULL));
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // Около 20% шанс клетката да е жива в началото
            current_grid[i][j] = (rand() % 5 == 0) ? 1 : 0; 
        }
    }
}

// Инициализация с фигура "Glider" (полезно за тест)
void init_glider() {
    // Изчистване на всичко
    for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) current_grid[i][j] = 0;
    
    // Координати за Glider в горния ляв ъгъл
    current_grid[1][2] = 1;
    current_grid[2][3] = 1;
    current_grid[3][1] = 1;
    current_grid[3][2] = 1;
    current_grid[3][3] = 1;
}

// Броене на съседите (с Wrap-around/Тороид логика)
int count_neighbors(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue; // Пропускаме самата клетка

            // Математика за "превъртане" на координатите
            int nr = (r + i + ROWS) % ROWS;
            int nc = (c + j + COLS) % COLS;

            count += current_grid[nr][nc];
        }
    }
    return count;
}

// Изчисляване на следващото поколение
void compute_next_generation() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int neighbors = count_neighbors(i, j);
            int is_alive = current_grid[i][j];

            if (is_alive) {
                // Ако е жива, проверяваме правилата за оцеляване (Survive)
                next_grid[i][j] = rule_survive[neighbors] ? 1 : 0;
            } else {
                // Ако е мъртва, проверяваме правилата за раждане (Born)
                next_grid[i][j] = rule_born[neighbors] ? 1 : 0;
            }
        }
    }

    // Копиране на next_grid в current_grid
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            current_grid[i][j] = next_grid[i][j];
        }
    }
}

// Визуализация
void draw_grid(int generation) {
    clear_screen();
    printf("GENERATION: %d  (Press Ctrl+C to stop)\n", generation);
    
    // Горна рамка
    for(int k=0; k<COLS+2; k++) printf("-");
    printf("\n");

    for (int i = 0; i < ROWS; i++) {
        printf("|"); // Лява рамка
        for (int j = 0; j < COLS; j++) {
            if (current_grid[i][j]) {
                printf("%c", ALIVE);
            } else {
                printf("%c", DEAD);
            }
        }
        printf("|"); // Дясна рамка
        printf("\n");
    }

    // Долна рамка
    for(int k=0; k<COLS+2; k++) printf("-");
    printf("\n");
}

// --- MAIN FUNCTION ---
int main() {
    char input_buffer[20];
    int choice;

    clear_screen();
    printf("=== GAME OF LIFE SIMULATION ===\n");
    printf("1. Standard Rules (Conway B3/S23)\n");
    printf("2. Custom Rules\n");
    printf("Select option (1-2): ");
    
    // Четене на избор и изчистване на буфера
    scanf("%d", &choice);
    while((getchar()) != '\n'); 

    if (choice == 2) {
        reset_rules();
        printf("\n--- Custom Rules Setup ---\n");
        printf("Enter neighbors count for BIRTH (e.g. for B36 enter '36'): ");
        fgets(input_buffer, sizeof(input_buffer), stdin);
        parse_custom_rule(input_buffer, rule_born);

        printf("Enter neighbors count for SURVIVAL (e.g. for S23 enter '23'): ");
        fgets(input_buffer, sizeof(input_buffer), stdin);
        parse_custom_rule(input_buffer, rule_survive);
    } else {
        set_conway_rules();
    }

    printf("\n--- Initial State ---\n");
    printf("1. Random Chaos\n");
    printf("2. Glider (Test Pattern)\n");
    printf("Select option (1-2): ");
    scanf("%d", &choice);

    if (choice == 2) init_glider();
    else init_random();

    // Основен цикъл на симулацията
    int generation = 0;
    while (1) {
        draw_grid(generation);
        compute_next_generation();
        generation++;
        sleep_ms(100); // Скорост на анимацията (100ms)
    }

    return 0;
}