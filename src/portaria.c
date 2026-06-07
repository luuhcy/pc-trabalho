#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_PESSOAS     10
#define NUM_CATRACAS     5
#define OPS_POR_CATRACA  6

/* estado compartilhado entre todas as threads */
typedef struct {
    int pessoas_dentro;
    int em_manutencao;
    pthread_mutex_t mutex;
    pthread_cond_t tem_vaga;    /* sinalizada quando alguém sai */
    pthread_cond_t nao_vazio;   /* sinalizada quando alguém entra */
    pthread_cond_t fim_manut;   /* sinalizada quando a manutenção acaba */
} Portaria;

static Portaria p;

void entrada(int id) {
    pthread_mutex_lock(&p.mutex);

    /* espera a manutenção acabar antes de qualquer coisa */
    while (p.em_manutencao)
        pthread_cond_wait(&p.fim_manut, &p.mutex);

    /* while em vez de if: reavalia a condição após cada wakeup */
    while (p.pessoas_dentro >= MAX_PESSOAS) {
        printf("[Catraca %d] lotado, aguardando vaga...\n", id);
        pthread_cond_wait(&p.tem_vaga, &p.mutex);
    }

    p.pessoas_dentro++;
    printf("[Catraca %d] entrada | dentro: %d/%d\n", id, p.pessoas_dentro, MAX_PESSOAS);
    pthread_cond_signal(&p.nao_vazio);

    pthread_mutex_unlock(&p.mutex);
}

void saida(int id) {
    pthread_mutex_lock(&p.mutex);

    while (p.pessoas_dentro <= 0)
        pthread_cond_wait(&p.nao_vazio, &p.mutex);

    p.pessoas_dentro--;
    printf("[Catraca %d] saida  | dentro: %d/%d\n", id, p.pessoas_dentro, MAX_PESSOAS);
    pthread_cond_signal(&p.tem_vaga);

    pthread_mutex_unlock(&p.mutex);
}

void *catraca(void *arg) {
    int id = *(int *)arg;
    free(arg);

    /* alterna entrada e saída: índices pares entram, ímpares saem */
    for (int i = 0; i < OPS_POR_CATRACA; i++) {
        if (i % 2 == 0)
            entrada(id);
        else
            saida(id);
        usleep((rand() % 500 + 100) * 1000); /* intervalo aleatório entre ops */
    }

    return NULL;
}

void *porteiro(void *arg) {
    (void)arg;
    sleep(1); /* deixa as catracas operarem um pouco antes de bloquear */

    pthread_mutex_lock(&p.mutex);
    p.em_manutencao = 1;
    printf("\n[Porteiro] manutencao iniciada\n\n");
    pthread_mutex_unlock(&p.mutex);

    sleep(2); /* tempo de manutenção */

    pthread_mutex_lock(&p.mutex);
    p.em_manutencao = 0;
    printf("\n[Porteiro] manutencao encerrada\n\n");
    /* broadcast porque pode ter várias catracas esperando ao mesmo tempo */
    pthread_cond_broadcast(&p.fim_manut);
    pthread_mutex_unlock(&p.mutex);

    return NULL;
}

int main(void) {
    srand((unsigned)time(NULL));

    p.pessoas_dentro = 0;
    p.em_manutencao  = 0;
    pthread_mutex_init(&p.mutex,    NULL);
    pthread_cond_init(&p.tem_vaga,  NULL);
    pthread_cond_init(&p.nao_vazio, NULL);
    pthread_cond_init(&p.fim_manut, NULL);

    pthread_t catracas[NUM_CATRACAS], port;
    pthread_create(&port, NULL, porteiro, NULL);

    for (int i = 0; i < NUM_CATRACAS; i++) {
        int *id = malloc(sizeof(int)); /* alocado no heap pra não ter race condition com &i */
        *id = i + 1;
        pthread_create(&catracas[i], NULL, catraca, id);
    }

    for (int i = 0; i < NUM_CATRACAS; i++)
        pthread_join(catracas[i], NULL);
    pthread_join(port, NULL);

    printf("\n[Main] fim. pessoas dentro: %d\n", p.pessoas_dentro);

    pthread_mutex_destroy(&p.mutex);
    pthread_cond_destroy(&p.tem_vaga);
    pthread_cond_destroy(&p.nao_vazio);
    pthread_cond_destroy(&p.fim_manut);

    return 0;
}