#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_VAGAS_LAVAGEM 2 /*duas vaga de lavagem*/
#define NUM_VAGAS_SECAGEM 2 /*duas vagas de secagem*/
#define NUM_VAGAS_ESPERA 4 /*duas vagas de espera*/
#define NUM_FUNCIONARIOS 4 /*quatro funcionarios*/
#define NUM_CLIENTES 25 /*dez clientes*/

sem_t sem_vagas_espera;
sem_t sem_vagas_lavagem;
sem_t sem_vagas_secagem;
sem_t sem_cliente_vaga_secagem;
sem_t sem_carro_lavado;
sem_t sem_cliente_vaga;
sem_t sem_carro_seco;

void* f_funcionario_lavagem (void *v) {
	int id = *(int *) v;
	sleep(id%3);
	while(1) {
		sem_wait(&sem_cliente_vaga); /*lava um dos carros que esta na vaga de lavagem*/
		// sleep(3); /*esse sleep deixa as lavagens muito demoradas e uma pa d eclientes nao entra no lavajato*/
		printf("Um carro foi lavado pelo funcionario %d.\n",id);
		sem_post(&sem_carro_lavado); /*o carro foi lavado por um funcionario*/
		sem_wait(&sem_cliente_vaga_secagem); /*espera o cliente na vaga de secagem*/
		printf("Um carro foi seco e encerado pelo funcionario %d\n", id);
		sem_post(&sem_carro_seco);
	}
	return NULL;
}

void* f_cliente(void* v) {
	int id = *(int*) v;
	sleep(id%3);

	if(sem_trywait(&sem_vagas_espera) == 0) { /*verifica se as vagas de espera podem ser decrementadas, se puderem, o cliente pode entrar e esperar*/
		printf("Carro do cliente %d entrou no lava jato.\n", id); /**/
		sem_wait(&sem_vagas_lavagem); /*o cliente chama um funcionario para atende-lo e entra na vaga de lavagem*/
		printf("Carro do cliente %d entrou na vaga de lavagem.\n", id);
		sem_post(&sem_cliente_vaga); /*tem um cliente em uma vaga de lavagem*/
		sem_post(&sem_vagas_espera); /*libera uma vaga de espera*/
		sem_wait(&sem_carro_lavado); /*um dos carros ja foi lavado*/
		sem_wait(&sem_vagas_secagem); /*espera por vaga de secagem*/
		sem_post(&sem_vagas_lavagem); /*aumenta o numero de vagas de lavagem disponiveis*/
		printf("Carro do cliente %d entrou na vaga de secagem\n",id);
		sem_post(&sem_cliente_vaga_secagem);
		sem_post(&sem_vagas_secagem);
		sem_wait(&sem_carro_seco);
		printf("Carro do cliente %d deixou o lava jato.\n",id);
	}
	else {
		printf("Carro do cliente %d nao entrou no lava jato.\n", id);	/*tava lotada essa porra*/
	}
	return NULL;
}

int main() {
	pthread_t thr_clientes[NUM_CLIENTES], thr_funcionario[NUM_FUNCIONARIOS];
	int i, id_clientes[NUM_CLIENTES], id_funcionarios[NUM_FUNCIONARIOS];

	sem_init(&sem_vagas_espera, 0, NUM_VAGAS_ESPERA);
	sem_init(&sem_vagas_lavagem, 0, NUM_VAGAS_LAVAGEM);
	sem_init(&sem_vagas_secagem, 0, NUM_VAGAS_SECAGEM);
	sem_init(&sem_carro_lavado, 0, 0);
	sem_init(&sem_cliente_vaga_secagem, 0, 0);
	sem_init(&sem_cliente_vaga, 0, 0);
	sem_init(&sem_carro_seco,0 ,0);

	for(i = 0; i < NUM_FUNCIONARIOS; i++) {
			id_funcionarios[i] = i;
	  	pthread_create(&thr_funcionario[i], NULL, f_funcionario_lavagem, (void*) &id_funcionarios[i]);
		}

	for(i = 0; i < NUM_CLIENTES; i++) {
			id_clientes[i] = i;
			pthread_create(&thr_clientes[i], NULL, f_cliente, (void*) &id_clientes[i]);
  	}


	for (i = 0; i < NUM_CLIENTES; i++)
		pthread_join(thr_clientes[i], NULL);

	return 0;
}
