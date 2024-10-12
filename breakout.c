unsigned long long __attribute__((used)) VGAaddress = 0xC8000000;  // Memory storing pixels
unsigned long long __attribute__((used)) UARTaddress = 0xFF201000; // Memory storing UART data
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;

// This variable might change depending on the size of the game. Supported value range: [1,18]
const unsigned char n_cols = 10;
const unsigned char n_rows = 15;

char *won = "You Won";
char *lost = "You Lost";
unsigned short height = 240;
unsigned short width = 320;
unsigned int bar_y = 120;
unsigned int ball_x = 15;
unsigned int ball_y = 120;
unsigned int is_heading_right = 1;
unsigned int ball_speed = 15;
unsigned int current_angle = 90;

void set_default_values()
{
    bar_y = 120;
    ball_x = 15;
    ball_y = 120;
    current_angle = 90;
    is_heading_right = 1;
}

/***
 * You might use and modify the struct/enum definitions below this comment
 */
typedef struct _block
{
    unsigned char destroyed;
    unsigned char deleted;
    unsigned int pos_x;
    unsigned int pos_y;
    unsigned int color;
} Block;
struct _block blocks[10][15];

typedef enum _gameState
{
    Stopped = 0,
    Running = 1,
    Won = 2,
    Lost = 3,
    Exit = 4,
} GameState;
GameState currentState = Stopped;

// assumes R0 = x-coord, R1 = y-coord, R2 = colorvalue
void SetPixel(unsigned int x_coord, unsigned int y_coord, unsigned int color);
asm("SetPixel: \n\t"
    "LDR R3, =VGAaddress \n\t"
    "LDR R3, [R3] \n\t"
    "LSL R1, R1, #10 \n\t"
    "LSL R0, R0, #1 \n\t"
    "ADD R1, R0 \n\t"
    "STRH R2, [R3, R1] \n\t"
    "BX LR");

void ClearScreen();
asm("ClearScreen: \n\t"
    "PUSH {LR} \n\t"
    "PUSH {R4, R5} \n\t"
    "LDR R2, =white \n\t"
    "LDR R2, [R2] \n\t"
    "MOV R5, #0 \n\t"
    "ClearScreenOuterLoop: \n\t"
    "MOV R4, #0  \n\t"
    "ClearScreenInnerLoop: \n\t"
    "MOV R0, R4 \n\t"
    "MOV R1, R5 \n\t"
    "BL SetPixel \n\t"
    "ADD R4, R4, #1 \n\t"
    "CMP R4, #320 \n\t"
    "BEQ ClearScreenNextRow \n\t"
    "B ClearScreenInnerLoop \n\t"
    "ClearScreenNextRow: \n\t"
    "ADD R5, R5, #1 \n\t"
    "CMP R5, #240 \n\t"
    "BEQ ClearScreenDone \n\t"
    "B ClearScreenOuterLoop \n\t"
    "ClearScreenDone: \n\t"
    "POP {R4, R5} \n\t"
    "POP {LR} \n\t"
    "BX LR");

// TODO: fix color parameter
// assumes R0=x, R1=y, R2=width, R3=height, R4=color
void DrawBlock(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color);
asm("DrawBlock: \n\t"
    "PUSH {LR} \n\t"
    "PUSH {R4-R9} \n\t"
    "MOV R9, R0 \n\t"     // x
    "MOV R5, R1 \n\t"     // y
    "MOV R6, R2 \n\t"     // width
    "ADD R6, R6, R9 \n\t" // width + x
    "MOV R7, R3 \n\t"     // height
    "ADD R7, R7, R5 \n\t" // height + y
    "LDR R2, [R4] \n\t"   // color
    "DrawBlockOuterLoop: \n\t"
    "MOV R4, R9 \n\t" // x
    "DrawBlockInnerLoop: \n\t"
    "MOV R0, R4 \n\t" // x
    "MOV R1, R5 \n\t" // y
    "BL SetPixel \n\t"
    "ADD R4, R4, #1 \n\t"
    "CMP R4, R6 \n\t" // compare x with width
    "BNE DrawBlockInnerLoop \n\t"
    "ADD R5, R5, #1 \n\t"
    "CMP R5, R7 \n\t" // comare y with height
    "BNE DrawBlockOuterLoop \n\t"
    "POP {R4-R9} \n\t"
    "POP {LR} \n\t"
    "BX LR");

// assumes R0=y
void DrawBar(unsigned int y);
asm("DrawBar: \n\t"
    "PUSH {LR} \n\t"
    "PUSH {R4} \n\t"
    "MOV R1, R0 \n\t"  // y
    "MOV R0, #0 \n\t"  // x
    "MOV R2, #7 \n\t"  // width
    "MOV R3, #45 \n\t" // height
    "LDR R4, =blue \n\t"
    "BL DrawBlock \n\t"
    "POP {R4} \n\t"
    "POP {LR} \n\t"
    "BX LR");

int ReadUart();
asm("ReadUart:\n\t"
    "LDR R1, =UARTaddress \n\t"
    "LDR R1, [R1] \n\t"
    "LDR R0, [R1] \n\t"
    "BX LR");

void WriteUart(char c);
asm("WriteUart:\n\t"
    "LDR R1, =UARTaddress \n\t"
    "LDR R1, [R1] \n\t"
    "STRH R0, [R1] \n\t"
    "BX LR");

void draw_ball()
{
    // the ball is a 7x7 black square
    DrawBlock(ball_x, ball_y, 7, 7, black);
}

void init_blocks()
{
    for (int i = 0; i < n_cols; i++)
    {
        for (int j = 0; j < n_rows; j++)
        {
            blocks[i][j].pos_x = 100 + i * 15;
            blocks[i][j].pos_y = j * 15;
            blocks[i][j].color = red;
            // blocks[i][j].color = 0x0000F0F0;
            blocks[i][j].destroyed = 0;
            blocks[i][j].deleted = 0;

            DrawBlock(blocks[i][j].pos_x, blocks[i][j].pos_y, 15, 15, blocks[i][j].color);
        }
    }
}

void draw_playing_field()
{
    // the blocks are 15x15, neighboring blocks have different colors
    // the bar is 7x45 pixels

    for (int i = 0; i < n_cols; i++)
    {
        for (int j = 0; j < n_rows; j++)
        {
            if (blocks[i][j].destroyed == 1)
                continue;

            DrawBlock(blocks[i][j].pos_x, blocks[i][j].pos_y, 15, 15, blocks[i][j].color);
        }
    }
}

int has_won_game()
{
    return ball_x >= width;
}

int has_lost_game()
{
    return ball_x < 7 && ball_y < bar_y && ball_y > bar_y + 45;
}

void check_blocks_collision()
{
    for (int i = 0; i < n_cols; i++)
    {
        for (int j = 0; j < n_rows; j++)
        {
            if (blocks[i][j].destroyed == 1)
                continue;

            unsigned int x_pos = blocks[i][j].pos_x;
            unsigned int y_pos = blocks[i][j].pos_y;

            unsigned int x_ball_center = ball_x + 3;
            unsigned int y_ball_center = ball_y + 3;

            if (x_ball_center < x_pos || x_ball_center > x_pos + 15 || y_ball_center < y_pos || y_ball_center > y_pos + 15)
                continue;

            // if (
            //     x_ball_center >= x_pos && x_ball_center <= x_pos + 15 && y_ball_center >= y_pos && y_ball_center <= y_pos + 15)
            // {
            blocks[i][j].destroyed = 1;

            // DrawBlock(x_pos, y_pos, 15, 15, white);

            is_heading_right = !is_heading_right;
            return;
            // }
        }
    }
}

void check_bar_collision()
{
    // if the ball hits the 15 upper pixels -> 45 angles
    // if the ball hits the 15 middle pixels -> 90 angles
    // if the ball hits the 15 lower pixels -> 135 angles
    if (ball_x > 7 || ball_y < bar_y || ball_y > bar_y + 45)
        return;

    // if (ball_x <= 7 && ball_y >= bar_y && ball_y <= bar_y + 45)
    // {
    is_heading_right = !is_heading_right;

    if (ball_y < bar_y + 15)
    {
        current_angle = 45;
    }
    else if (ball_y < bar_y + 30)
    {
        current_angle = 90;
    }
    else
    {
        current_angle = 135;
    }
}

void update_game_state()
{
    if (currentState != Running)
        return;

    if (is_heading_right)
    {
        if (current_angle == 45)
        {
            ball_x += ball_speed;
            ball_y -= ball_speed;
        }
        else if (current_angle == 90)
        {
            ball_x += ball_speed;
        }
        else
        {
            ball_x += ball_speed;
            ball_y += ball_speed;
        }
    }
    else
    {
        if (current_angle == 45)
        {
            ball_x -= ball_speed;
            ball_y -= ball_speed;
        }
        else if (current_angle == 90)
        {
            ball_x -= ball_speed;
        }
        else
        {
            ball_x -= ball_speed;
            ball_y += ball_speed;
        }
    }

    if (ball_y >= height - 7 || ball_y <= 0)
    {
        current_angle = 180 - current_angle;

        if (ball_y >= height - 7)
        {
            ball_y = height - 7;
        }
        else
        {
            ball_y = 0;
        }
    }

    // TODO: fix winning in losing conditions
    if (has_won_game())
    {
        currentState = Won;
        return;
    }

    if (has_lost_game())
    {
        currentState = Lost;
        return;
    }

    // check for collisions with blocks
    if (is_heading_right == 1)
    {
        check_blocks_collision();
    }
    else
    {
        check_bar_collision();
    }
}

void update_bar_state()
{
    int remaining = 0;

    do
    {
        unsigned long long out = ReadUart();

        if (!(out & 0x8000))
        {
            // not valid - abort reading
            return;
        }
        remaining = (out & 0xFF0000) >> 4;

        char c = out & 0xFF;

        if (c == 0x77)
        {
            // write("up!");
            if (bar_y >= 15)
            {
                bar_y -= 15;
            }
        }
        else if (c == 0x73)
        {
            // write("down!");
            if (bar_y < height - 45)
            {
                bar_y += 15;
            }
        }
        else if (c == 0x0A)
        {
            currentState = Exit;
            return;
        }
    } while (remaining > 0);
}

void write(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        WriteUart(str[i]);
    }
}

void play()
{
    write("Game started! Good luck!\n");

    init_blocks();

    // main game loop
    while (1)
    {
        ClearScreen();
        // reset();
        update_game_state();
        update_bar_state();

        if (currentState != Running)
            break;

        draw_playing_field();
        draw_ball();
        DrawBar(bar_y);
    }

    if (currentState == Won)
    {
        write(won);
    }
    else if (currentState == Lost)
    {
        write(lost);
    }
    else if (currentState == Exit)
    {
        write("Game ended!\n");
        return;
    }

    currentState = Stopped;
}

void reset()
{
    // HINT: This is draining the UART buffer
    int remaining = 0;

    do
    {
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            // not valid - abort reading
            return;
        }
        remaining = (out & 0xFF0000) >> 4;
    } while (remaining > 0);

    // reset other states here
    set_default_values();
}

void wait_for_start()
{
    // waiting behaviour until the user presses either w/s
    while (1)
    {
        int remaining = 0;

        do
        {
            unsigned long long out = ReadUart();

            if (!(out & 0x8000))
            {
                // not valid - abort reading
                break;
            }
            remaining = (out & 0xFF0000) >> 4;

            char c = out & 0xFF;

            if (c == 0x77 || c == 0x73)
            {
                currentState = Running;
                return;
            }
        } while (remaining > 0);
    }
}

int main(int argc, char *argv[])
{
    if (n_cols < 1 || n_cols > 18)
    {
        write("Invalid number of columns\n");
        return 1;
    }

    ClearScreen();

    // loop allows the user to restart the game after loosing/winning the previous game
    while (1)
    {
        wait_for_start();
        set_default_values();
        play();
        reset();

        if (currentState == Exit)
            break;
    }

    return 0;
}
