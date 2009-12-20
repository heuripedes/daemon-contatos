/**
 * lista.c
 *
 * Licença: GPLv3
 * Autores: Higor Euripedes
 *          Francilene Coelho
 *
 */
#include <stdlib.h>
#include "lista.h"

Lista *lista_adicionar (Lista *lista, TIPO_LISTA dado)
{
	Lista *l = (Lista *)calloc(sizeof(Lista),1);
	l->dado = dado;
	if (lista)
		l->prox = lista;

	return l;
}

Lista *lista_remover (Lista *lista, TIPO_LISTA *dado)
{
	Lista *l = lista->prox;
	
	if (dado)
		*dado = lista->dado;
	
	free(lista);
	return l;
}

void lista_liberar (Lista *lista)
{
	Lista *l;
	while (lista) {
		l = lista->prox;
		free(lista->dado);
		free(lista);
		lista = l;
	}
}

