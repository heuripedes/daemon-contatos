/*
 * contato.c
 *
 * Licença: GPLv3
 * Autores: Higor Euripedes
 *          Francilene Coelho
 *
 */

#include <stdlib.h>
#include <string.h>
#include "lista.h"
#include "contato.h"

Contato *contato_criar (int num, char *nome, char *endereco, char *email, char *aniversario)
{
	Contato *n = (Contato *)calloc(sizeof(Contato), 1);
	n->numero = num;
	strncpy(n->nome,        nome,        CONTATO_MAX_NOME);
	strncpy(n->endereco,    endereco,   CONTATO_MAX_END);
	strncpy(n->email,       email,       CONTATO_MAX_EMAIL);
	strncpy(n->aniversario, aniversario, 6);

	return n;
}

Lista *contato_ordem_alfabetica (Lista *lista)
{
	Lista *l, *j;

	for (l = lista; l->prox; l = l->prox) {
		for (j = l->prox; j; j = j->prox) {
			Contato *a = (Contato *)l->dado, *b = (Contato *)j->dado;
			if (strcmp(a->nome, b->nome) > 0) {
				l->dado = (void*)b;
				j->dado = (void*)a;
			}
		}
	}
	return lista;
}

