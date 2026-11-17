#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <queue>             
#include <chrono>       

#ifdef _WIN32
#include <windows.h>         
#endif

#define EDGE_PERCENT 35 

struct node
{
    int inf;
    struct node* next;
};

struct node* head = NULL;
struct node* tail = NULL;

struct node* get_struct(int value)
{
    struct node* p = NULL;
    if ((p = (struct node*)malloc(sizeof(struct node))) == NULL)
    {
        printf("Ошибка при распределении памяти\n");
        exit(1);
    }
    p->inf = value;
    p->next = NULL;
    return p;
}

int empty(void) { return head == NULL; }

void push(int value)
{
    struct node* p = get_struct(value);
    if (tail) tail->next = p;
    else head = p;
    tail = p;
}

int front(void) { return head->inf; }

void pop(void)
{
    struct node* p;
    if (head == NULL)
    {
        printf("Очередь пуста\n");
        return;
    }
    p = head;
    head = head->next;
    if (head == NULL) tail = NULL;
    free(p);
}

void clear_queue(void) { while (!empty()) pop(); }

static int* generate_matrix(int n, int percent) {
    int* g = (int*)calloc((size_t)n * n, sizeof(int));
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            int e = (rand() % 100) < percent ? 1 : 0;
            g[i * n + j] = e;
            g[j * n + i] = e;
        }
    return g;
}

static void print_matrix(const int* g, int n) {
    printf("Матрица смежности:\n    ");
    for (int j = 0; j < n; j++) printf("%2d ", j + 1);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("%2d: ", i + 1);
        for (int j = 0; j < n; j++) printf("%2d ", g[i * n + j]);
        printf("\n");
    }
}

static void print_probabilities(int percent) {
    printf("Вероятность ребра: %.2f\n", percent / 100.0);
}

static void bfs_matrix_std(const int* g, int n, int v0, int* visited, int print) {
    std::queue<int> Q;
    Q.push(v0);
    visited[v0] = 1;
    while (!Q.empty()) {
        int v = Q.front();
        Q.pop();
        if (print) printf("%d ", v + 1);
        for (int i = 0; i < n; i++)
            if (g[v * n + i] == 1 && !visited[i]) {
                Q.push(i);
                visited[i] = 1;
            }
    }
}

static void bfs_matrix_list(const int* g, int n, int v0, int* visited, int print) {
    push(v0);
    visited[v0] = 1;
    while (!empty()) {
        int v = front();
        pop();
        if (print) printf("%d ", v + 1);
        for (int i = 0; i < n; i++)
            if (g[v * n + i] == 1 && !visited[i]) {
                push(i);
                visited[i] = 1;
            }
    }
    clear_queue();
}

typedef void (*BfsFn)(const int*, int, int, int*, int);

static void full_traversal(const int* g, int n, BfsFn bfs, int print, int start) {
    int* visited = (int*)calloc(n, sizeof(int));
    bfs(g, n, start, visited, print);
    for (int v = 0; v < n; v++)
        if (!visited[v]) bfs(g, n, v, visited, print);
    if (print) printf("\n");
    free(visited);
}

typedef struct AdjNode {
    int to;
    struct AdjNode* next;
} AdjNode;

static AdjNode** matrix_to_lists(const int* g, int n) {
    AdjNode** adj = (AdjNode**)calloc(n, sizeof(AdjNode*));
    for (int i = 0; i < n; i++) {
        AdjNode* last = NULL;
        for (int j = 0; j < n; j++)
            if (g[i * n + j] == 1) {
                AdjNode* nd = (AdjNode*)malloc(sizeof(AdjNode));
                nd->to = j;
                nd->next = NULL;
                if (last) last->next = nd; else adj[i] = nd;
                last = nd;
            }
    }
    return adj;
}

static void print_lists(AdjNode** adj, int n) {
    printf("Списки смежности:\n");
    for (int i = 0; i < n; i++) {
        printf("%2d:", i + 1);
        for (AdjNode* p = adj[i]; p; p = p->next) printf(" %d", p->to + 1);
        printf("\n");
    }
}

static void free_lists(AdjNode** adj, int n) {
    for (int i = 0; i < n; i++) {
        AdjNode* p = adj[i];
        while (p) { AdjNode* t = p->next; free(p); p = t; }
    }
    free(adj);
}

static void bfs_adjlist_std(AdjNode** adj, int n, int v0, int* visited, int print) {
    (void)n;
    std::queue<int> Q;
    Q.push(v0);
    visited[v0] = 1;
    while (!Q.empty()) {
        int v = Q.front();
        Q.pop();
        if (print) printf("%d ", v + 1);
        for (AdjNode* p = adj[v]; p; p = p->next)
            if (!visited[p->to]) {
                Q.push(p->to);
                visited[p->to] = 1;
            }
    }
}

static void bfs_adjlist_list(AdjNode** adj, int n, int v0, int* visited, int print) {
    (void)n;
    push(v0);
    visited[v0] = 1;
    while (!empty()) {
        int v = front();
        pop();
        if (print) printf("%d ", v + 1);
        for (AdjNode* p = adj[v]; p; p = p->next)
            if (!visited[p->to]) {
                push(p->to);
                visited[p->to] = 1;
            }
    }
    clear_queue();
}

typedef void (*BfsAdjFn)(AdjNode**, int, int, int*, int);

static void full_traversal_lists(AdjNode** adj, int n, BfsAdjFn bfs, int print, int start) {
    int* visited = (int*)calloc(n, sizeof(int));
    bfs(adj, n, start, visited, print);
    for (int v = 0; v < n; v++)
        if (!visited[v]) bfs(adj, n, v, visited, print);
    if (print) printf("\n");
    free(visited);
}

static double now_ms(void) {
    using namespace std::chrono;
    return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
}

static double measure_matrix(const int* g, int n, BfsFn bfs, int reps) {
    double t0 = now_ms();
    for (int r = 0; r < reps; r++) full_traversal(g, n, bfs, 0, 0);
    return (now_ms() - t0) / reps;
}

static double measure_lists(AdjNode** adj, int n, BfsAdjFn bfs, int reps) {
    double t0 = now_ms();
    for (int r = 0; r < reps; r++) full_traversal_lists(adj, n, bfs, 0, 0);
    return (now_ms() - t0) / reps;
}

static void print_times(const int* g, AdjNode** adj, int n, int reps) {
    double t1 = measure_matrix(g, n, bfs_matrix_std, reps);
    double t2 = measure_matrix(g, n, bfs_matrix_list, reps);
    double t3 = measure_lists(adj, n, bfs_adjlist_std, reps);
    double t4 = measure_lists(adj, n, bfs_adjlist_list, reps);
    printf("Время обходов (мс, среднее за %d повт.):\n", reps);
    printf("  1) матрица  + std::queue     : %.4f\n", t1);
    printf("  2) матрица  + очередь-список : %.4f\n", t2);
    printf("  3) списки   + std::queue     : %.4f\n", t3);
    printf("  4) списки   + очередь-список : %.4f\n", t4);
}

static void benchmark(void) {
    int sizes[] = { 100, 200, 500, 1000, 2000, 3000 };
    int cnt = (int)(sizeof(sizes) / sizeof(sizes[0]));

    printf("\nСравнение времени работы (вероятность ребра = %.2f)\n", EDGE_PERCENT / 100.0);
    printf("%-8s %-14s %-14s %-14s %-14s\n",
        "n",
        "матр+std, мс", "матр+спис, мс", "спис+std, мс", "спис+спис, мс");

    for (int k = 0; k < cnt; k++) {
        int n = sizes[k];
        int* g = generate_matrix(n, EDGE_PERCENT);
        AdjNode** adj = matrix_to_lists(g, n);
        int reps = n <= 500 ? 50 : (n <= 1000 ? 10 : 3);

        double t1 = measure_matrix(g, n, bfs_matrix_std, reps);
        double t2 = measure_matrix(g, n, bfs_matrix_list, reps);
        double t3 = measure_lists(adj, n, bfs_adjlist_std, reps);
        double t4 = measure_lists(adj, n, bfs_adjlist_list, reps);

        printf("%-8d %-14.4f %-14.4f %-14.4f %-14.4f\n",
            n, t1, t2, t3, t4);

        free_lists(adj, n);
        free(g);
    }
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    srand((unsigned)time(NULL));

    int n;
    printf("Введите число вершин графа: ");
    if (scanf("%d", &n) != 1 || n < 1) {
        printf("Некорректный ввод.\n");
        return 1;
    }

    int* g = generate_matrix(n, EDGE_PERCENT);
    print_matrix(g, n);
    printf("\n");
    print_probabilities(EDGE_PERCENT);

    AdjNode** adj = matrix_to_lists(g, n);
    printf("\n");
    print_lists(adj, n);

    int start;
    printf("\nВведите номер вершины, с которой начать обход (1..%d): ", n);
    if (scanf("%d", &start) != 1 || start < 1 || start > n) {
        printf("Некорректный номер вершины.\n");
        free_lists(adj, n);
        free(g);
        return 1;
    }
    start--;

    printf("\nОбход в ширину (матрица смежности, std::queue):\n");
    full_traversal(g, n, bfs_matrix_std, 1, start);

    printf("\nОбход в ширину (матрица смежности, очередь на списке):\n");
    full_traversal(g, n, bfs_matrix_list, 1, start);

    printf("\nОбход в ширину (списки смежности, std::queue):\n");
    full_traversal_lists(adj, n, bfs_adjlist_std, 1, start);

    printf("\nОбход в ширину (списки смежности, очередь на списке):\n");
    full_traversal_lists(adj, n, bfs_adjlist_list, 1, start);

    printf("\n");
    print_times(g, adj, n, n <= 100 ? 1000 : (n <= 500 ? 50 : 5));

    free_lists(adj, n);
    free(g);

    benchmark();

    return 0;
}