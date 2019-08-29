#include <xcb/xcb.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "constants.h"
#include "list.h"
#include "log.h"

void
init (list *self)
{
  pthread_mutex_init(&self->lock, NULL);
  self->len = 0;
  self->head = NULL;
  self->tail = NULL;
}

void
window_list_push (list *self, xcb_window_t w) 
{
  if(self->head == NULL) {
    self->head = (struct window_node*) malloc(sizeof(struct window_node));
    self->head->window = w;
    self->head->next = NULL;
    self->tail = self->head;
  }
  else {
    self->tail->next = (struct window_node*) malloc(sizeof(struct window_node));
    self->tail->next->window = w;
    self->tail->next->next = NULL;
    self->tail = self->tail->next;
  }
  self->len++;
}

int
window_list_remove (list *self, xcb_window_t w)
{
  if(!self->head) {
    return 1;
  }
  else if(self->head->window == w) {
    struct window_node *temp = self->head;
    self->head = self->head->next;
    free(temp);
    self->len--;
    return 0;
  }
  else {
    struct window_node *previous = self->head;
    for(struct window_node *current = self->head; current != NULL; current = current->next) {
      if (current->window == w) {
        previous->next = current->next;
        free(current);
        self->len--;
        return 0;
      }
      previous = current;
    }
    return 1;
  }
}

int
window_list_modify (list *self, xcb_window_t w, xcb_connection_t *connection, uint32_t position[]) 
{
  struct window_node *current = self->head;
  while(current != NULL) {
    if(current->window == w) {
      xcb_configure_window(connection, current->window, XCB_MOVE_RESIZE, position);
      xcb_map_window(connection, current->window);
      return 0;
    }
    else
      current = current->next;
  }
  return 1;
}

/* Note: Do not attempt to acquire a lock before destroying a list as it will deadlock. */
void
window_list_destroy (list *self)
{
  pthread_mutex_lock(&self->lock);
  while(self->head) {
    struct window_node *next = self->head->next;
    free(self->head);
    self->head = next;
  }
  pthread_mutex_unlock(&self->lock);
  pthread_mutex_destroy(&self->lock);
  self->len = 0;
}

void
window_list_map_all (list *self, xcb_connection_t *connection)
{
  struct window_node *current = self->head;
  while(current != NULL) {
    write_to_log("Mapping window\n");
    xcb_map_window(connection, current->window);
    current = current->next;
  }
}

void
window_list_unmap_all (list *self, xcb_connection_t *connection)
{
  struct window_node *current = self->head;
  while(current != NULL) {
    write_to_log("Unmapping window\n");
    xcb_unmap_window(connection, current->window);
    current = current->next;
  }
}

#ifdef DEBUG
/* Note: Cannot test map and unmap all here. 
 * Tests can be made using make list_tests
 */
int 
list_tests ()
{
  list test_list;
  pthread_mutex_lock(&test_list.lock);
  init(&test_list);
  pthread_mutex_unlock(&test_list.lock);

  xcb_window_t test;
  pthread_mutex_lock(&test_list.lock);
  window_list_push(&test_list, test);
  pthread_mutex_unlock(&test_list.lock);

  xcb_window_t test1;
  pthread_mutex_lock(&test_list.lock);
  window_list_push(&test_list, test1);
  pthread_mutex_unlock(&test_list.lock);

  xcb_window_t test2;
  pthread_mutex_lock(&test_list.lock);
  window_list_push(&test_list, test2);
  pthread_mutex_unlock(&test_list.lock);

  pthread_mutex_lock(&test_list.lock);
  printf("%d\n", window_list_remove(&test_list, test2));
  printf("%d\n", window_list_remove(&test_list, test1));
  printf("%d\n", window_list_remove(&test_list, test));
  pthread_mutex_unlock(&test_list.lock);

  window_list_destroy(&test_list);
  return 0;
}

#ifndef DEBUG_BUILD
int
main ()
{
  return list_tests();
}
#endif
#endif
