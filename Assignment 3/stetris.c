#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <linux/ioctl.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <poll.h>

// The game state can be used to detect what happens on the playfield
#define GAMEOVER 0
#define ACTIVE (1 << 0)
#define ROW_CLEAR (1 << 1)
#define TILE_ADDED (1 << 2)

// If you extend this structure, either avoid pointers or adjust
// the game logic allocate/deallocate and reset the memory
typedef struct
{
    bool occupied;
    int color;
} tile;

typedef struct
{
    unsigned int x;
    unsigned int y;
} coord;

typedef struct
{
    coord const grid;                     // playfield bounds
    unsigned long const uSecTickTime;     // tick rate
    unsigned long const rowsPerLevel;     // speed up after clearing rows
    unsigned long const initNextGameTick; // initial value of nextGameTick

    unsigned int tiles; // number of tiles played
    unsigned int rows;  // number of rows cleared
    unsigned int score; // game score
    unsigned int level; // game level

    tile *rawPlayfield; // pointer to raw memory of the playfield
    tile **playfield;   // This is the play field array
    unsigned int state;
    coord activeTile; // current tile

    unsigned long tick;         // incremeted at tickrate, wraps at nextGameTick
                                // when reached 0, next game state calculated
    unsigned long nextGameTick; // sets when tick is wrapping back to zero
                                // lowers with increasing level, never reaches 0
} gameConfig;

gameConfig game = {
    .grid = {8, 8},
    .uSecTickTime = 10000,
    .rowsPerLevel = 2,
    .initNextGameTick = 50,
};

size_t screen_size = 8 * 8;
static char *frame_buffer_pointer = 0;
static int frame_buffer_device = -1;
static int joystick_device = -1;
static struct fb_fix_screeninfo fixed_screeninfo;
static struct input_keymap_entry keymap;

const int red_color = 0xf800;
const int green_color = 0x07e0;
const int blue_color = 0x001f;
const int cyan_color = 0x07ff;
const int magenta_color = 0xf81f;
const int yellow_color = 0xffe0;
const int orange_color = 0xfd40;
const int black = 0x0000;

int colors[] = {red_color, green_color, blue_color, cyan_color, magenta_color, yellow_color, orange_color};
unsigned int counter = 0;

static inline bool tileOccupied(coord const target);

int getNextColor()
{
    // new color every 7th call
    counter++;
    return colors[counter % 7];
}

// This function is called on the start of your application
// Here you can initialize what ever you need for your task
// return false if something fails, else true
bool foundFrameBuffer()
{
    unsigned int i = 0;
    char frame_buffer_path[9];

    // loop through all frame buffer devices
    while (i < FB_MAX)
    {
        // format path string
        snprintf(frame_buffer_path, sizeof(frame_buffer_path), "/dev/fb%d", i);
        // open device
        frame_buffer_device = open(frame_buffer_path, O_RDWR);

        // check if device is valid
        if (frame_buffer_device == -1)
        {
            i++;
            continue;
        }

        // get fixed screen info
        if (ioctl(frame_buffer_device, FBIOGET_FSCREENINFO, &fixed_screeninfo) == 0)
        {
            // check if device is RPi-Sense
            if (strcmp(fixed_screeninfo.id, "RPi-Sense FB") == 0)
            {
                return true;
            }
        }

        // close device and move onto the next one
        close(frame_buffer_device);
        i++;
    }

    perror("Error: cannot find RPi-Sense framebuffer device");
    return false;
}

bool foundJoystickDevice()
{
    unsigned int i = 0;
    char joystick_path[20];
    char joystick_name[32];

    // loop through possible input devices
    while (i < 9)
    {
        // format path string
        snprintf(joystick_path, sizeof(joystick_path), "/dev/input/event%d", i);
        // open device
        joystick_device = open(joystick_path, O_RDONLY | O_NONBLOCK);

        // check if device is valid
        if (joystick_device == -1)
        {
            i++;
            continue;
        }

        // get device name
        ioctl(joystick_device, EVIOCGNAME(sizeof(joystick_name)), joystick_name);
        // check if device is the correct joystick
        if (strcmp(joystick_name, "Raspberry Pi Sense HAT Joystick") == 0)
        {
            return true;
        }

        // close device and move onto the next one
        close(joystick_device);
        i++;
    }

    perror("Error: cannot find RPi-Sense joystick device");
    return false;
}

bool initializeSenseHat()
{
    bool found_frame_buffer = foundFrameBuffer();

    if (!found_frame_buffer)
    {
        return false;
    }

    // map the device to memory
    frame_buffer_pointer = (char *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, frame_buffer_device, 0);

    if ((int)frame_buffer_pointer == -1)
    {
        perror("Error: failed to map framebuffer device to memory");
        return false;
    }

    bool found_joystick = foundJoystickDevice();

    if (!found_joystick)
    {
        return false;
    }

    return true;
}

// This function is called when the application exits
// Here you can free up everything that you might have opened/allocated
void freeSenseHat()
{
    // free up devices
    if (frame_buffer_pointer)
    {
        munmap(frame_buffer_pointer, screen_size);
    }
    if (frame_buffer_device != -1)
    {
        close(frame_buffer_device);
    }
    if (joystick_device != -1)
    {
        close(joystick_device);
    }
}

// This function should return the key that corresponds to the joystick press
// KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, with the respective direction
// and KEY_ENTER, when the the joystick is pressed
// !!! when nothing was pressed you MUST return 0 !!!
int readSenseHatJoystick()
{
    struct input_event ev;

    while (read(joystick_device, &ev, sizeof(struct input_event)) > 0)
    {
        // is not a key event
        if (ev.type != EV_KEY)
        {
            continue;
        }

        // is not a key press or auto-repeat event
        if (ev.value != 1 && ev.value != 2)
        {
            continue;
        }

        // check which key was pressed
        switch (ev.code)
        {
        case KEY_UP:
            return KEY_UP;
        case KEY_DOWN:
            return KEY_DOWN;
        case KEY_LEFT:
            return KEY_LEFT;
        case KEY_RIGHT:
            return KEY_RIGHT;
        case KEY_ENTER:
            return KEY_ENTER;
        }
    }

    return 0;
}

int getLocation(unsigned int x, unsigned int y)
{
    int bytes_per_pixel = 16 / 8;
    int bytes_per_line = bytes_per_pixel * 8;

    return x * bytes_per_pixel + y * bytes_per_line;
}

void clearSenseHatMatrix()
{
    // loop through screen and set every pixel to black
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        for (unsigned int x = 0; x < game.grid.x; x++)
        {
            int location = getLocation(x, y);
            *((unsigned int *)(frame_buffer_pointer + location)) = black;
        }
    }
}

// This function should render the gamefield on the LED matrix. It is called
// every game tick. The parameter playfieldChanged signals whether the game logic
// has changed the playfield

void renderSenseHatMatrix(bool const playfieldChanged)
{
    // clear the screen
    clearSenseHatMatrix();

    // loop through the playfield and draw the tiles
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        for (unsigned int x = 0; x < game.grid.x; x++)
        {
            coord const checkTile = {x, y};

            // nothing to draw here
            if (!tileOccupied(checkTile))
            {
                continue;
            }

            int location = getLocation(x, y);
            *((unsigned int *)(frame_buffer_pointer + location)) = getTileColor(checkTile);
        }
    }

    (void)playfieldChanged;
}

// The game logic uses only the following functions to interact with the playfield.
// if you choose to change the playfield or the tile structure, you might need to
// adjust this game logic <> playfield interface

static inline void newTile(coord const target)
{
    game.playfield[target.y][target.x].occupied = true;
    // every new tile gets a "new" color (repeating every 7th iteration)
    game.playfield[target.y][target.x].color = getNextColor();
}

int getTileColor(coord const target)
{
    return game.playfield[target.y][target.x].color;
}

static inline void copyTile(coord const to, coord const from)
{
    memcpy((void *)&game.playfield[to.y][to.x], (void *)&game.playfield[from.y][from.x], sizeof(tile));
}

static inline void copyRow(unsigned int const to, unsigned int const from)
{
    memcpy((void *)&game.playfield[to][0], (void *)&game.playfield[from][0], sizeof(tile) * game.grid.x);
}

static inline void resetTile(coord const target)
{
    memset((void *)&game.playfield[target.y][target.x], 0, sizeof(tile));
}

static inline void resetRow(unsigned int const target)
{
    memset((void *)&game.playfield[target][0], 0, sizeof(tile) * game.grid.x);
}

static inline bool tileOccupied(coord const target)
{
    return game.playfield[target.y][target.x].occupied;
}

static inline bool rowOccupied(unsigned int const target)
{
    for (unsigned int x = 0; x < game.grid.x; x++)
    {
        coord const checkTile = {x, target};
        if (!tileOccupied(checkTile))
        {
            return false;
        }
    }
    return true;
}

static inline void resetPlayfield()
{
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        resetRow(y);
    }
}

// Below here comes the game logic. Keep in mind: You are not allowed to change how the game works!
// that means no changes are necessary below this line! And if you choose to change something
// keep it compatible with what was provided to you!

bool addNewTile()
{
    game.activeTile.y = 0;
    game.activeTile.x = (game.grid.x - 1) / 2;
    if (tileOccupied(game.activeTile))
        return false;
    newTile(game.activeTile);
    return true;
}

bool moveRight()
{
    coord const newTile = {game.activeTile.x + 1, game.activeTile.y};
    if (game.activeTile.x < (game.grid.x - 1) && !tileOccupied(newTile))
    {
        copyTile(newTile, game.activeTile);
        resetTile(game.activeTile);
        game.activeTile = newTile;
        return true;
    }
    return false;
}

bool moveLeft()
{
    coord const newTile = {game.activeTile.x - 1, game.activeTile.y};
    if (game.activeTile.x > 0 && !tileOccupied(newTile))
    {
        copyTile(newTile, game.activeTile);
        resetTile(game.activeTile);
        game.activeTile = newTile;
        return true;
    }
    return false;
}

bool moveDown()
{
    coord const newTile = {game.activeTile.x, game.activeTile.y + 1};
    if (game.activeTile.y < (game.grid.y - 1) && !tileOccupied(newTile))
    {
        copyTile(newTile, game.activeTile);
        resetTile(game.activeTile);
        game.activeTile = newTile;
        return true;
    }
    return false;
}

bool clearRow()
{
    if (rowOccupied(game.grid.y - 1))
    {
        for (unsigned int y = game.grid.y - 1; y > 0; y--)
        {
            copyRow(y, y - 1);
        }
        resetRow(0);
        return true;
    }
    return false;
}

void advanceLevel()
{
    game.level++;
    switch (game.nextGameTick)
    {
    case 1:
        break;
    case 2 ... 10:
        game.nextGameTick--;
        break;
    case 11 ... 20:
        game.nextGameTick -= 2;
        break;
    default:
        game.nextGameTick -= 10;
    }
}

void newGame()
{
    game.state = ACTIVE;
    game.tiles = 0;
    game.rows = 0;
    game.score = 0;
    game.tick = 0;
    game.level = 0;
    resetPlayfield();
}

void gameOver()
{
    game.state = GAMEOVER;
    game.nextGameTick = game.initNextGameTick;
}

bool sTetris(int const key)
{
    bool playfieldChanged = false;

    if (game.state & ACTIVE)
    {
        // Move the current tile
        if (key)
        {
            playfieldChanged = true;
            switch (key)
            {
            case KEY_LEFT:
                moveLeft();
                break;
            case KEY_RIGHT:
                moveRight();
                break;
            case KEY_DOWN:
                while (moveDown())
                {
                };
                game.tick = 0;
                break;
            default:
                playfieldChanged = false;
            }
        }

        // If we have reached a tick to update the game
        if (game.tick == 0)
        {
            // We communicate the row clear and tile add over the game state
            // clear these bits if they were set before
            game.state &= ~(ROW_CLEAR | TILE_ADDED);

            playfieldChanged = true;
            // Clear row if possible
            if (clearRow())
            {
                game.state |= ROW_CLEAR;
                game.rows++;
                game.score += game.level + 1;
                if ((game.rows % game.rowsPerLevel) == 0)
                {
                    advanceLevel();
                }
            }

            // if there is no current tile or we cannot move it down,
            // add a new one. If not possible, game over.
            if (!tileOccupied(game.activeTile) || !moveDown())
            {
                if (addNewTile())
                {
                    game.state |= TILE_ADDED;
                    game.tiles++;
                }
                else
                {
                    gameOver();
                }
            }
        }
    }

    // Press any key to start a new game
    if ((game.state == GAMEOVER) && key)
    {
        playfieldChanged = true;
        newGame();
        addNewTile();
        game.state |= TILE_ADDED;
        game.tiles++;
    }

    return playfieldChanged;
}

int readKeyboard()
{
    struct pollfd pollStdin = {
        .fd = STDIN_FILENO,
        .events = POLLIN};
    int lkey = 0;

    if (poll(&pollStdin, 1, 0))
    {
        lkey = fgetc(stdin);
        if (lkey != 27)
            goto exit;
        lkey = fgetc(stdin);
        if (lkey != 91)
            goto exit;
        lkey = fgetc(stdin);
    }
exit:
    switch (lkey)
    {
    case 10:
        return KEY_ENTER;
    case 65:
        return KEY_UP;
    case 66:
        return KEY_DOWN;
    case 67:
        return KEY_RIGHT;
    case 68:
        return KEY_LEFT;
    }
    return 0;
}

void renderConsole(bool const playfieldChanged)
{
    if (!playfieldChanged)
        return;

    // Goto beginning of console
    fprintf(stdout, "\033[%d;%dH", 0, 0);
    for (unsigned int x = 0; x < game.grid.x + 2; x++)
    {
        fprintf(stdout, "-");
    }
    fprintf(stdout, "\n");
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        fprintf(stdout, "|");
        for (unsigned int x = 0; x < game.grid.x; x++)
        {
            coord const checkTile = {x, y};
            fprintf(stdout, "%c", (tileOccupied(checkTile)) ? '#' : ' ');
        }
        switch (y)
        {
        case 0:
            fprintf(stdout, "| Tiles: %10u\n", game.tiles);
            break;
        case 1:
            fprintf(stdout, "| Rows:  %10u\n", game.rows);
            break;
        case 2:
            fprintf(stdout, "| Score: %10u\n", game.score);
            break;
        case 4:
            fprintf(stdout, "| Level: %10u\n", game.level);
            break;
        case 7:
            fprintf(stdout, "| %17s\n", (game.state == GAMEOVER) ? "Game Over" : "");
            break;
        default:
            fprintf(stdout, "|\n");
        }
    }
    for (unsigned int x = 0; x < game.grid.x + 2; x++)
    {
        fprintf(stdout, "-");
    }
    fflush(stdout);
}

inline unsigned long uSecFromTimespec(struct timespec const ts)
{
    return ((ts.tv_sec * 1000000) + (ts.tv_nsec / 1000));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    // This sets the stdin in a special state where each
    // keyboard press is directly flushed to the stdin and additionally
    // not outputted to the stdout
    {
        struct termios ttystate;
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag &= ~(ICANON | ECHO);
        ttystate.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    }

    // Allocate the playing field structure
    game.rawPlayfield = (tile *)malloc(game.grid.x * game.grid.y * sizeof(tile));
    game.playfield = (tile **)malloc(game.grid.y * sizeof(tile *));
    if (!game.playfield || !game.rawPlayfield)
    {
        fprintf(stderr, "ERROR: could not allocate playfield\n");
        return 1;
    }
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        game.playfield[y] = &(game.rawPlayfield[y * game.grid.x]);
    }

    // Reset playfield to make it empty
    resetPlayfield();
    // Start with gameOver
    gameOver();

    if (!initializeSenseHat())
    {
        fprintf(stderr, "ERROR: could not initilize sense hat\n");
        return 1;
    };

    // Clear console, render first time
    fprintf(stdout, "\033[H\033[J");
    renderConsole(true);
    renderSenseHatMatrix(true);

    while (true)
    {
        struct timeval sTv, eTv;
        gettimeofday(&sTv, NULL);

        int key = readSenseHatJoystick();
        if (!key)
        {
            // NOTE: Uncomment the next line if you want to test your implementation with
            // reading the inputs from stdin. However, we expect you to read the inputs directly
            // from the input device and not from stdin (you should implement the readSenseHatJoystick
            // method).
            // key = readKeyboard();
        }
        if (key == KEY_ENTER)
            break;

        bool playfieldChanged = sTetris(key);
        renderConsole(playfieldChanged);
        renderSenseHatMatrix(playfieldChanged);

        // Wait for next tick
        gettimeofday(&eTv, NULL);
        unsigned long const uSecProcessTime = ((eTv.tv_sec * 1000000) + eTv.tv_usec) - ((sTv.tv_sec * 1000000 + sTv.tv_usec));
        if (uSecProcessTime < game.uSecTickTime)
        {
            usleep(game.uSecTickTime - uSecProcessTime);
        }
        game.tick = (game.tick + 1) % game.nextGameTick;
    }

    freeSenseHat();
    free(game.playfield);
    free(game.rawPlayfield);

    return 0;
}