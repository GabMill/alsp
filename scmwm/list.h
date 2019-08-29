struct window_node {
  xcb_window_t window;
  struct window_node *next;
};

typedef struct {
  uint32_t len;
  struct window_node *head;
  struct window_node *tail;
  pthread_mutex_t lock;
} list;

void init (list *self);
void window_list_push (list *self, xcb_window_t w);
int window_list_remove (list *self, xcb_window_t w);
int window_list_modify (list *self, xcb_window_t w, xcb_connection_t *connection, uint32_t position[]);
void window_list_destroy (list *self);
void window_list_map_all (list *self, xcb_connection_t *connection);
void window_list_unmap_all (list *self, xcb_connection_t *connection);
