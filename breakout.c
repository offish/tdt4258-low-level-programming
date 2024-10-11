unsigned long long __attribute__((used)) VGAaddress = 0xC8000000;  // Memory storing pixels
unsigned long long __attribute__((used)) UARTaddress = 0xFF201000; // Memory storing UART data
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;

unsigned char n_cols = 10; // <- This variable might change depending on the size of the game. Supported value range: [1,18]

char *won = "You Won";
char *lost = "You Lost";
unsigned short height = 240;
unsigned short width = 320;
unsigned int bar_y = 120;
unsigned int ball_x = 15;
unsigned int ball_y = 120;

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

// assumes R0=x, R1=y, R2=width, R3=height, R4=color
void DrawBlock(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color);
asm("DrawBlock: \n\t"
    "PUSH {LR} \n\t"
    "PUSH {R5-R9} \n\t"
    "MOV R9, R0 \n\t"   // x
    "MOV R5, R1 \n\t"   // y
    "MOV R6, R2 \n\t"   // width
    "MOV R7, R3 \n\t"   // height
    "LDR R2, [R4] \n\t" // color
    "DrawBlockOuterLoop: \n\t"
    "MOV R4, R9 \n\t" // x
    "DrawBlockInnerLoop: \n\t"
    "MOV R0, R4 \n\t" // x
    "MOV R1, R5 \n\t" // y
    "BL SetPixel \n\t"
    "ADD R4, R4, #1 \n\t"
    "CMP R4, R6 \n\t"
    "BNE DrawBlockInnerLoop \n\t"
    "ADD R5, R5, #1 \n\t"
    "CMP R5, R7 \n\t"
    "BNE DrawBlockOuterLoop \n\t"
    "POP {R5-R9} \n\t"
    "POP {LR} \n\t"
    "BX LR");

// TODO: Impelement the DrawBar function in assembly. You need to accept the parameter as outlined in the c declaration above (unsigned int y)
void DrawBar(unsigned int y);
asm("DrawBar: \n\t"
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

// TODO: Implement the C functions below
void draw_ball()
{
    // the ball is a 7x7 black square
    DrawBlock(ball_x, ball_y, 7, 7, black);
}

void draw_playing_field()
{
    // the blocks are 15x15, neighboring blocks have different colors
    // the bar is 7x45 pixels
    // if the ball hits the 15 upper pixels -> 45 angles
    // if the ball hits the 15 middle pixels -> 90 angles
    // if the ball hits the 15 lower pixels -> 135 angles
}

int has_won_game()
{
    return ball_x == width;
}

int has_lost_game()
{
    return ball_x == 0;
}

void update_game_state()
{
    if (currentState != Running)
    {
        return;
    }

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

    // TODO: Update balls position and direction

    // TODO: Hit Check with Blocks
    // HINT: try to only do this check when we potentially have a hit, as it is relatively expensive and can slow down game play a lot
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
            write("up!");
            bar_y += 15;
        }
        else if (c == 0x73)
        {
            write("down!");
            bar_y -= 15;
        }
        else if (c == 0x0A)
        {
            currentState = Exit;
            return;
        }
    } while (remaining > 0);

    // w or s will also start the game
    // enter ends the game

    // TODO: Read all chars in the UART Buffer and apply the respective bar position updates
    // HINT: w == 77, s == 73, enter == 10
    // w -> move bar up 15 pixels
    // s -> move bar down 15 pixels

    // HINT Format: 0x00 'Remaining Chars':2 'Ready 0x80':2 'Char 0xXX':2, sample: 0x00018077 (1 remaining character, buffer is ready, current character is 'w')
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

    if (n_cols < 1 || n_cols > 18)
    {
        write("Invalid number of columns\n");
        return;
    }

    // ClearScreen();

    // HINT: This is the main game loop
    while (1)
    {
        update_game_state();
        update_bar_state();

        if (currentState != Running)
        {
            break;
        }

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

    // TODO: You might want to reset other state in here
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
    // reset();
    ClearScreen();

    DrawBlock(0, 0, 15, 15, red);

    // HINT: This loop allows the user to restart the game after loosing/winning the previous game
    while (1)
    {
        wait_for_start();
        play();
        reset();

        if (currentState == Exit)
        {
            break;
        }
    }

    return 0;
}
