// gcc -Wall -Werror -O0 -o main main.c

// --- IMPORTS ---
#include <stdio.h>  // standard input output
#include <stdlib.h>  // standard library
#include <stdbool.h>  // include booleans(??)
#include <time.h>  // time library
#include <SDL.h>  // SDL

// --- DEFINITIONS ---
#define BOARDX 25
#define BOARDY 10


// --- STRUCTS --- (must be defined before prototypes) -----------------------------------------------------------------
struct point {
    int x;
    int y;
};


// --- PROTOTYPES ------------------------------------------------------------------------------------------------------
// passing 2D arrays as pointer:  https://stackoverflow.com/questions/16724368/how-to-pass-a-2d-array-by-pointer-in-c
// rather than making a 2D array of pointers, make a (pointer that points to)-> a 2D array :))
bool init_sdl(void);
void resetGame(struct point *pPlayer, int (*pMap)[BOARDY][BOARDX]);
bool validateMap(int map[BOARDY][BOARDX], struct point *pPlayer);
char getInput(void);
char keyInput(void);
struct point movement(char cmd, struct point player, int (*pMap)[BOARDY][BOARDX]);
void draw(int (*pMap)[BOARDY][BOARDX], struct point player, int traversed);
bool pointInMap(struct point *pPlayer);
int random_number(int min_num, int max_num);


// --- MAIN ------------------------------------------------------------------------------------------------------------
int main() {
    // defs
    bool run = init_sdl();
    char cmd;
    struct point player;
    int map[BOARDY][BOARDX];
    int traversed = 0;
    resetGame(&player, &map);  // provide memory location of map and player

    // -- main loop --
    draw(&map, player, traversed);  // initial display of board before first input has been given
    while (run) {
        // -- get input (ignore newline) --
        //cmd = getInput();
        cmd = keyInput();
        if (cmd != '\n' && cmd != ' ') {

            // -- handle input --
            // handle quit
            if (cmd == 'q' || cmd == EOF) {
                run = false;
            } else if (map[player.y][player.x] == 2) {  // if player accesses door
                resetGame(&player, &map);
                traversed++;
            } else if (cmd == 'r') {
                resetGame(&player, &map);
            } else {
                player = movement(cmd, player, &map);
            }

            // -- show board state --
            draw(&map, player, traversed);
        }
    }

    return 0;  // indicate successful termination
}


// --- FUNCTIONS -------------------------------------------------------------------------------------------------------
// init sdl
bool init_sdl(void) {
    // init everything via SDL const
    // if an error occurs (0 indicates success), output error
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");  // print error to standard error output
        return false;
    }
    return true;
}

// modify map using pointer
void resetGame(struct point *pPlayer, int (*pMap)[BOARDY][BOARDX]) {
    // reset player pos for new room (-> used to access struct attr from pointer)
    pPlayer->x = 1;
    pPlayer->y = BOARDY / 2;

    bool valid = false;  // whether map can be traversed
    // re-generate map until it is traversable
    while (!valid) {
        // generate empty board
        // empty cell < 10. Filled cell >= 10
        for (int y=0; y < BOARDY; y++) {
            for (int x=0; x < BOARDX; x++) {

                // border walls
                if (y == 0 || x == 0 || y == BOARDY - 1 || x == BOARDX - 1) {
                    (*pMap)[y][x] = 10;

                // randomise other cells (empty or wall)
                } else {
                    int rand = random_number(0, 1);
                    if (rand < 0.5) {
                        (*pMap)[y][x] = 0;
                    } else {
                        (*pMap)[y][x] = 10;
                    }
                }
            }
        }

        // add door (ensure not in inaccessible corners)
        (*pMap)[random_number(1, BOARDY - 2)][BOARDX - 1] = 2;
        // evaluate map traversibility
        valid = validateMap(*pMap, pPlayer);
    }

    // modifiers to point to find neighbours
    struct point neighbours[] = {{1, 0},
                                 {0, 1},
                                 {-1, 0},
                                 {0, -1}};
    // fill out spaces that are surrounded by walls
    for (int y=0; y < BOARDY; y++) {
        for (int x=0; x < BOARDX; x++) {

            // skip cell if its already a wall
            if ((*pMap)[y][x] != 10) {
                int surrounded = 0;

                // determine how many walls a cell is surrounded by
                for (int n=0; n < 4; n++) {
                    struct point neighbour = {x + neighbours[n].x,
                                              y + neighbours[n].y};
                    if (pointInMap(&neighbour) && (*pMap)[neighbour.y][neighbour.x] == 10) {
                        surrounded++;
                    }
                }

                // change to wall if surrounded by walls
                if (surrounded == 4) {
                    (*pMap)[y][x] = 10;
                }
            }
        }
    }

    // ensure player pos is empty
    (*pMap)[pPlayer->y][pPlayer->x] = 0;
}


bool validateMap(int map[BOARDY][BOARDX], struct point *pPlayer) {
    bool valid = false;  // assume not valid until valid path found
    // modifiers to point to find neighbours
    struct point neighbours[] = {{1, 0},
                                 {0, 1},
                                 {-1, 0},
                                 {0, -1}};
    // init visited map
    bool visited[BOARDY][BOARDX];
    for (int y=0; y < BOARDY; y++) {
        for (int x=0; x < BOARDX; x++) {
            visited[y][x] = false;
        }
    }

    // must be able to store at max the entire boards worth of nodes
    // begin with player position as starting point of search
    struct point queue[BOARDY * BOARDX];
    queue[0] = *pPlayer;
    map[pPlayer->y][pPlayer->x] = 10;  // set player as visited (wall)
    int writeIndex = 1;  // next index to be written in array

    int i = 0;  // current node index in queue
    // loop until either the end of the queue is reached (no more available nodes, or door is found
    while (valid == false && i < writeIndex) {
        // loop through neighbours
        for (int nI=0; nI < 4; nI++) {
            struct point neighbour = {queue[i].x + neighbours[nI].x,
                                      queue[i].y + neighbours[nI].y};

            // make sure point is in map and has not been visited and is not a wall, then proceed
            if (pointInMap(&neighbour) && visited[neighbour.y][neighbour.x] == false && map[neighbour.y][neighbour.x] != 10) {
                // check if neighbour is door
                if (map[neighbour.y][neighbour.x] == 2) {
                    valid = true;
                    break;
                // add neighbour to queue
                } else {
                    queue[writeIndex] = neighbour;
                    writeIndex++;
                }
                // set neighbour as visited
                visited[neighbour.y][neighbour.x] = true;
            }
        }
        i++;
    }
    return valid;
}


// returns single char NOT string
char getInput(void) {
    char cmd[20];
    printf("\n> ");
    scanf("%s", cmd);  // store input at cmd memory address
    return cmd[0];
}


// process inputs
char keyInput(void) {
   SDL_Event event;  // event struct
   SDL_PollEvent(&event);  // go check all the inputs

   if (event.type == SDL_KEYDOWN) {
       switch (event.key.keysym.sym) {
           case SDLK_a:
               return 'a';
           case SDLK_LEFT:
                return 'a';

            case SDLK_d:
                return 'd';
            case SDLK_RIGHT:
                return 'd';

            case SDLK_w:
                return 'w';
            case SDLK_UP:
                return 'w';

            case SDLK_s:
                return 's';
            case SDLK_DOWN:
                return 's';

            case SDLK_r:
                return 'r';
            case SDLK_q:
                return 'q';
       }
   }

   return ' ';
}


struct point movement(char cmd, struct point player, int (*pMap)[BOARDY][BOARDX]) {
    struct point ogPlayer = player;  // original player pos

    // find new player position
    // dimension - 1 to account for counting from 0
    if (cmd == 'd' && player.x + 1 < BOARDX) {
        player.x++;
    } else if (cmd == 'a' && player.x - 1 >= 0) {
        player.x--;
    } else if (cmd == 'w' && player.y - 1 >= 0) {
        player.y--;
    } else if (cmd == 's' && player.y + 1 < BOARDY) {
        player.y++;
    }

    // collision detection
    // as long as the player is on an empty space, move player, otherwise stay still
    // empty < 10. Filled >= 10
    if ((*pMap)[player.y][player.x] < 10) {
        return player;
    } else {
        return ogPlayer;
    }
}


void draw(int (*pMap)[BOARDY][BOARDX], struct point player, int traversed) {
    system("clear");  // clear screen using terminal command

    printf("\n");
    // print each cell
    for (int y=0; y < BOARDY; y++) {
        for (int x=0; x < BOARDX; x++) {
            // print player in its cell
            // codes: empty < 10. Filled >= 10
            if (player.x == x && player.y == y) {
                printf("P ");
            } else if ((*pMap)[y][x] == 0) {
                printf(". ");
            } else if ((*pMap)[y][x] == 2) {
                printf("> ");
            } else if ((*pMap)[y][x] == 10) {
                printf("⎕ ");
            }
        }
        printf("\n");
    }
    printf("Rooms traversed: %d", traversed);  // no \n to account for trailing \n from map draw
}


// return if point is in map or not
bool pointInMap(struct point *pPoint) {
    if (0 <= pPoint->x && pPoint->x < BOARDX && 0 <= pPoint->y && pPoint->y < BOARDY) {
        return true;
    }
    return false;
}


// generate cryptographically secure random number (seeded with time which is always changing)
int random_number(int min_num, int max_num) {
        int result = 0, low_num = 0, hi_num = 0;

        // validate parameters
        if (min_num < max_num)
        {
            low_num = min_num;
            hi_num = max_num + 1; // include max_num in output
        } else {
            low_num = max_num + 1; // include max_num in output
            hi_num = min_num;
        }

        //srand(time(NULL));  // !!!! seed random with time for cryptographically secure numbers !!!!
        result = (rand() % (hi_num - low_num)) + low_num;  // get result between bounds
        return result;
    }
