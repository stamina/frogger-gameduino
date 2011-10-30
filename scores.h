/*
 * Scores & Achievement library for the Brugmania Arcade Cabinet
 *
 * Created by Bas Brugman
 * http:// www.visionnaire.nl 2011
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
#ifndef _SCORES_H_INCLUDED
#define _SCORES_H_INCLUDED

#include <avr/eeprom.h> // avr-glibc EEPROM functionality (Electronically Erasable Read-Only Memory)
#include <string.h> // avr's string functions

#define GAME_HWID "brugmania"

typedef struct SHighScore {
  long points; // player score
  char player[9]; // player name, max 8 char + 0x00
  uint8_t level; // level reached, if applicable
} ScoreT;

typedef struct SAchiev {
  char name[33]; // achievement name, max 32 char + 0x00
  char player[9]; // player name, max 8 char + 0x00
} AchievT;

typedef struct SGames {
  char name[9]; // game title
  char id[10]; // game platform id
  ScoreT scores[10]; // top 10 score structs
  AchievT achievements[6]; // latest 6 achievement structs
} GameT;

GameT scoreinfo; // game info that the specific game will use and probably manipulate
uint8_t gameidx; // index of game in EEPROM struct array

// proto's
GameT vis_score_read(const char *game);
void vis_score_reset(const char *game, int8_t game_index);
void vis_score_write(GameT game, int8_t game_index);

#endif

