/*
 * Frogger "Brugmania" Arcade version for Gameduino
 *
 * Based on example code of http://excamera.com/sphinx/gameduino/
 * by James Bowman
 *
 * Sprite Sheet credits go to GaryCXJk
 *
 * Totally recreated by by Bas Brugman
 * http:// www.visionnaire.nl 2011
 *
 * Original Frogger by Konami (c) 1981 / Distributed by SEGA
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _FROGGER_H_INCLUDED
#define _FROGGER_H_INCLUDED

// Proto's
static uint16_t atxy(byte x, byte y);
static void draw_lives(void);
static void draw_curtains(void);
static void draw_score(uint16_t dst, long n);
static int roadat(byte y, uint16_t tt);
static int riverat(byte y, uint16_t tt);
static void draw_general_sprite(uint16_t x, uint16_t y, byte anim, byte rot, byte jk);
static void draw_general_intro_sprite(uint16_t x, uint16_t y, byte anim, byte rot, byte jk);
static void draw_text(const char *text, uint16_t x, uint16_t y, uint16_t color, const byte align);
static void color_chars(unsigned int rgb, byte type);
static void set_cars(byte amount, byte anim, byte spacing, uint16_t x, byte y);
static void draw_cars(uint16_t x, byte y, byte anim, uint16_t interval);
static void set_turtles(byte amount, byte length, byte spacing, uint16_t x, byte y);
static void draw_turtles(uint16_t x, byte y, byte length, uint16_t interval, byte diving);
static void set_logs(byte amount, byte length, byte spacing, uint16_t x, byte y);
static void draw_logs(uint16_t x, byte y, byte length, uint16_t interval);
static void draw_croc(uint16_t x, byte y, uint16_t interval);
static void draw_log_snake(void);
static void draw_bank_snake(void);
static void draw_otter(void);
static void draw_female(void);
static void reset_names(void);
static void draw_highscore(void);
static void check_konami_code(void);
static void change_environment(void);
static void p1_init(void);
static void p2_init(void);
static void intro_start(void);
static void save_scores_start(void);
static void change_letters(byte player, byte *p_done, byte *current_letter, char *p_name);
static void game_start(void);
static void bank_clear(void);
static void screen_clear(void);
static void init_start(void);
static void init_intro(void);
static void init_game(void);
static void init_saving(void);
static void level_start(void);
static void squarewave(uint16_t freq, byte amp);
static void sound(void);
static byte location(uint16_t x, uint16_t y);
static void show_insect(void);
static void show_croc(void);
static void display_msg(void);
static void draw_level_timer(void);
static void msg_init(uint8_t *head, uint8_t *tail);
static uint8_t msg_full(uint8_t tail, const uint8_t size);
static uint8_t msg_empty(uint8_t head, uint8_t tail);
static void update_scores(int8_t *score_pos, byte *score_level, long *pscore);
static void check_score(long *score, byte *prev_score_divisor, byte *lives);
static void player_start(byte player);
static void check_scores(void);

// Game globals
static unsigned int t;              // global timer
static unsigned int level_timer;    // time left per level
static byte intro = 1;              // flag to check if intro runs
static byte saving;                 // flag to check if saving name runs
static byte game;                   // flag to check if game runs
static long hiscore;                // current high-score
static byte level;                  // max 255 levels, try it :)
static char levelstr[3];            // level string for display
static byte done[5];                // progress of homes filled
static byte homes[5] = { 24, 72, 120, 168, 216 }; // x-pos of homes
static uint16_t p1_frogx, p1_frogy; // screen position
static byte p1_leaping;             // 0 means not leaping, 1-8 animates the leap
static byte p1_frogface;            // which way is the frog facing, as a sprite ROT field
static byte p1_dying;               // 0 means not dying, 1-64 animation counter
static byte p1_done;                // player 1 has finished entering his name
static int8_t p1_score_pos = -1;    // player 1's position in top score list
static byte p1_current_letter;      // p1's current letter of name saving
static char p1_name[16];            // name of player 1 to be saved, 8 chars and with each char follows a NULL
static char p1_finalname[9];        // final name of player 1
static byte p1_touched;             // byte number of sprite that p1 touched
static byte p1_touching;            // bool to indicate touch of p1
static long p1_score;               // p1 current score
static byte p1_level;               // p1 level reached 
static byte p1_prev_score_divisor;  // p1 previous score check
static byte p1_lives;               // p1 lives left
static byte p1_achievs;             // p1 latests achievements
static uint8_t p1_bonustime;        // p1's time bonus
static uint16_t river_object_posx_p1; // x-pos of touched river object by p1
static uint16_t frog_p1_posx;         // x-pos of p1, when touching something
static byte deadly_river_object_p1; // sprite image number that p1 touched
static byte deadly_river_object_pal_p1; // sprite palette that p1 touched
static uint16_t river_sibling_object_posx_right_p1; // sprite on the right where p1 landed
static uint16_t river_sibling_object_posx_left_p1; // sprite on the left where p1 landed
static uint16_t p2_frogx, p2_frogy; // screen position
static byte p2_leaping;             // 0 means not leaping, 1-8 animates the leap
static byte p2_frogface;            // which way is the frog facing, as a sprite ROT field
static byte p2_dying;               // 0 means not dying, 1-64 animation counter
static byte p2_done;                // player 2 has finished entering his name
static int8_t p2_score_pos = -1;    // player 2's position in top score list
static byte p2_current_letter;      // p2's current letter of name saving
static char p2_name[16];            // name of player 2 to be saved, 8 chars with each char follows a NULL
static char p2_finalname[9];        // final name of player 2
static byte p2_touched;             // byte number of sprite that p2 touched
static byte p2_touching;            // bool to indicate touch of p2
static long p2_score;               // p2 current score
static byte p2_level;               // p2 level reached 
static byte p2_prev_score_divisor;  // p2 previous score check
static byte p2_lives;               // p2 lives left
static byte p2_achievs;             // p2 latests achievements
static uint8_t p2_bonustime;        // p2's time bonus
static uint16_t river_object_posx_p2; // x-pos of touched river object by p2
static uint16_t frog_p2_posx;         // x-pos of p2, when touching something
static byte deadly_river_object_p2; // sprite image number that p2 touched
static byte deadly_river_object_pal_p2; // sprite palette that p2 touched
static uint16_t river_sibling_object_posx_right_p2; // sprite on the right where p2 landed
static uint16_t river_sibling_object_posx_left_p2; // sprite on the left where p2 landed
static byte insectspr = 99;         // sprite pos of bonus insect
static byte bonus200spr = 97;       // sprite pos of bonus 200
static byte crocheadspr = 96;       // sprite pos of croc head in home
static byte snaketailspr = 94;      // sprite pos of snake's tail, head follows next slot
static byte snaketailspr2 = 92;     // sprite pos of snake's tail, head follows next slot
static byte otterspr = 91;          // sprite pos of otter
static byte otter;                  // otter appearance flag, also used for face direction
static uint16_t otterx;             // otter's current x pos
static uint16_t ottery;             // otter's current y pos
static byte otter_touched;          // sprite number which the otter touched
static byte snakelog;               // log number where a snake will crawl
static uint16_t snakelogx;          // log x pos where a snake will crawl
static byte croclog;                // log number that turns into a croc
static byte bonusx;                 // sprite xpos of showing bonus at home locations
static byte showinsect;             // show insect flag
static unsigned int showtime_insect;// insect show time
static unsigned int showtime_croc;  // croc show time
static byte insecthome;             // home of insect
static byte showcrochead;           // show crochead flag
static byte crocheadhome;           // home of crochead
static byte p1_female;              // flag when p1 is carrying a female frog
static byte p2_female;              // flag when p2 is carrying a female frog
static byte femalelog;              // log number of female frog
static uint16_t femalelogx;         // log x pos of female
static uint16_t femalex;            // x pos of female
static byte female_leaping;         // 0 means not leaping, 1-8 animates the leap
static byte frog_leaping;           // 0 means not leaping, 1-8 animates the leap
static uint16_t frog_introx = 440;  // starting x pos of frogs in intro
static uint16_t frog_introy = 144;  // starting y pos of frogs in intro
static byte female_spr = 100;       // sprite pos of female npc frogger
static byte p1_frogspr = 101;       // sprite pos of frogger player 1
static byte p2_frogspr = 102;       // sprite pos of frogger player 2
static byte femalecarry_spr = 103;  // sprite pos of carrying female npc frogger, this needs to be higher, so it shows on top of frogger
static byte frog_anim_green[]   = {2, 1, 0, 0, 2}; // sprite slots, jumping, player 1 green frog
static byte frog_anim_purple[]  = {22, 21, 20, 20, 22}; // sprite slots, jumping, player 2 frog, or godmode in 1-player mode
static byte frog_anim_pink[]    = {12, 11, 10, 10, 12}; // sprite slots, jumping, female pink frog
static byte die_drown_anim[]    = {31, 32, 33, 30}; // sprite slots, dying drowning
static byte die_splash_anim[]   = {40, 41, 42, 30}; // sprite slots, dying splash
static byte diving_anim[] = {50, 51, 52, 53, 54, 55, 54, 53}; // diving sprite animation of turtles
static byte caramountrand[5];      // random bytes calculated each level to change amount of cars per lane
static byte carspacerand[5];       // random bytes calculated each level to change space between cars per lane
static byte riveramountrand[5];    // random bytes calculated each level to change amount of river objects per lane
static byte riverspacerand[5];     // random bytes calculated each level to change space between river objects per lane
static byte turtledive1;           // random number of turtle group that dives on lane 1
static byte turtledive2;           // random number of turtle group that dives on lane 4
static byte speed_factor;          // speed increases every 6 levels, also lessens the amount of river objects
static byte msg_head;              // head pointer of queue, first to leave
static byte msg_tail;              // tail pointer of queue, last in row
static byte konami_code;           // checking bit fields for cheat mode enabling
static byte godmode;               // god mode, you can't die anymore
static byte gamemode;              // game mode, 0 is one-player, 1 is two-player 
static byte achiev[6];             // the 6 achievement byte flags
static byte displaymsg;            // displaying of message flag
static byte joy_p1;                // joystick status player 1
static byte btn_p1;                // button status player 1
static byte joy_p2;                // joystick status player 2
static byte btn_p2;                // button status player 2

typedef struct msg_queue {
  byte display_time;
  char display_text[33];
} MsgT;

static MsgT msg_dequeue(MsgT *q, uint8_t *head);
static void msg_enqueue(MsgT *q, uint8_t *tail, MsgT msg);
static MsgT current_msg; // current message

#define MSG_SIZE      64           // max messages to enqueue for display, resets every level
static MsgT msgs[MSG_SIZE];
static ScoreT scores_updated[10];  // updated top score list: p1 and/or p2 scores at some pos

// midi frequency table
static PROGMEM prog_uint16_t midifreq[128] = {
32,34,36,38,41,43,46,48,51,55,58,61,65,69,73,77,82,87,92,97,103,110,116,123,130,138,146,155,164,174,184,195,207,220,233,246,261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987,1046,1108,1174,1244,1318,1396,1479,1567,1661,1760,1864,1975,2093,2217,2349,2489,2637,2793,2959,3135,3322,3520,3729,3951,4186,4434,4698,4978,5274,5587,5919,6271,6644,7040,7458,7902,8372,8869,9397,9956,10548,11175,11839,12543,13289,14080,14917,15804,16744,17739,18794,19912,21096,22350,23679,25087,26579,28160,29834,31608,33488,35479,37589,39824,42192,44701,47359,50175
};
#define MIDI(n) pgm_read_word(midifreq + (n))

// these background character codes correspond with the results of the online encoder: http://gameduino.com/results/5f30d40b/
#define BG_BLACK      34 + 128  // black char
#define BG_GREEN      37 + 128  // green timer char
#define BG_RED        42 + 128  // red timer char
#define BG_ZERO       12 + 128  // the scoring number '0'
#define BG_BLUE        0 + 128  // blue char
#define BG_LIFE       35 + 128  // lives char

// locations of the frog
#define LOC_BOTTOM_BANK 0 
#define LOC_MIDDLE_BANK 1 
#define LOC_ROAD        2 
#define LOC_RIVER       4 
#define LOC_HOME        8 
#define LOC_EDGE        16 
#define LOC_UNKNOWN     32 

// game info
#define GAME_NAME         "frogger"
#define GAME_VERSION      "1.0"
#define GAME_PLAYER1       0
#define GAME_PLAYER2       1
#define GAME_NAMEP1       "player 1"
#define GAME_NAMEP2       "player 2" 
#define GAME_LVLTXT       "LEVEL"
#define GAME_LVLTXTLOW    "level"
#define GAME_CREDIT1      "Brugmania Arcade / Bas Brugman / 2011"
#define GAME_CREDIT2      "Konami (c) / 1981"
#define GAME_BTNINFO      "1-up: middle btn / 2-up: bottom btn"
#define GAME_OVER         "GAME OVER"
#define GAME_OVER_P1      "GAME OVER FOR PLAYER 1"
#define GAME_OVER_P2      "GAME OVER FOR PLAYER 2"
#define GAME_GO           "START"
#define GAME_TIMEUP       "TIME UP"
#define GAME_TIMEBONUS    "TIME BONUS "
#define GAME_EXTRA        "EXTRA LIFE"
#define GAME_FEMALEBONUS  "FEMALE BONUS"
#define GAME_INSECTBONUS  "INSECT BONUS"
#define GAME_HISCORES     "HIGH SCORES"
#define GAME_ACHIEV       "LATEST ACHIEVEMENTS"
#define GAME_POINTTABLE   "POINT TABLE"
#define GAME_ACHIEVINFO   "ACHIEVEMENTS"
#define GAME_DIEINFO      "WAYS TO DIE"
#define GAME_GODMODE      "GOD MODE"
#define GAME_DONE         "done"
#define GAME_TEXTTIME      216 
#define GAME_TEXTTIMESHORT 100 
#define GAME_GRATS        "Congratulations!"
#define GAME_TOPLIST      "Enter your name, save with middle button."
#define GAME_POINTS1      "10 points per forward leap"
#define GAME_POINTS2      "50 points per home savely"
#define GAME_POINTS3      "200 points per bonus insect"
#define GAME_POINTS4      "200 points per female frog"
#define GAME_POINTS5      "10 points x remaining timer seconds"
#define GAME_POINTS6      "1000 points per completed level"
#define GAME_POINTS7      "5000 points per achievement"
#define GAME_POINTS8      "EVERY 20000 points an extra life!"
static const char* game_points[] = {GAME_POINTS1, GAME_POINTS2, GAME_POINTS3, GAME_POINTS4, GAME_POINTS5, GAME_POINTS6, GAME_POINTS7, GAME_POINTS8};
#define GAME_ACHIEV1      "THE RIGHT WAY"
#define GAME_ACHIEV2      "fill all home slots from left to right"
#define GAME_ACHIEV3      "SPEEDY GONZALES"
#define GAME_ACHIEV4      "complete a level in less than 60 seconds"
#define GAME_ACHIEV5      "LARRY LAFFER STYLE"
#define GAME_ACHIEV6      "save 5 lady frogs in one level"
#define GAME_ACHIEV7      "THE HUNGRY PIG"
#define GAME_ACHIEV8      "eat 5 insects in one level"
#define GAME_ACHIEV9      "CALLING ME CHICKEN?"
#define GAME_ACHIEV10     "stay foot until the timer turns red and score!"
#define GAME_ACHIEV11     "TOO MUCH SEINFELD"
#define GAME_ACHIEV12     "get hit by a truck 2 times in one level"
static const char* game_achievs[] = {GAME_ACHIEV1, GAME_ACHIEV2, GAME_ACHIEV3, GAME_ACHIEV4, GAME_ACHIEV5, GAME_ACHIEV6, GAME_ACHIEV7, GAME_ACHIEV8, \
  GAME_ACHIEV9, GAME_ACHIEV10, GAME_ACHIEV11, GAME_ACHIEV12};
#define GAME_DIE1      "running into road vehicles"
#define GAME_DIE2      "jumping into the river's water"
#define GAME_DIE3      "running into snakes, crocs and otters"
#define GAME_DIE4      "staying on top of a diving turtle too long"
#define GAME_DIE5      "drifting off the screen"
#define GAME_DIE6      "jumping into a home already occupied by a frog"
#define GAME_DIE7      "jumping into the side of a home"
#define GAME_DIE8      "running against the side/bottom edges"
#define GAME_DIE9      "running out of time"
static const char* game_deaths[] = {GAME_DIE1, GAME_DIE2, GAME_DIE3, GAME_DIE4, GAME_DIE5, GAME_DIE6, GAME_DIE7, GAME_DIE8, GAME_DIE9};

// colors
#define COLOR_RED     RGB(255, 0, 0)
#define COLOR_BLUE    RGB(0, 0, 255)
#define COLOR_GREEN   RGB(0, 255, 0)
#define COLOR_YELLOW  RGB(255, 255, 0)
#define COLOR_WHITE   RGB(255, 255, 255)
#define COLOR_BLACK   RGB(0, 0, 0)

// misc: aligning and coloring text
#define ALIGN_X       0x00 
#define ALIGN_CENTER  0x01 
#define STR_MASK      "        "
#define COLOR_ALL     0x00 
#define COLOR_UPCASE  0x01 
#define COLOR_LOWERCASE 0x02 

#endif

