/**
 * daemon.c
 *
 * Licença: GPLv3
 * Autores: Higor Euripedes
 *          Francilene Coelho
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "lista.h"
#include "contato.h"
#include "comum.h"

Lista *contatos = NULL;
int numcontatos = 0;
int infd, outfd;
int trava;

void terminar ()/*{{{*/
{
	printf("INFO: Finalizando daemon.\n");

	if (contatos)
		lista_liberar(contatos);

	close(infd);
	close(outfd);
	close(trava);

	unlink(TRAVA);
	unlink(PIPEIN);
	unlink(PIPEOUT);
	exit(EXIT_SUCCESS);
}/*}}}*/
void salvar ()/*{{{*/
{
	Lista *i;
	FILE *fp;

	if (!(fp = fopen(BANCO, "w")))
		return;
	
	printf("INFO: Salvando banco de dados.\n");

	for (i = contatos; i; i = i->prox) {
		Contato *c = (Contato *)i->dado;
		fprintf(fp, "%s\t%s\t%s\t%s\n", c->nome, c->endereco, c->email, c->aniversario);
	}

	printf("INFO: Banco de dados salvo.\n");

	fclose(fp);
}/*}}}*/
Contato* novo_contato (int num, char *nome, char *endereco, char *email, char *aniversario)/*{{{*/
{
	Contato *c = contato_criar(num, nome, endereco, email, aniversario);
	contatos = lista_adicionar(contatos, (void *)c);
	return c;
}/*}}}*/
void ler ()/*{{{*/
{
	int ret;
	FILE *fp;
	char nome[CONTATO_MAX_NOME + 1];
	char endereco[CONTATO_MAX_END + 1];
	char email[CONTATO_MAX_EMAIL + 1];
	char aniversario[6];

	if (!(fp = fopen(BANCO, "r"))) {
		printf("AVISO: Não foi possivel abrir o arquivo do banco de dados.\n");
		return;
	}

	char *format = "%[^\t]\t%[^\t]\t%[^\t]\t%[^\n]\n";

	lista_liberar(contatos);
	contatos = NULL;
	numcontatos = 0;

	printf("INFO: Atualizando banco de dados.\n");

	ret = fscanf(fp, format, nome, endereco, email, aniversario);
	if (ret != 4) {
		printf("AVISO: Não foi possivel ler o arquivo do banco de dados.\n");
		fclose(fp);
		return;
	}

	do {
		novo_contato(++numcontatos, nome, endereco, email, aniversario );
		ret = fscanf(fp, format, nome, endereco, email, aniversario);
	} while (ret == 4);

	printf("INFO: Atualização terminada.\n");

	fclose(fp);
}/*}}}*/
void gerenciador_de_sinais (int sinal)/*{{{*/
{
	printf("INFO: Sinal '%i' recebido.\n", sinal);

	/*
	 * SIGHUP: atualizar banco de dados.
	 * SIGINT,SIGTERM: liberar recursos e finalizar.
	 * SIGKILL: abortar execução.
	 */
	switch (sinal)
	{
		case SIGHUP:
			ler();
			break;
		case SIGINT:
		case SIGTERM:
			terminar();
			break;
		case SIGKILL:
			abort();
	}
}/*}}}*/
void daemonizar ()/*{{{*/
{
	pid_t pid;
	int i;

	pid = fork();

	if (pid < 0) {
		printf("ERRO: Não foi possivel bifurcar.\n");
		exit(EXIT_FAILURE);
	}

	if (pid > 0)
		exit(EXIT_SUCCESS); // pai
	
	printf("INFO: Bifurcando... PID: %d\n", getpid());

	// uma trava previne contra multiplas instancias do daemon.
	trava = open(TRAVA, O_WRONLY|O_CREAT|O_EXCL, 0640);

	if (trava < 0) {
		printf("ERRO: Não foi possivel criar o arquivo de trava '%s'. Provavelmente ele já existe.\n", TRAVA);
		exit(EXIT_FAILURE); // não foi possivel criar o arquivo de trava.
	}
	
	if (lockf(trava, F_TLOCK, 0) != 0) {
		printf("INFO: Parece que ja existe uma isntancia do daemon. Finalizando.\n");
		exit(EXIT_SUCCESS); // possivelmente já existe uma instancia do daemon
	}

	char result[100] = {0};
	sprintf(result, "%d", getpid());
	write(trava, result, strlen(result)); // grava pid da instancia atual
	
	// o daemon precisa ser idependente de outros processos
	setsid();  // obter novo grupo de processos.

	// o daemon precisa ser desligado dos descritores padrão (entrada/saida)
	// que foram herdados do processo pai.
	for (i = getdtablesize(); i >= 0; --i)
		close(i);

	// std(in/out/err) precisam existir para evitar certos erros
	// basta abrir um descritor e clona-lo
	i = open("/dev/null", O_RDWR); // stdin
	open("DAEMON.log", O_WRONLY|O_CREAT|O_APPEND, 0640); // stdout é o arquivo de log
	dup(i); // stderr
	
	// privilegio de criação de arquivos
	umask(027); // 027 é o complemento de 750

	//chdir("/tmp");

	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGINT,  gerenciador_de_sinais);
	signal(SIGHUP,  gerenciador_de_sinais);
	signal(SIGTERM, gerenciador_de_sinais);
	signal(SIGKILL, gerenciador_de_sinais);

	printf("=== NOVA INSTANCIA: %i ===\n", getpid());

	// cria os pipes de etnrada e saida 
	if (mkfifo(PIPEIN, 0666) == -1) {
		printf("ERRO: Não foi possivel criar o pipe de entrada.\n");
		exit(EXIT_FAILURE);
	}
	if (mkfifo(PIPEOUT, 0666) == -1) {
		printf("ERRO: Não foi possivel criar o pipe de saida.\n");
		exit(EXIT_FAILURE);
	}

	infd  = open(PIPEIN, O_RDWR);
	outfd = open(PIPEOUT, O_RDWR);

	if (infd < 0) {
		printf("ERRO: Não foi possivel abrir o pipe de entrada.\n");
		exit(EXIT_FAILURE);
	}
	if (outfd < 0) {
		printf("ERRO: Não foi possivel abrir o pipe de saida.\n");
		exit(EXIT_FAILURE);
	}

}/*}}}*/
void enviar (char *buffer)/*{{{*/
{
	write(outfd, buffer, strlen(buffer));
	printf("INFO: Enviado %s\n", buffer);
}/*}}}*/
int receber (char *buffer)/*{{{*/
{
	int numread;

	numread = read(infd, buffer, MAXBUF);
	if (numread) {
		printf("INFO: Recebido %s\n", buffer);
	}

	return numread;
}/*}}}*/
void incluir (char *nome, char *endereco, char *email, char *aniversario)/*{{{*/
{
	printf("INFO: Inserindo contato '%s'...\n", nome);
	novo_contato(++numcontatos, nome, endereco, email, aniversario);
}/*}}}*/
void alterar (int codigo, int opcao, char *valor)/*{{{*/
{
	Lista *l;
	
	printf("INFO: Alterando o contato #%i..\n", codigo);

	for (l= contatos; l; l = l->prox) {
		Contato *c = (void*)l->dado;
		if (c->numero == codigo) {
			if (!opcao)
				break;

			switch(opcao)
			{
				case 1:
					strncpy(c->nome, valor, CONTATO_MAX_NOME);
					break;
				case 2:
					strncpy(c->aniversario, valor, 6);
					break;
				case 3:
					strncpy(c->endereco, valor, CONTATO_MAX_END);
					break;
				case 4:
					strncpy(c->email, valor, CONTATO_MAX_EMAIL);
					break;
			}
			break;
		}
	}
}/*}}}*/
void deletar (int codigo)/*{{{*/
{
	Lista *l = NULL, *j = NULL;

	printf("INFO: Deletando o contato #%i..\n", codigo);

	for (l = contatos; l; l = l->prox) {
		if (((Contato *)l->dado)->numero == codigo) {
			if (j) {
				j->prox = l->prox;
			}
			if (contatos == l && !j)
				contatos = l->prox;
			else if (contatos == l && j)
				contatos = j;

			free(l->dado);
			free(l);
			break;
		}
		j = l;
	}
}/*}}}*/
void buscar (char *data)/*{{{*/
{
	char buf[1024] = {0};
	int enviou = 0;
	const char *formato =
		"%03i - %s\n"
		"\tAniversario.: %s\n"
		"\tEmail.......: %s\n"
		"\tEndereco....: %s\n";

	printf("INFO: Buscando por '%s'...\n", data);
	
	Lista *l = NULL;
	for (l = contatos; l; l = l->prox) {
		Contato *c = (Contato *) l->dado;
		char *p1;
		char *p2;
		p2 = strchr(c->aniversario, '/');

		if (!p2)
			continue;

		p1 = strstr(p2, data);

		if (p1 && sprintf(buf, formato, c->numero, c->aniversario, c->nome, c->endereco)) {
			enviar(buf);
			enviou = 1;
		}
	}
	if (!enviou)
		enviar("nenhum contato encontrado\n");
}/*}}}*/
void listar ()/*{{{*/
{
	Lista *l;

	printf("INFO: Listando contatos...\n");

	ler();
	char buf[1024] = {0};
	int enviou = 0;
	const char *formato =
		"%03i - %s\n"
		"\tAniversario.: %s\n"
		"\tEmail.......: %s\n"
		"\tEndereco....: %s\n";
	for (l = contatos; l; l = l->prox) {
		Contato *c = (Contato *)l->dado;
		if (sprintf(buf, formato, c->numero, c->nome, c->aniversario, c->email, c->endereco)) {
			enviar(buf);
			enviou = 1;
		}
	}
	if (!enviou)
		enviar("nenhum contato encontrado\n");
}/*}}}*/

int main (int argc, char **argv)
{
	daemonizar();

	char pipebuf[MAXBUF] = {0};
	int numread = 0;


	while (1) {
		char nome[CONTATO_MAX_NOME + 1];
		char endereco[CONTATO_MAX_END + 1];
		char email[CONTATO_MAX_EMAIL + 1];
		char aniversario[6];
		int cod = 0, opt = 0;
		char valor[255] = {0};

		sleep(1);

		memset(pipebuf, 0, MAXBUF);

		numread = receber(pipebuf);
		pipebuf[numread] = 0;

		if (numread < 1) {
			continue;
		}

		switch (pipebuf[0]) {
			case '9':
				terminar();
				break;
			case '1':
				;
				sscanf(pipebuf, "%*c|%[^|]|%[^|]|%[^|]|%[^$]$$", nome, endereco, email, aniversario);
				incluir(nome, endereco, email, aniversario);
				break;
			case '2':
				;
				sscanf(pipebuf, "%*c|%i|%i|%[^$]$$", &cod, &opt, valor);
				alterar(cod, opt, valor);
				break;
			case '3':
				;
				sscanf(pipebuf, "%*c|%i$$", &cod);
				deletar(cod);
				break;
			case '4': listar(); break;
			case '5': 
				;
				sscanf(pipebuf, "%*c|%[^$]$$", aniversario);
				buscar(aniversario);
				break;
		}
		if (pipebuf[0] < '4')
			salvar();
	}

	return 0;
}
