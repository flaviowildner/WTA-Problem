#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux__
#define CLEAR_CONSOLE "clear"
#elif _WIN32
#define CLEAR_CONSOLE "cls"
#endif


struct alvo {
	float valor;
	int tipo;
};

struct arma {
	int tipo;
	struct alvo alvo;
};

struct cromossomo {
	struct arma *armas;
	float fitness;
};


int i, j, k;
int numeroTotalArmas = 0;
int numeroTipoArmas = 0, numeroAlvos = 0;
int nPopulacao = 0;
int taxaMutacao;
int iteracao;
int maxIteracoes;
int pontoBuscaLocal = 1;
int tempoLimite;
int criterioParada;



struct arma *VetorArmas = NULL;
struct alvo *VetorAlvos = NULL;
float **matrizProbabilidades = NULL;

struct cromossomo *populacao = NULL;
struct cromossomo *novaPopulacao = NULL;
struct cromossomo melhorSolucao;



void ordenarPopulacao() {
	int nPopulacaoTemp = nPopulacao;
	int i = nPopulacao / 2, pai, filho;
	struct cromossomo aux;
	for (;;) {
		if (i > 0) {
			i--;
			aux = populacao[i];
		}
		else {
			nPopulacaoTemp--;
			if (nPopulacaoTemp == 0) return;
			aux = populacao[nPopulacaoTemp];
			populacao[nPopulacaoTemp] = populacao[0];
		}
		pai = i;
		filho = i * 2 + 1;
		while (filho < nPopulacaoTemp) {
			if ((filho + 1 < nPopulacaoTemp) && (populacao[filho + 1].fitness > populacao[filho].fitness))
				filho++;
			if (populacao[filho].fitness > aux.fitness) {
				populacao[pai] = populacao[filho];
				pai = filho;
				filho = pai * 2 + 1;
			}
			else {
				break;
			}
		}
		populacao[pai] = aux;
	}
}

void mutacao() {
	int random;
	int i, j;

	for (i = 0; i < nPopulacao; i++) {
		for (j = 0; j < numeroTotalArmas; j++) {
			if (rand() % 100 < taxaMutacao) {

				random = rand() % numeroAlvos;
				while (random == populacao[i].armas[j].alvo.tipo) {
					random = rand() % numeroAlvos;
				}
				populacao[i].armas[j].alvo.tipo = random;
			}
		}
	}
}

float avaliar(struct cromossomo *populacao) {
	float mult = 0;
	float somatorio = 0;
	float aux;


	for (j = 0; j < numeroAlvos; j++) {
		aux = VetorAlvos[j].valor;
		mult = 1;

		for (k = 0; k < numeroTotalArmas; k++) {
			if (populacao->armas[k].alvo.tipo == j) {
				mult = (1 - matrizProbabilidades[populacao->armas[k].tipo][j]) * mult;
			}
		}
		somatorio += aux * mult;
	}
	populacao->fitness = somatorio;
	return somatorio;
}

void avaliar_populacao() {
	for (i = 0; i < nPopulacao; i++)
		avaliar(&populacao[i]);

	ordenarPopulacao();
	if (populacao[0].fitness < melhorSolucao.fitness) {
		melhorSolucao = populacao[0];
		iteracao = 0;
		for (k = 0; k < numeroTotalArmas; k++) {
			printf("%1d ", melhorSolucao.armas[k].alvo.tipo);
		}
		printf("\t%f\n", melhorSolucao.fitness);
	}
}

void buscal_local() {
	int i, j, k;
	struct cromossomo temp;
	temp.armas = (struct arma*)malloc(numeroTotalArmas * sizeof(struct arma));
	for (i = 0; i < numeroTotalArmas; i++)
		temp.armas[i].tipo = VetorArmas[i].tipo;

	for (i = 0; i < nPopulacao; i++) {
		for (k = 0; k < numeroTotalArmas; k++)
			temp.armas[k].alvo.tipo = populacao[i].armas[k].alvo.tipo;
		for (j = 0; j < numeroTotalArmas; j++) {
			for (k = 0; k < numeroAlvos; k++) {
				temp.armas[j].alvo.tipo = k;
				if (avaliar(&temp) < populacao[i].fitness) {
					populacao[i].armas[j].alvo.tipo = k;
					avaliar(&populacao[i]);
				}
			}
			temp.armas[j].alvo.tipo = populacao[i].armas[j].alvo.tipo;
		}
	}
}

void imprimirpopulacao(struct cromossomo *populacao) {
	for (i = 0; i < nPopulacao; i++) {
		printf("%d: ", i);
		for (j = 0; j < numeroTotalArmas; j++) {
			printf("%d ", populacao[i].armas[j].alvo.tipo);
		}
		printf("%f\n", populacao[i].fitness);
	}
	printf("\n\n");
}

void start(FILE *in_file) {
	int intTemp;
	float floatTemp;
	float tempProbabilidades;
	fscanf(in_file, "%d", &numeroTipoArmas);

	for (i = 0; i < numeroTipoArmas; i++) {
		fscanf(in_file, "%d", &intTemp);

		numeroTotalArmas += intTemp;

		VetorArmas = (struct arma*)realloc(VetorArmas, numeroTotalArmas * sizeof(struct arma));

		for (k = numeroTotalArmas - intTemp; k < numeroTotalArmas; k++) {
			VetorArmas[k].tipo = i;
		}
	}
	melhorSolucao.armas = (struct arma*)malloc(numeroTotalArmas * sizeof(struct arma));
	intTemp = 0;


	fscanf(in_file, "%d", &numeroAlvos);
	VetorAlvos = (struct alvo*)malloc(numeroAlvos * sizeof(struct alvo));


	for (i = 0; i < numeroAlvos; i++) {
		fscanf(in_file, "%f", &floatTemp);
		VetorAlvos[i].valor = floatTemp;
		VetorAlvos[i].tipo = i;
	}


	matrizProbabilidades = (float**)malloc(numeroTipoArmas * sizeof(float*));
	for (i = 0; i < numeroTipoArmas; i++) {
		matrizProbabilidades[i] = (float*)malloc(numeroAlvos * sizeof(float));
	}


	for (i = 0; i < numeroTipoArmas; i++) {
		for (j = 0; j < numeroAlvos; j++) {
			fscanf(in_file, "%f", &tempProbabilidades);
			matrizProbabilidades[i][j] = tempProbabilidades;
		}
	}
}

void iniciarpopulacao() {
	populacao = (struct cromossomo*)malloc(nPopulacao * sizeof(struct cromossomo));
	for (i = 0; i < nPopulacao; i++) {
		populacao[i].armas = (struct arma*)malloc(numeroTotalArmas * sizeof(struct arma));
		for (j = 0; j < numeroTotalArmas; j++) {
			populacao[i].armas[j].tipo = VetorArmas[j].tipo;
			populacao[i].armas[j].alvo = VetorAlvos[rand() % numeroAlvos];
		}
	}

	novaPopulacao = (struct cromossomo*)malloc(nPopulacao * sizeof(struct cromossomo));
	for (i = 0; i < nPopulacao; i++) {
		novaPopulacao[i].armas = (struct arma*)malloc(numeroTotalArmas * sizeof(struct arma));
		for (j = 0; j < numeroTotalArmas; j++) {
			novaPopulacao[i].armas[j].tipo = VetorArmas[j].tipo;
		}
	}
	avaliar_populacao();
}

void crossover() {
	int selecionado1, selecionado2 = -1;
	float faixa = 0, random = 0, random2 = 0;
	int corte = 0;
	float total = 0;
	int over = 1;

	for (i = 0; i < nPopulacao; i++) {
		total += populacao[i].fitness;
	}

	if (total > RAND_MAX)
		over += (int)(total / RAND_MAX);

	for (i = 0; i < nPopulacao; i++) {

		//SELECAO DOS PAIS
		corte = (rand() % (numeroTotalArmas - 1)) + 1;


		for (j = 0; j < over; j++)
			random += (float)rand();

		random /= (over * RAND_MAX / total);
		faixa = 0;
		for (selecionado1 = 0; selecionado1 < nPopulacao; selecionado1++) {
			faixa += populacao[selecionado1].fitness;
			if (random < faixa) {
				break;
			}
		}


		do {
			random2 = 0;
			for (j = 0; j < over; j++)
				random2 += (float)rand();

			random2 /= (over * RAND_MAX / total);
			faixa = 0;
			for (selecionado2 = 0; selecionado2 < nPopulacao; selecionado2++) {
				faixa += populacao[selecionado2].fitness;
				if (random2 < faixa) {
					break;
				}
			}
		} while (selecionado1 == selecionado2);

		selecionado1 = abs(selecionado1 - (nPopulacao - 1));
		selecionado2 = abs(selecionado2 - (nPopulacao - 1));
		////////////////////////


		//REPRODUCAO
		for (j = 0; j < corte; j++) {
			novaPopulacao[i].armas[j].alvo.tipo = populacao[selecionado1].armas[j].alvo.tipo;
		}
		for (j = corte; j < numeroTotalArmas; j++) {
			novaPopulacao[i].armas[j].alvo.tipo = populacao[selecionado2].armas[j].alvo.tipo;
		}
		/////////////////

	}


	for (i = 0; i < nPopulacao; i++) {
		for (j = 0; j < numeroTotalArmas; j++) {
			populacao[i].armas[j].alvo.tipo = novaPopulacao[i].armas[j].alvo.tipo;
		}
	}
}

int proxima(struct cromossomo *seq, int N, int M) {
	int t = N - 1;
	while (t >= 0) {

		seq->armas[t].alvo.tipo = (seq->armas[t].alvo.tipo + 1) % M;
		if (seq->armas[t].alvo.tipo == 0)
			t--;
		else
			return 0;
	}
	return -1;
}

void imp_seq_n_base_m(struct cromossomo *seq, int n, int m) {
	int i;
	int k;
	float curr = 999999;
	for (i = 0; i < n; i++)
		seq->armas[i].alvo.tipo = 0;
	do {

		curr = avaliar(seq);

		if (curr < melhorSolucao.fitness) {
			melhorSolucao.fitness = curr;
			for (k = 0; k < numeroTotalArmas; k++) {
				melhorSolucao.armas[k].alvo.tipo = seq->armas[k].alvo.tipo;
			}
		}
	} while (proxima(seq, n, m) == 0);
}

void exaustivo() {
	int i = 0;
	struct cromossomo current;
	current.armas = (struct arma*)malloc(numeroTotalArmas * sizeof(struct arma));
	for (i = 0; i < numeroTotalArmas; i++)
		current.armas[i].tipo = VetorArmas[i].tipo;


	imp_seq_n_base_m(&current, numeroTotalArmas, numeroAlvos);
}

void algoritmoGenetico() {
	clock_t tempo_inicio;
	clock_t tempo_fim;
	clock_t tempo_current;

	if (criterioParada == 1) {
		for (iteracao = 0; iteracao < maxIteracoes; iteracao++) {
			crossover();
			mutacao();
			avaliar_populacao();
		}
	}
	else if (criterioParada == 2) {
		tempo_inicio = clock();
		tempo_fim = tempo_inicio + tempoLimite * 1000;
		tempo_current = clock();
		while (tempo_current < tempo_fim) {
			crossover();
			mutacao();
			avaliar_populacao();
			tempo_current = clock();
		}
	}
}

void algoritmoGeneticoComBuscalocal() {
	clock_t tempo_inicio;
	clock_t tempo_fim;
	clock_t tempo_current;
	if (criterioParada == 1) {
		for (iteracao = 0; iteracao < maxIteracoes; iteracao++) {
			crossover();
			mutacao();
			avaliar_populacao();
			if (iteracao > pontoBuscaLocal)
				buscal_local();
			ordenarPopulacao();
		}
	}
	else if (criterioParada == 2) {
		tempo_inicio = clock();
		tempo_fim = tempo_inicio + tempoLimite * 1000;
		tempo_current = clock();
		while (tempo_current < tempo_fim) {
			crossover();
			mutacao();
			avaliar_populacao();
			buscal_local();
			ordenarPopulacao();
			tempo_current = clock();
		}
	}
}

void gerarInstancia() {
	srand((unsigned)time(NULL));
	FILE *out_file;

	int numeroTotalArmas;
	int numeroAlvos;
	int i;

	system(CLEAR_CONSOLE);




	do {
		system(CLEAR_CONSOLE);
		printf("Numero de alvos: ");
		scanf("%d", &numeroAlvos);
	} while (numeroAlvos < 0);

	do {
		system(CLEAR_CONSOLE);
		printf("Numero de armas: ");
		scanf("%d", &numeroTotalArmas);
	} while (numeroTotalArmas < 0);



	out_file = fopen("Instancia.txt", "w");


	fprintf(out_file, "%d\n", numeroTotalArmas);
	for (i = 0; i < numeroTotalArmas; i++) {
		fprintf(out_file, "1\n");
	}
	fprintf(out_file, "%d\n", numeroAlvos);
	for (i = 0; i < numeroAlvos; i++) {
		fprintf(out_file, "%d\n", rand() % 100);
	}
	for (i = 0; i < numeroTotalArmas * numeroAlvos; i++) {
		fprintf(out_file, "%f\n", (float)rand() / (float)(RAND_MAX));
	}

	fclose(out_file);

}

void gerarParam() {
	srand((unsigned)time(NULL));
	FILE *out_file;

	int maxIteracoes;
	int nPopulacao;
	int taxaMutacao;
	int pontoBuscaLocal;


	do {
		system(CLEAR_CONSOLE);
		printf("Numero de iteracoes sem melhora: ");
		scanf("%d", &maxIteracoes);
	} while (numeroAlvos < 0);

	do {
		system(CLEAR_CONSOLE);
		printf("Numero de individuos na populacao: ");
		scanf("%d", &nPopulacao);
	} while (numeroTotalArmas < 0);
	do {
		system(CLEAR_CONSOLE);
		printf("Taxa mutacao: ");
		scanf("%d", &taxaMutacao);
	} while (numeroAlvos < 0);

	do {
		system(CLEAR_CONSOLE);
		printf("Ponto busca local(0-100): ");
		scanf("%d", &pontoBuscaLocal);
	} while (numeroTotalArmas < 0);


	out_file = fopen("Parametros.txt", "w");



	fprintf(out_file, "%d\n", maxIteracoes);
	fprintf(out_file, "%d\n", nPopulacao);
	fprintf(out_file, "%d\n", taxaMutacao);
	fprintf(out_file, "%d\n", pontoBuscaLocal);

	fclose(out_file);

}

int main()
{
	int i;
	int mode;
	int msec;



	srand((unsigned)time(NULL));
	clock_t tempo_inicio;
	clock_t tempo_fim;
	clock_t tempo_exaust;
	clock_t tempo_metaheuristica;

	FILE *out_file;
	FILE *in_file;
	FILE *param_file;

	melhorSolucao.fitness = 9999999;



	do {
		system(CLEAR_CONSOLE);
		printf("1- Metodo exaustivo\n2- Metodo meta-heuristica\n3- Metodo meta-heuristica com busca local\n4- Benchmark heuristica sem busca local\n5- Benchmark heuristica com busca local\n6- Gerar arquivo instancia\n7- Gerar arquivo parametro\nOpcao: ");
		scanf("%d", &mode);
	} while (mode < 1 || mode > 7);


	//EXAUSTIVO

	if (mode == 1) {
		in_file = fopen("Instancia.txt", "r");
		if (in_file == 0) {
			printf("Arquivos de instancia inexistente.\n");
			return 0;
		}
		out_file = fopen("Resultados.txt", "a+");

		start(in_file);

		printf("Rodando exaustivo...\n");
		tempo_inicio = clock();

		exaustivo();

		tempo_fim = clock();
		tempo_exaust = tempo_fim - tempo_inicio;

		fprintf(out_file, "Exaustivo:\n%f\n", melhorSolucao.fitness);
		for (i = 0; i < numeroTotalArmas; i++)
			fprintf(out_file, "%d ", melhorSolucao.armas[i].alvo.tipo);

		msec = tempo_exaust * 1000 / CLOCKS_PER_SEC;
		fprintf(out_file, "\nTempo de execucao: %d segundos %d milisegundos\n\n", msec / 1000, msec % 1000);

		fclose(in_file);
		fclose(out_file);
	}
	///////////////////////////////////////


	//HEURISTICAS
	if (mode == 2 || mode == 3) {
		in_file = fopen("Instancia.txt", "r");
		param_file = fopen("Parametros.txt", "r");
		if (in_file == 0 && param_file != 0) {
			printf("Arquivos de instancia inexistente.\n");
			return 0;
		}
		else if (in_file != 0 && param_file == 0) {
			printf("Arquivos de parametro inexistente.\n");
			return 0;
		}
		else if (in_file == 0 && param_file == 0) {
			printf("Arquivos de instancia e parametros inexistentes.\n");
			return 0;
		}
		out_file = fopen("Resultados.txt", "a+");

		start(in_file);



		fscanf(param_file, "%d", &maxIteracoes);
		fscanf(param_file, "%d", &nPopulacao);
		fscanf(param_file, "%d", &taxaMutacao);
		fscanf(param_file, "%d", &pontoBuscaLocal);

		pontoBuscaLocal = pontoBuscaLocal / 100 * maxIteracoes;


		do {
			system(CLEAR_CONSOLE);
			printf("Criterio de parada:\n1- Iteracoes sem melhora\n2- Tempo limite de execucao\nOpcao: ");
			scanf("%d", &criterioParada);
		} while (criterioParada < 1 || criterioParada > 2);

		if (criterioParada == 2) {
			do {
				system(CLEAR_CONSOLE);
				printf("Digita o tempo limite em segundos: ");
				scanf("%d", &tempoLimite);
			} while (tempoLimite < 0);
			pontoBuscaLocal = 0;
		}
		fprintf(out_file, "Para %d armas e %d alvos:\n", numeroTotalArmas, numeroAlvos);


		system(CLEAR_CONSOLE);

		fprintf(out_file, "Benchmark\n");



		printf("Rodando meta-heuristica...\n");
		tempo_inicio = clock();
		iniciarpopulacao();


		if (mode == 2) {
			algoritmoGenetico();
		}
		else {
			algoritmoGeneticoComBuscalocal();
		}


		tempo_fim = clock();
		tempo_metaheuristica = tempo_fim - tempo_inicio;

		if (mode == 2)
			fprintf(out_file, "Meta-heuristica:\n%f\n", melhorSolucao.fitness);
		else
			fprintf(out_file, "Meta-heuristica com busca local:\n%f\n", melhorSolucao.fitness);
		for (i = 0; i < numeroTotalArmas; i++)
			fprintf(out_file, "%d ", melhorSolucao.armas[i].alvo.tipo);

		msec = tempo_metaheuristica * 1000 / CLOCKS_PER_SEC;
		fprintf(out_file, "\nTempo de execucao: %d segundos %d milisegundos\n\n", msec / 1000, msec % 1000);

		fclose(in_file);
		fclose(out_file);
		fclose(param_file);
	}
	//////////////////////////////////////



	//BENCHMARK
	if (mode == 4 || mode == 5) {
		in_file = fopen("Instancia.txt", "r");
		param_file = fopen("Parametros.txt", "r");
		if (in_file == 0 && param_file != 0) {
			printf("Arquivos de instancia inexistente.\n");
			return 0;
		}
		else if (in_file != 0 && param_file == 0) {
			printf("Arquivos de parametro inexistente.\n");
			return 0;
		}
		else if (in_file == 0 && param_file == 0) {
			printf("Arquivos de instancia e parametros inexistentes.\n");
			return 0;
		}
		out_file = fopen("Resultados.txt", "a+");

		start(in_file);

		int iteracoesBench;
		float mediaMeta = 0;
		float mediaTimeMeta = 0;

		fscanf(param_file, "%d", &maxIteracoes);
		fscanf(param_file, "%d", &nPopulacao);
		fscanf(param_file, "%d", &taxaMutacao);
		fscanf(param_file, "%d", &pontoBuscaLocal);

		pontoBuscaLocal = pontoBuscaLocal / 100 * maxIteracoes;


		do {
			system(CLEAR_CONSOLE);
			printf("Criterio de parada:\n1- Iteracoes sem melhora\n2- Tempo limite de execucao\nOpcao: ");
			scanf("%d", &criterioParada);
		} while (criterioParada < 1 || criterioParada > 2);

		if (criterioParada == 2) {
			do {
				system(CLEAR_CONSOLE);
				printf("Digita o tempo limite em segundos: ");
				scanf("%d", &tempoLimite);
			} while (tempoLimite < 0);
			pontoBuscaLocal = 0;
		}
		fprintf(out_file, "Para %d armas e %d alvos:\n", numeroTotalArmas, numeroAlvos);


		system(CLEAR_CONSOLE);
		printf("Digite o numero de testes: ");
		scanf("%d", &iteracoesBench);

		fprintf(out_file, "Benchmark\n");


		if (mode == 4) {
			printf("Heuristica sem busca local\n");
			fprintf(out_file, "\n\nHeuristica sem busca local:\n");
			for (i = 0; i < iteracoesBench; i++) {
				printf("Tentativa %d...\n", i + 1);
				melhorSolucao.fitness = 9999999;
				tempo_inicio = clock();

				iniciarpopulacao();
				algoritmoGenetico();

				tempo_fim = clock();
				tempo_metaheuristica = tempo_fim - tempo_inicio;

				mediaMeta += melhorSolucao.fitness;
				mediaTimeMeta += tempo_metaheuristica;

				fprintf(out_file, "%f\n", melhorSolucao.fitness);
				msec = tempo_metaheuristica * 1000 / CLOCKS_PER_SEC;
				fprintf(out_file, "%d segundos %d milisegundos\n\n", msec / 1000, msec % 1000);
			}
			mediaMeta /= iteracoesBench;
			mediaTimeMeta /= iteracoesBench;

			fprintf(out_file, "Media: %f\n", mediaMeta);

			msec = mediaTimeMeta * 1000 / CLOCKS_PER_SEC;
			fprintf(out_file, "Tempo medio: %d segundos %d milisegundos\n", msec / 1000, msec % 1000);
		}
		else if(mode == 5){
			printf("Heuristica com busca local\n");
			fprintf(out_file, "\n\nMeta-heuristica com busca local:\n");
			for (i = 0; i < iteracoesBench; i++) {
				printf("Tentativa %d...\n", i + 1);
				melhorSolucao.fitness = 9999999;
				tempo_inicio = clock();

				iniciarpopulacao();
				algoritmoGeneticoComBuscalocal();

				tempo_fim = clock();
				tempo_metaheuristica = tempo_fim - tempo_inicio;

				mediaMeta += melhorSolucao.fitness;
				mediaTimeMeta += tempo_metaheuristica;

				fprintf(out_file, "%f\n", melhorSolucao.fitness);
				msec = tempo_metaheuristica * 1000 / CLOCKS_PER_SEC;
				fprintf(out_file, "%d segundos %d milisegundos\n\n", msec / 1000, msec % 1000);
			}
			mediaMeta /= iteracoesBench;
			mediaTimeMeta /= iteracoesBench;

			fprintf(out_file, "Media: %f\n", mediaMeta);

			msec = mediaTimeMeta * 1000 / CLOCKS_PER_SEC;
			fprintf(out_file, "Tempo medio: %d segundos %d milisegundos\n\n", msec / 1000, msec % 1000);
		}
		fprintf(out_file, "//////////////////////////////////////////////////\n\n");


		fclose(in_file);
		fclose(out_file);
		fclose(param_file);
	}




	//GERADORES DO ARQUIVO DE INSTANCIAS E PARAMETROS
	if (mode == 6) {
		gerarInstancia();
	}

	if (mode == 7) {
		gerarParam();
	}
	///////////////////////////////////////


	printf("Terminou.\n");


	return 0;
}

//OBS: system(CLEAR_CONSOLE) NO WINDOWS > system("clear"); NO LINUX
//	   system("pause") NO WINDOWS > getchar(); NO LINUX
