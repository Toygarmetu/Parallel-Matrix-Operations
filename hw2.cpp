#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include "hw2_output.h"
#include <semaphore.h>

using namespace std;
struct RowMonitor
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool finished = false;
};

struct ColumnMonitor
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int num;
    bool finished;
};

// Declaration of various global variables and vectors to store threads and matrices
int n, m, k, value;
vector<pthread_t *> additions;
vector<pthread_t *> additions2;
vector<pthread_t *> multiplications;
vector<vector<int> > matrix1;
vector<vector<int> > matrix2;
vector<vector<int> > matrix3;
vector<vector<int> > matrix4;
vector<vector<int> > matrix5;
vector<vector<int> > matrix6;
vector<vector<int> > matrix7;
vector<RowMonitor> RowMonitors;
vector<ColumnMonitor> ColumnMonitors;

void initRowMonitor(RowMonitor *monitor)
{
    pthread_mutex_init(&monitor->mutex, NULL);
    pthread_cond_init(&monitor->cond, NULL);
    monitor->finished = false;
}

void initColumnMonitor(ColumnMonitor *monitor)
{
    pthread_mutex_init(&monitor->mutex, NULL);
    pthread_cond_init(&monitor->cond, NULL);
    monitor->num = 0;
    monitor->finished = false;
}

void waitRaw(RowMonitor *monitor)
{
    pthread_mutex_lock(&monitor->mutex);
    while (!monitor->finished)
    {
        pthread_cond_wait(&monitor->cond, &monitor->mutex);
    }
    pthread_mutex_unlock(&monitor->mutex);
}

void signalRow(RowMonitor *monitor)
{
    pthread_mutex_lock(&monitor->mutex);
    monitor->finished = true;
    pthread_cond_broadcast(&monitor->cond);
    pthread_mutex_unlock(&monitor->mutex);
}

void waitColumn(ColumnMonitor *monitor)
{
    pthread_mutex_lock(&monitor->mutex);
    while (!monitor->finished)
    {
        pthread_cond_wait(&monitor->cond, &monitor->mutex);
    }
    pthread_mutex_unlock(&monitor->mutex);
}

void signalColumn(ColumnMonitor *monitor)
{
    pthread_mutex_lock(&monitor->mutex);
    monitor->num += 1;
    if (monitor->num >= m)
    {
        monitor->finished = true;
        pthread_cond_broadcast(&monitor->cond);
    }
    pthread_mutex_unlock(&monitor->mutex);
}


void *addition_thread1(void *pp)
{
    long row = (long)pp;
    int size = matrix1.at(0).size();
    for (int i = 0; i < size; i++)
    {
        matrix5[row][i] = matrix1[row][i] + matrix2[row][i];
        hw2_write_output(0, row + 1, i + 1, matrix5[row][i]);
    }
    signalRow(&RowMonitors[row]);
    return NULL;
}
int counter = 0;

void *addition_thread2(void *pp)
{

    long row = (long)pp;
    int size = matrix3.at(0).size();
    for (int i = 0; i < size; i++)
    {
        matrix6[row][i] = matrix3[row][i] + matrix4[row][i];
        hw2_write_output(1, row + 1, i + 1, matrix6[row][i]);
        signalColumn(&ColumnMonitors[i]);
    }
    return NULL;
}


void *multiplication_thread(void *pp)
{
    long row = (long)pp;
    int size = matrix6.size();
    int matrixSize = matrix6.at(0).size();
    waitRaw(&RowMonitors[row]);
    for (int i = 0; i < matrixSize; i++)
    {
        waitColumn(&ColumnMonitors[i]);
        for (int j = 0; j < size; j++)
        {
            matrix7[row][i] += matrix5[row][j] * matrix6[j][i];
            hw2_write_output(2, row + 1, i + 1, matrix7[row][i]);
        }
    }
    return NULL;
}


int main()
{
    hw2_init_output();

    
    cin >> n >> m;
    matrix1.resize(n, vector<int>(m));
    for (int x = 0; x < n; x++)
    {
        for (int y = 0; y < m; y++)
        {
            cin >> matrix1[x][y];
        }
    }

    cin >> n >> m; // row n column m
    matrix2.resize(n, vector<int>(m));
    for (int x = 0; x < n; x++)
    {
        for (int y = 0; y < m; y++)
        {
            cin >> matrix2[x][y];
        }
    }

    cin >> m >> k; // row m column k

    matrix3.resize(m, vector<int>(k));
    for (int x = 0; x < m; x++)
    {
        for (int y = 0; y < k; y++)
        {
            cin >> matrix3[x][y];
        }
    }

    matrix4.resize(m, vector<int>(k));
    cin >> m >> k;
    for (int x = 0; x < m; x++)
    {
        for (int y = 0; y < k; y++)
        {
            cin >> matrix4[x][y];
        }
    }

    matrix5.resize(n, vector<int>(m));
    matrix6.resize(m, vector<int>(k));
    matrix7.resize(n, vector<int>(k));

    additions.resize(n);
    additions2.resize(m);
    multiplications.resize(n);
    RowMonitors.resize(n);
    ColumnMonitors.resize(k);

    for (int x = 0; x < n; x++)
    {
        initRowMonitor(&RowMonitors[x]);
    }
    for (int x = 0; x < k; x++)
    {
        initColumnMonitor(&ColumnMonitors[x]);
    }

    
    for (long i = 0; i < additions.size(); i++)
    {
        additions[i] = new pthread_t;
        pthread_create(additions[i], NULL, addition_thread1, (void *)i);
    }

    
    for (long i = 0; i < additions2.size(); i++)
    {
        additions2[i] = new pthread_t;
        pthread_create(additions2[i], NULL, addition_thread2, (void *)i);
    }

  
    for (long i = 0; i < multiplications.size(); i++)
    {
        multiplications[i] = new pthread_t;
        pthread_create(multiplications[i], NULL, multiplication_thread, (void *)i);
    }

    
    for (long i = 0; i < additions.size(); i++)
    {
        pthread_join(*(additions[i]), NULL);
        delete additions[i];
    }

    for (long i = 0; i < additions2.size(); i++)
    {
        pthread_join(*(additions2[i]), NULL);
        delete additions2[i];
    }

    for (int i = 0; i < multiplications.size(); i++)
    {
        pthread_join(*(multiplications[i]), NULL);
        delete multiplications[i];
    }

   
    for (int i = 0; i < matrix7.size(); i++)
    {
        for (int j = 0; j < matrix7.at(0).size(); j++)
        {
            cout << matrix7[i][j] << " ";
        }
        cout << endl;
    }

    return 0;
}
