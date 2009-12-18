/**
 * cliente.c
 *
 * Licensa: GPLv3
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
#include "lista.h"
#include "contato.h"
#include "comum.h"

int infd, outfd;

void enviar (char *buffer)/*{{{*/
{
	write(outfd, buffer, strlen(buffer));
}/*}}}*/
int receber (char *buffer)/*{{{*/
{
	return read(infd, buffer, MAXBUF);
}/*}}}*/
void incluir ()/*{{{*/
{
	char nome[CONTATO_MAX_NOME + 1];
	char endereco[CONTATO_MAX_END + 1];
	char email[CONTATO_MAX_EMAIL + 1];
	char aniversario[6];

	printf("\n###--- Incluir contato ---###\n\n");
	printf("Informe os dados a seguir:\n");

	printf("Nome: ");
	fgets(nome, CONTATO_MAX_NOME, stdin);

	printf("Endereco: ");
	fgets(endereco, CONTATO_MAX_END, stdin);

	printf("Email: ");
	fgets(email, CONTATO_MAX_EMAIL, stdin);

	fflush(stdin);	
	printf("Anversario (dd/mm, ex: 31/09): ");
	fgets(aniversario, 6, stdin);

	if(nome[strlen(nome)-1] < ' ')
		nome[strlen(nome)-1] = 0;

	if(endereco[strlen(endereco)-1] < ' ')
		endereco[strlen(endereco)-1] = 0;

	if(email[strlen(email)-1] < ' ')
		email[strlen(email)-1] = 0;

	if(aniversario[strlen(aniversario)-1] < ' ')
		aniversario[strlen(aniversario)-1] = 0;

	char buf[1024] = {0};
	if (sprintf(buf, "1|%s|%s|%s|%s$$", nome, endereco, email, aniversario))
		enviar(buf);
}/*}}}*/
void alterar ()/*{{{*/
{
	int codigo;
	int opcao;
	char valor[255] = {0};
	printf("\n###--- Alterar contato ---###\n\n");
	printf("Informe o código do contato: ");
	scanf("%i", &codigo);
	getchar();
	
	printf("O que deseja alterar (0=Nada,1=Nome,2=Aniversario,3=Endereco,4=Email)? ");
	scanf("%i", &opcao);
	getchar();

	if (!opcao)
		return;

	switch(opcao)
	{
		case 1:
			;
			printf("Novo nome: ");
			fgets(valor, CONTATO_MAX_NOME, stdin);
			break;
		case 2:
			;
			printf("Novo aniversario (dd/mm, ex: 03/12): ");
			fgets(valor, 6, stdin);
			break;
		case 3:
			;
			printf("Novo endereco: ");
			fgets(valor, CONTATO_MAX_END, stdin);
			break;
		case 4:
			;
			printf("Novo email: ");
			fgets(valor, CONTATO_MAX_EMAIL, stdin);
			break;
	}

	if(valor[strlen(valor)-1] < ' ')
		valor[strlen(valor)-1] = 0;
	
	char buf[1024] = {0};
	if (sprintf(buf, "2|%i|%i|%s$$", codigo, opcao, valor))
		enviar(buf);
}/*}}}*/
void deletar ()/*{{{*/
{
	int codigo;
	printf("\n###--- Deletar contato ---###\n\n");
	printf("Informe o código do contato: ");
	scanf("%i", &codigo);
	getchar();

	char buf[20] = {0};
	if (sprintf(buf, "3|%i$$", codigo))
		enviar(buf);
}/*}}}*/
void listar ()/*{{{*/
{
	char buf[1024] = {0};
	enviar("4$$");
		
	printf("\n###--- Listagem alfabética ---###\n\n");

	receber(buf);
	printf("%s\n", buf);
	getchar();
}/*}}}*/
void buscar ()/*{{{*/
{
	char data[6];
	printf("-- Menu > Buscar --\n\n");
	printf("Informe o mes (dd, ex: 09): ");
	fgets(data, 3, stdin);

	if(data[strlen(data)-1] < ' ')
		data[strlen(data)-1] = 0;

	char buf[1024] = {0};
	if (sprintf(buf, "5|%s$$", data))
		enviar(buf);

	receber(buf);
	printf(buf);
}/*}}}*/

int main (int argc, char **argv)
{
	int option = -1;
	infd  = open(PIPEOUT, O_RDWR);
	outfd = open(PIPEIN, O_RDWR);

	system("clear");

	while (option) {
		printf("\n###--- Menu ---###\n");
		printf("1 - Incluir\n");
		printf("2 - Alterar\n");
		printf("3 - Deletar\n");
		printf("4 - Listagem alfabetica\n");
		printf("5 - Busca por aniversariantes\n");
		printf("9 - Finalizar daemon\n");
		printf("0 - Sair\n\nOpção: ");
		scanf("%i", &option);
		getchar();
		
		system("clear");

		switch (option) {
			case 1: incluir(); break;
			case 2: alterar(); break;
			case 3: deletar(); break;
			case 4: listar(); break;
			case 5: buscar(); break;
			case 9: enviar("9"); break;
		}
	}

	return 0;
}
