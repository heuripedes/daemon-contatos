/**
 * lista.h
 *
 * Licença: GPLv3
 * Autores: Higor Euripedes
 *          Francilene Coelho
 *
 */
#ifndef LISTA_H
#define LISTA_H

#define TIPO_LISTA void *

typedef struct lista {
	struct lista* prox;
	TIPO_LISTA *dado;
} Lista;

Lista *lista_adicionar (Lista *lista, TIPO_LISTA dado);
Lista *lista_remover (Lista *lista, TIPO_LISTA *dado);
void lista_liberar (Lista *lista);

#endif
