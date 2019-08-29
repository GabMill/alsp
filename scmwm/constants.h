#ifndef CLIENT_FIFO
#define CLIENT_FIFO "/tmp/scmwm"
#endif
#ifndef MAX_EVENTS
#define MAX_EVENTS 1
#endif
#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE 200
#endif

/* Internal constants
 * Modified from hootwm 
 */
#ifndef XCB_MOVE
#define XCB_MOVE XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#endif
#ifndef XCB_RESIZE
#define XCB_RESIZE XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#endif
#ifndef XCB_MOVE_RESIZE
#define XCB_MOVE_RESIZE XCB_MOVE | XCB_RESIZE
#endif

#ifndef NUM_WINDOWS
#define NUM_WINDOWS 4
#endif

/* User defined constants */
#ifndef PAD_LEFT
#define PAD_LEFT 30
#endif
#ifndef PAD_RIGHT
#define PAD_RIGHT 0
#endif
#ifndef PAD_BOTTOM
#define PAD_BOTTOM 0
#endif
#ifndef PAD_TOP
#define PAD_TOP 30
#endif
#ifndef NUM_TAGS
#define NUM_TAGS 10
#endif

#ifndef WINDOW_DEFAULT_SIZE
#define WINDOW_DEFAULT_SIZE 800, 800
#endif
#ifndef WINDOW_DEFAULT_POSITION
#define WINDOW_DEFAULT_POSITION PAD_TOP, PAD_LEFT
#endif
#ifndef WINDOW_DEFAULTS
#define WINDOW_DEFAULTS {WINDOW_DEFAULT_POSITION, WINDOW_DEFAULT_SIZE}
#endif
