#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <string.h>

#include "constants.h"
#include "list.h"
#include "log.h"

/* Why is this a global? Globals are common design elements in window managers, which rely on manipulating global state.
 * Why is not behind a lock? Precisely 2 functions mutate it: setup() and quit(). Neither of these should happen at the same time.
 */
xcb_connection_t *connection;
list windows;

static int mod = 0;

typedef struct {
  FILE *fifo;
  int fifofd;
} ipc_info;

static int
Fork()
{
  pid_t pid;

  if ((pid = fork()) < 0) {
    write_to_log("Fork error\n");
    exit(0);
  }
  return(pid);
}

// int
// check_mod(uint32_t mask, char ***keys)
// {
//     int i, keys_pressed = 0;
//
//     const char *MODIFIERS = {
//             "Shift", "Lock", "Ctrl", "Alt",
//             "Mod2", "Mod3", "Mod4", "Mod5",
//             "Button1", "Button2", "Button3", "Button4", "Button5"
//     };
//     for (const char **modifier = MODIFIERS ; mask; mask >>= 1, ++modifier) {
//         if (mask & 1) {
//             keys_pressed++;
//             if(keys_pressed > 3)
//               continue;
//             (*keys)[i] = strdup(*modifier);
//             i++;
//             // printf (*modifier);
//         }
//     }
//     return keys_pressed;
// }

/* Thread driver
 * epoll() code taken primarily from epoll man page and rewritten to work with pipes instead of a socket. */
static void*
handle_input (void *arg)
{
#ifndef LOGGING_RELEASE
  write_to_log("Started worker thread.\n");
#endif
  char **args = NULL;
  int pid, exec;
  ipc_info *ipc = (ipc_info*) arg;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, readsz;
  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    perror("epoll error");
    exit(EXIT_FAILURE);
  }

#ifndef LOGGING_RELEASE
  write_to_log("epoll service started.\n");
#endif
  ev.events = EPOLLIN | EPOLLET;
  int server_endpoint_fd = ipc->fifofd;
  ev.data.fd = server_endpoint_fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_endpoint_fd, &ev) == -1) {
    perror("epoll error: epoll_ctl");
    exit(EXIT_FAILURE);
  }

  int minimized = 0;
  char buf[MAX_LINE_SIZE];
  for(;;) {
#ifndef LOGGING_RELEASE
    write_to_log("Begin waiting for epoll.\n");
#endif
    memset(buf, 0, MAX_LINE_SIZE);
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }
    for(int n=0; n < nfds; n++) {
      if (events[n].data.fd == server_endpoint_fd) {
        if((readsz = read(ipc->fifofd, buf, MAX_LINE_SIZE)) < 0) {
#ifndef LOGGING_RELEASE
          write_to_log("Read error.\n");
#endif
          perror("Read error");
        }
        buf[readsz-1] = 0;
        if(strcmp(buf, "minimize") == 0) {
          if(minimized == 0) {
            pthread_mutex_lock(&windows.lock);
            window_list_unmap_all(&windows, connection);
            pthread_mutex_unlock(&windows.lock);
            xcb_flush(connection);
            minimized = 1;
          } else {
            pthread_mutex_lock(&windows.lock);
            window_list_map_all(&windows, connection);
            pthread_mutex_unlock(&windows.lock);
            xcb_flush(connection);
            minimized = 0;
          }
        }
        else if(strcmp(buf, "xterm") == 0){
          pid = Fork();
          if(pid == 0) {
            if((exec = execvp("/usr/bin/xterm", args)) == -1) {
#ifndef LOGGING_RELEASE
            write_to_log("Exec error");
#endif
            exit(0);
            }
          }
        }
        else if(strcmp(buf, "chrome") == 0){
          pid = Fork();
          if(pid == 0) {
            if((exec = execvp("/usr/bin/google-chrome-stable", args)) == -1) {
#ifndef LOGGING_RELEASE
            write_to_log("Exec error");
#endif
            exit(0);
            }
          }
        }
        else {
          int window_number = 0;
          int position_x = 0;
          int position_y = 0;
          int size_x = 500;
          int size_y = 500;
          sscanf(buf, "%d %d %d %d %d", &window_number, &position_x, &position_y, &size_x, &size_y);
          pthread_mutex_lock(&windows.lock);
          /* Traverse to the window_numberth place in the window list */
          struct window_node *current = windows.head;
          for(int i=0; i < window_number && current != NULL; i++) {
            current = current->next;
          }
          if (current) {
            uint32_t target_position[] = {position_x, position_y, size_x, size_y};
            xcb_configure_window(connection, current->window, XCB_MOVE_RESIZE, target_position);
            xcb_map_window(connection, current->window);
            xcb_flush(connection);
          }
          pthread_mutex_unlock(&windows.lock);
        }
      }
    }
  }
}

/* Quit the window manager in response to a signal. */
void
quit (int signo)
{
  xcb_disconnect(connection);
}

//Key handling adapted from i3
void
handle_keypress(xcb_key_press_event_t *event)
{
  char **args = NULL;
  int pid, exec;
  static xcb_key_symbols_t *symbols;
  int col = ((event->state) & XCB_MOD_MASK_SHIFT);
  xcb_keysym_t sym;
  sym = xcb_key_press_lookup_keysym(symbols, event, col);
  //Check if mod key, if so set mod and return
  if(sym == XK_Mode_switch){
      mod = 1;
      return;
    }
    //Otherwise, check for some commands
    if(mod == 1) {
      if(sym == XK_Return) {
        pid = Fork();
        if(pid == 0) {
          if((exec = execvp("/usr/bin/xterm", args)) == -1) {
#ifndef LOGGING_RELEASE
            write_to_log("Exec error");
#endif
            exit(0);
          }
        }
      }
    }
}

void
handle_keyrelease(xcb_key_release_event_t *event)
{
   static xcb_key_symbols_t *symbols;
   xcb_keysym_t sym = xcb_key_press_lookup_keysym(symbols, event, event->state);
   //Check to see if a mod key was released and reset mod
   if(sym == XK_Mode_switch){
     mod = 0;
     return;
   }
}

void
handle_buttons (xcb_button_t detail)
{
  /* Event handler. Register in switch/case statement.
   * Idea as a way to try things: fork/exec in here on keypress?
   */
  switch (detail)
  {
    default:
      break;
  }
}

int
handle_event (xcb_generic_event_t *event)
{
  switch(event->response_type & ~0x80)
  {
    case XCB_MAP_REQUEST:
      /* What to do when windows are created
       * Note that we must cast the generic event to the correct type before using it.
       * XCB implements events as a union with a response_type attached to it. Since we know
       * the type of request, it's safe to cast.
       */
      {
#ifndef LOGGING_RELEASE
        write_to_log("Map request received\n");
#endif
        xcb_map_request_event_t *map_event = (xcb_map_request_event_t*)event;
        uint32_t default_position[] = WINDOW_DEFAULTS;
        xcb_configure_window(connection, map_event->window, XCB_MOVE_RESIZE, default_position);
        xcb_map_window(connection, map_event->window);

        /* Add to list of windows for the workspace */
        pthread_mutex_lock(&windows.lock);
        window_list_push(&windows, map_event->window);
        pthread_mutex_unlock(&windows.lock);
        xcb_flush(connection);
      }
      break;
    case XCB_DESTROY_NOTIFY:
      /* What to do when windows are destroyed */
      {
        xcb_destroy_notify_event_t *destroy_event = (xcb_destroy_notify_event_t*)event;

        /* Remove from list of windows for the workspace */
        pthread_mutex_lock(&windows.lock);
        window_list_remove(&windows, destroy_event->window);
        pthread_mutex_unlock(&windows.lock);
        xcb_flush(connection);
      }
      break;
    case XCB_BUTTON_PRESS:
      {
        xcb_button_press_event_t *button_event = (xcb_button_press_event_t *) event;
#ifndef LOGGING_RELEASE
        write_to_log("button handled");
#endif
        handle_buttons(button_event->detail);
      }
      break;
    case XCB_KEY_PRESS:
      {
        xcb_key_press_event_t *key_event = (xcb_key_press_event_t *) event;
#ifndef LOGGING_RELEASE
        write_to_log("Key handled");
#endif
        handle_keypress(key_event);
      }
      break;
    case XCB_KEY_RELEASE:
      {
        xcb_key_release_event_t *key_event = (xcb_key_release_event_t *) event;
#ifndef LOGGING_RELEASE
        write_to_log("Key handled");
#endif
        handle_keyrelease(key_event);
      }
      break;
    default:
      break;
  }
  free(event);
  return 0;
}

/* Set up substructure redirection on the root window
 * We assume we have a connection by now and don't bother to
 * get one. Written initially based off hootwm's setup function.
 * https://github.com/steinuil/hootwm
 */
void
setup ()
{
  xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  /* Mask for substructure redirection requests */
  uint32_t mask[1] = {
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
    XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
    XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
    XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
    XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE
  };
  xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, mask);
  /*
  xcb_grab_keyboard_cookie_t cookie;
  xcb_grab_keyboard_reply_t *reply = NULL;
  int count = 0;
  while ((reply == NULL || reply->status != XCB_GRAB_STATUS_SUCCESS) && (count++ < 500)) {
     cookie = xcb_grab_keyboard(connection, 0, screen->root, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
     reply = xcb_grab_keyboard_reply(connection, cookie, NULL);
     usleep(1000);
  }

  if (reply->status != XCB_GRAB_STATUS_SUCCESS) {
     fprintf(stderr, "Could not grab keyboard, status = %d\n", reply->status);
     exit(-1);
  }
  */
  xcb_flush(connection);
}

int
main ()
{
  /* Connect to X server */
  connection = xcb_connect(NULL, NULL);
  if(connection == NULL)
    perror("Startup failed.");
  assert(connection);

  /* Setup signal handler so we can gracefully exit */
  struct sigaction quit_sighandler, saved;
  quit_sighandler.sa_handler = &quit;
  sigaction(SIGINT, &quit_sighandler, &saved);

  /* Setup substructure redirection */
  setup();

  /* Set up communication with clients through a well known FIFO. */
#ifndef LOGGING_RELEASE
  write_to_log("Builing client FIFO.\n");
#endif
  char *server_endpoint = CLIENT_FIFO;
  mkfifo(server_endpoint, 0666);
  int server_endpoint_file = open(server_endpoint, O_RDONLY | O_NONBLOCK);
#ifndef LOGGING_RELEASE
  write_to_log("Finished builing client FIFO.\n");
#endif
  ipc_info ipc;
  //ipc.fifo = server_endpoint_file;
  ipc.fifofd = server_endpoint_file;

  /* Note that there are several ways to do this.
   * Hootwm, which is credited in this source frequently, will poll instead. This is inefficient and uses
   * a lot of resources to constantly look for events occuring. Tinywm does it the correct way, but through xlib
   * instead of xcb. Herbstluftwm uses another approach where the client will actually connect to the display and
   * directly manipulate the environment.
   *
   * This pthread also doesn't exit, or shouldn't anyways. TODO: Make this detached.
   */
  pthread_t input_handler;
  pthread_create(&input_handler, NULL, &handle_input, &ipc);

  int pid;
  char **args = NULL;
  char *start_loc = "/.config/scmwm/start";
  char *env = getenv("HOME");
  char *start = malloc(strlen(env) + strlen(start_loc));
  strcat(start, env);
  strcat(start, start_loc);
  if (access(start, X_OK) == 0) {
    pid = Fork();
    if(pid == 0){
      if(execv(start, args) == -1) {
        free(start);
#ifndef LOGGING_RELEASE
            write_to_log("Couldn't run start script\n");
#endif
      }
      exit(0);
    }
  }


  xcb_generic_event_t *ev;
  for (;;) {
#ifndef LOGGING_RELEASE
      write_to_log("Handling event.\n");
#endif
      ev = xcb_wait_for_event(connection);
      handle_event(ev);
#ifndef LOGGING_RELEASE
      write_to_log("Finished handling event.\n");
#endif
  }
  return 0;
}
