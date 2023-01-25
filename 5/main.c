#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
typedef struct VertexInner Vertex;
typedef struct EdgeInner Edge;
typedef struct PriorityQueueInner PriorityQueue;
struct EdgeInner
{
    Vertex *destination;
    int weight;
};
struct VertexInner
{
    short row;
    short col;
    Edge *adjacent_list; //a vertex has at most 3 neighbors.
    int adjacent_count;

    //intermidate variabls used in dijkstra algorithm.
    Vertex *prev_vertex;
    int time;
    int location; //array index in queue array.
};

struct PriorityQueueInner
{
    Vertex **vertexs;
    int size;
};
//create a queue, it is a minimal root heap
static PriorityQueue *create_queue(int capacity);
//add an new vertex into queue
static void enqueue(PriorityQueue *queue, Vertex *vertex);
//remove the first vertex in queue;
static Vertex *dequeue(PriorityQueue *queue);
//free all the dynamically allocated memory
static void free_queue(PriorityQueue *queue);
//decrease the time of vertex
static void decrease_time(PriorityQueue *queue, Vertex *vertex);

/**
 * read binary grid file
 * store the result in content, rows, cols
*/
static void read_binary_grid(const char *path, int ***matrix, int *rows, int *cols);

//print the matrix to the fail with Text grid file format
static void write_text_grid(const char *path, int **matrix, int rows, int cols);

//free all the dynamically allocated memory
static void free_int_matrix(int **matrix, int rows, int cols);
static void free_vertex_matrix(Vertex **matrix, int rows, int cols);

/**
 * construct the graph for the matrix.
 * Every cell will be a graph vertex, its neighbors are its adjacent node.
 * the vertex at the last row will be connected with goal_vertex
*/
static void build_graph(int **matrix, int rows, int cols, Vertex ***vertex_matrix, Vertex **start_vertex);

static void enqueue_vertex(PriorityQueue *queue, Vertex *v, Vertex *source);
//the algorithm is dijkstra algorithm
static void run_dijkstra(Vertex **vertex_matrix, int rows, int cols, Vertex *source);

int main(int argc, char **argv)
{
    int **matrix = 0;
    int rows = 0;
    int cols = 0;
    //1. read binary grid file
    read_binary_grid(argv[1], &matrix, &rows, &cols);
    //2. write text grid file
    write_text_grid(argv[2], matrix, rows, cols);
    //build graph
    Vertex **vertex_matrix = 0;
    Vertex *start_vertex = 0;
    build_graph(matrix, rows, cols, &vertex_matrix, &start_vertex);

    //3. write fastest times file
    run_dijkstra(vertex_matrix, rows, cols, start_vertex);
    int opt_column = -1;
    int opt_time = INT_MAX;
    {
        FILE *fp = fopen(argv[3], "wb");
        if (fp)
        {
            short s_cols = cols;
            fwrite(&s_cols, 1, sizeof(short), fp);
            for (int c = 0; c < cols; c++)
            {
                Vertex *v = &(vertex_matrix[0][c]);
                int time = v->time + matrix[0][c];
                fwrite(&time, 1, sizeof(int), fp);
                if (opt_time > time)
                {
                    opt_column = c;
                    opt_time = time;
                }
            }
            fclose(fp);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }

    //4. Fastest path file format
    {
        FILE *fp = fopen(argv[4], "wb");
        if (fp)
        {
            fwrite(&opt_time, 1, sizeof(int), fp);
            Vertex *current_vertex = &(vertex_matrix[0][opt_column]);
            int count = 0;
            while (current_vertex != start_vertex)
            {
                count++;
                current_vertex = current_vertex->prev_vertex;
            }
            fwrite(&count, 1, sizeof(int), fp);

            current_vertex = &(vertex_matrix[0][opt_column]);
            while (current_vertex != start_vertex)
            {
                fwrite(&current_vertex->row, 1, sizeof(short), fp);
                fwrite(&current_vertex->col, 1, sizeof(short), fp);
                current_vertex = current_vertex->prev_vertex;
            }

            fclose(fp);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }

    //free all data.
    free(start_vertex->adjacent_list);
    free(start_vertex);
    free_vertex_matrix(vertex_matrix, rows, cols);
    free_int_matrix(matrix, rows, cols);
    return 0;
}

void write_text_grid(const char *path, int **matrix, int rows, int cols)
{
    FILE *fp = fopen(path, "w");
    if (fp)
    {
        fprintf(fp, "%d %d\n", rows, cols);
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                fprintf(fp, "%d", matrix[i][j]);
                if (j + 1 != cols)
                {
                    fprintf(fp, " ");
                }
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    else
    {
        printf("can not write %s.\n", path);
        exit(EXIT_FAILURE);
    }
}

//read size bytes and store them in p from fp.
//if any error occur, report it.
static void safe_read(void *p, size_t size, FILE *fp)
{
    if (fread(p, 1, size, fp) != size)
    {
        printf("read error.\n");
        exit(EXIT_FAILURE);
    }
}

//r is in [0, rows) and c is in [0, cols)
static int is_in_range(int rows, int cols, int r, int c)
{
    return (r >= 0 && r < rows && c >= 0 && c < cols) ? 1 : 0;
}

void build_graph(int **matrix, int rows, int cols, Vertex ***pvertex_matrix, Vertex **pstart_vertex)
{
    Vertex **vertex_matrix = (Vertex **)calloc(sizeof(Vertex *) * rows, 1);
    *pvertex_matrix = vertex_matrix;
    for (int i = 0; i < rows; i++)
    {
        vertex_matrix[i] = (Vertex *)calloc(sizeof(Vertex) * cols, 1);
    }

    //left, right, down
    int rds[] = {0, 0, -1};
    int cds[] = {-1, 1, 0};
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            Vertex *v = &(vertex_matrix[r][c]);
            v->row = r;
            v->col = c;
            v->adjacent_list = (Edge *)calloc(sizeof(Edge) * 3, 1);
            //candidate neighbors.
            for (int k = 0; k < 3; k++)
            {
                int nr = r + rds[k];
                int nc = c + cds[k];
                if (is_in_range(rows, cols, nr, nc))
                {
                    v->adjacent_list[v->adjacent_count].destination = &(vertex_matrix[nr][nc]);
                    v->adjacent_list[v->adjacent_count].weight = matrix[r][c];
                    v->adjacent_count++;
                    assert(v->adjacent_count <= 3);
                }
            }
        }
    }

    {
        Vertex *start_node = (Vertex *)calloc(sizeof(Vertex), 1);
        *pstart_vertex = start_node;
        start_node->adjacent_list = (Edge *)calloc(sizeof(Edge) * cols, 1);
        start_node->adjacent_count = cols;
        if (rows > 0)
        {
            int r = rows - 1;
            for (int c = 0; c < cols; c++)
            {
                Vertex *v = &(vertex_matrix[r][c]);
                start_node->adjacent_list[c].destination = v;
            }
        }
    }
}

void read_binary_grid(const char *path, int ***pcontent, int *rows, int *cols)
{
    FILE *fp = fopen(path, "rb");
    if (fp)
    {
        short m = 0;
        short n = 0;
        safe_read(&m, sizeof(m), fp);
        safe_read(&n, sizeof(n), fp);
        *rows = m;
        *cols = n;
        int **content = (int **)calloc(sizeof(int *) * m, 1);
        *pcontent = content;
        for (int i = 0; i < m; i++)
        {
            content[i] = (int *)calloc(sizeof(int) * n, 1);
        }
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                short tmp = 0;
                safe_read(&tmp, sizeof(tmp), fp);
                content[i][j] = tmp;
            }
        }
        fclose(fp);
    }
    else
    {
        printf("can not read %s.\n", path);
        exit(EXIT_FAILURE);
    }
}
static void free_vertex_matrix(Vertex **matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            free(matrix[i][j].adjacent_list);
        }
        free(matrix[i]);
    }
    free(matrix);
}

void free_int_matrix(int **matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

PriorityQueue *create_queue(int capacity)
{
    PriorityQueue *queue = (PriorityQueue *)calloc(sizeof(PriorityQueue), 1);
    queue->vertexs = (Vertex **)calloc(sizeof(Vertex *) * capacity, 1);
    return queue;
}
void enqueue(PriorityQueue *queue, Vertex *vertex)
{
    queue->size++;
    int location = queue->size - 1;
    vertex->location = location;
    queue->vertexs[location] = vertex;
    decrease_time(queue, vertex);
}
static void exchange_vertex(PriorityQueue *queue, int i, int j)
{
    Vertex *v = queue->vertexs[i];
    queue->vertexs[i] = queue->vertexs[j];
    queue->vertexs[j] = v;
    queue->vertexs[i]->location = i;
    queue->vertexs[j]->location = j;
}
Vertex *dequeue(PriorityQueue *queue)
{
    Vertex *pop_vertex = NULL;
    if (queue->size > 0)
    {
        Vertex **vertexs = queue->vertexs;
        pop_vertex = vertexs[0];
        pop_vertex->location = -1;
        queue->size--;
        if (queue->size > 0)
        {
            vertexs[0] = vertexs[queue->size];
            vertexs[queue->size] = 0;
            vertexs[0]->location = 0;
            int i = 0;
            while (1)
            {
                int li = 2 * i + 1;
                int ri = 2 * i + 2;
                int opti = i;
                if (li < queue->size && vertexs[li]->time < vertexs[opti]->time)
                {
                    opti = li;
                }
                if (ri < queue->size && vertexs[ri]->time < vertexs[opti]->time)
                {
                    opti = ri;
                }
                if (opti != i)
                {
                    exchange_vertex(queue, i, opti);
                    i = opti;
                }
                else
                {
                    break;
                }
            }
        }
    }
    return pop_vertex;
}
void free_queue(PriorityQueue *queue)
{
    free(queue->vertexs);
    free(queue);
}
void decrease_time(PriorityQueue *queue, Vertex *vertex)
{
    int i = vertex->location;
    Vertex **vertexs = queue->vertexs;
    while (i > 0)
    {
        int pi = (i - 1) / 2;
        if (vertexs[i]->time < vertexs[pi]->time)
        {
            exchange_vertex(queue, i, pi);
            i = pi;
        }
        else
        {
            break;
        }
    }
}
void run_dijkstra(Vertex **vertex_matrix, int rows, int cols, Vertex *source)
{
    PriorityQueue *queue = create_queue(rows * cols + 1);

    //make all data into queue.
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            enqueue_vertex(queue, &(vertex_matrix[i][j]), source);
        }
    }
    enqueue_vertex(queue, source, source);

    Vertex *u = NULL;
    while ((u = dequeue(queue)))
    {
        for (int i = 0; i < u->adjacent_count; i++)
        {
            Edge *edge = &(u->adjacent_list[i]);
            Vertex *v = edge->destination;
            if (u->time != INT_MAX)
            {
                double new_time = u->time + edge->weight;
                if (v->time > new_time)
                {
                    v->prev_vertex = u;
                    v->time = new_time;
                    decrease_time(queue, v);
                }
            }
        }
    }
    free_queue(queue);
}

void enqueue_vertex(PriorityQueue *queue, Vertex *v, Vertex *source)
{
    v->location = -1;
    v->time = INT_MAX;
    v->prev_vertex = NULL;
    if (source == v)
    {
        source->time = 0.0;
    }
    enqueue(queue, v);
}