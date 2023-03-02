/**
 *  @brief		link list API for timer management.
 *  @details	link list API for timer management.
 *  @author
	- unknown(porting from ZyNOS)
 *  @copyright
.


 *  Copyright (C) 2013 ZyXEL Communications, Corp.
 *  All Rights Reserved.
 *
 *  ZyXEL Confidential;
 *  Protected as an unpublished work, treated as confidential,
 *  and hold in trust and in strict confidence by receiving party.
 *  Only the employees who need to know such ZyXEL confidential information
 *  to carry out the purpose granted under NDA are allowed to access.
 *
 *  The computer program listings, specifications and documentation
 *  herein are the property of ZyXEL Communications, Corp. and shall
 *  not be reproduced, copied, disclosed, or used in whole or in part
 *  for any reason without the prior express written permission of
 *  ZyXEL Communications, Corp.
 *  @file				timer_list.c
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "timer_list.h"

#include "zos_api.h"
#include "zlog_api.h"

int timer_list_init (timer_list_t * li)
{
  if (li==NULL)
    return -1;
  memset(li, 0, sizeof(timer_list_t));
  return 0;			/* ok */
}

void
timer_list_special_free (timer_list_t * li, void *(*free_func) (void *))
{
  int pos = 0;
  void *element;

  if (li == NULL)
    return;
  while (!timer_list_eol (li, pos))
    {
      element = (void *) timer_list_get (li, pos);
      timer_list_remove (li, pos);
      if (free_func!=NULL)
	free_func (element);
    }
//  timer_free (li);
}

void
timer_list_ofchar_free (timer_list_t * li)
{
  int pos = 0;
  char *chain;

  if (li == NULL)
    return;
  while (!timer_list_eol (li, pos))
    {
      chain = (char *) timer_list_get (li, pos);
      timer_list_remove (li, pos);
      timer_free (chain);
    }
  timer_free (li);
}

int
timer_list_size (const timer_list_t * li)
{
  /*
     Robin Nayathodan <roooot@softhome.net>
     N.K Electronics INDIA

     NULL Checks
   */

  if (li != NULL)
    return li->nb_elt;
  else
    return -1;
}

int
timer_list_eol (const timer_list_t * li, int i)
{
  if(li==NULL) return -1;
  if (i < li->nb_elt)
    return 0;			/* not end of list */
  return 1;			/* end of list */
}

/* index starts from 0; */
int
timer_list_add (timer_list_t * li, void *el, int pos)
{
  __node_t *ntmp;
  int i = 0;

  if (li == NULL) return -1;

  if (li->nb_elt == 0)
    {

      li->node = (__node_t *) timer_malloc (sizeof (__node_t));
      if (li->node == NULL) return -1;
      li->node->element = el;
      li->node->next = NULL;
      li->nb_elt++;
      return li->nb_elt;
    }

  if (pos == -1 || pos >= li->nb_elt)
    {				/* insert at the end  */
      pos = li->nb_elt;
    }

  ntmp = li->node;		/* exist because nb_elt>0  */

  if (pos == 0) /* pos = 0 insert before first elt  */
    {
      li->node = (__node_t *) timer_malloc (sizeof (__node_t));
      if (li->node == NULL)
	{
	  /* leave the list unchanged */
	  li->node=ntmp;
	  return -1;
	}
      li->node->element = el;
      li->node->next = ntmp;
      li->nb_elt++;
      return li->nb_elt;
    }


  while (pos > i + 1)
    {
      i++;
      /* when pos>i next node exist  */
      ntmp = (__node_t *) ntmp->next;
    }

  /* if pos==nb_elt next node does not exist  */
  if (pos == li->nb_elt)
    {
      ntmp->next = (__node_t *) timer_malloc (sizeof (__node_t));
      if (ntmp->next == NULL) return -1; /* leave the list unchanged */
      ntmp = (__node_t *) ntmp->next;
      ntmp->element = el;
      ntmp->next = NULL;
      li->nb_elt++;
      return li->nb_elt;
    }

  /* here pos==i so next node is where we want to insert new node */
  {
    __node_t *nextnode = (__node_t *) ntmp->next;

    ntmp->next = (__node_t *) timer_malloc (sizeof (__node_t));
    if (ntmp->next ==  NULL) {
      /* leave the list unchanged */
      ntmp->next=nextnode;
      return -1;
    }
    ntmp = (__node_t *) ntmp->next;
    ntmp->element = el;
    ntmp->next = nextnode;
    li->nb_elt++;
  }
  return li->nb_elt;
}

/* index starts from 0 */
void *
timer_list_get (const timer_list_t * li, int pos)
{
  __node_t *ntmp;
  int i = 0;

  if (li == NULL) return NULL;

  if (pos < 0 || pos >= li->nb_elt)
    /* element does not exist */
    return NULL;


  ntmp = li->node;		/* exist because nb_elt>0 */

  while (pos > i)
    {
      i++;
      ntmp = (__node_t *) ntmp->next;
    }
  return ntmp->element;
}

/* added by bennewit@cs.tu-berlin.de */
void * timer_list_get_first( timer_list_t * li, timer_list_iterator_t * iterator )
{
  if ( 0 >= li->nb_elt) {
    iterator->actual = 0;
    return 0;
  }

  iterator->actual = li->node;
  iterator->prev = &li->node;
  iterator->li = li;
  iterator->pos = 0;

  return li->node->element;
}

/* added by bennewit@cs.tu-berlin.de */
void * timer_list_get_next( timer_list_iterator_t * iterator )
{
  iterator->prev = (__node_t**)&(iterator->actual->next);
  iterator->actual = iterator->actual->next;
  ++(iterator->pos);

  if ( timer_list_iterator_has_elem( *iterator ) ) {
    return iterator->actual->element;
  }

  iterator->actual = 0;
  return 0;
}

/* added by bennewit@cs.tu-berlin.de */
void * timer_list_iterator_remove( timer_list_iterator_t * iterator )
{
  if ( timer_list_iterator_has_elem( *iterator ) ) {
    --(iterator->li->nb_elt);

    *(iterator->prev) = iterator->actual->next;

    timer_free( iterator->actual );
    iterator->actual = *(iterator->prev);
  }

  if ( timer_list_iterator_has_elem( *iterator ) ) {
    return iterator->actual->element;
  }

  return 0;
}

/* return -1 if failed */
int
timer_list_remove (timer_list_t * li, int pos)
{

  __node_t *ntmp;
  int i = 0;

  if (li == NULL) return -1;

  if (pos < 0 || pos >= li->nb_elt)
    /* element does not exist */
    return -1;

  ntmp = li->node;		/* exist because nb_elt>0 */

  if ((pos == 0))
    {				/* special case  */
      li->node = (__node_t *) ntmp->next;
      li->nb_elt--;
      timer_free (ntmp);
      return li->nb_elt;
    }

  while (pos > i + 1)
    {
      i++;
      ntmp = (__node_t *) ntmp->next;
    }

  /* insert new node */
  {
    __node_t *remnode;

    remnode = (__node_t *) ntmp->next;
    ntmp->next = ((__node_t *) ntmp->next)->next;
    timer_free (remnode);
    li->nb_elt--;
  }
  return li->nb_elt;
}


void *timer_malloc (int size)
{
  void *value;

  value = ZOS_MALLOC (size);

#if 0
  if (value == 0)
  	return NULL;
#endif

  return value;
}

void timer_free (void *ptr)
{
  if (ptr != NULL)
    ZOS_FREE (ptr);
}

