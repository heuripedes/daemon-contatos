/**
 * contato.h
 *
 * Licensa: GPLv3
 * Autores: Higor Euripedes
 *          Francilene Coelho
 *
 */
#ifndef CONTATO_H
#define CONTATO_H

#define CONTATO_MAX_NOME  50
#define CONTATO_MAX_END   50
#define CONTATO_MAX_EMAIL 30

typedef struct contato {
	char nome[CONTATO_MAX_NOME + 1];
	char endereco[CONTATO_MAX_END + 1];
	char email[CONTATO_MAX_EMAIL + 1];
	char aniversario[6];
	int numero;
} Contato;

Contato *contato_criar (int num, char *nome, char *endereco, char *email, char *aniversario);
Lista *contato_ordem_alfabetica (Lista *lista);

#endif

