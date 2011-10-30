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
#include "scores.h" // header file with prototypes and declarations of the main functionality

// currently 5 games supported, 2k eeprom space should be enough
// if you have a big avr mcu with like 8k EEPROM, feel free to change this to 20
// and the "for loops" below
GameT EEMEM eep_games[5]; 

// read the scores/achievements for a specific game
GameT vis_score_read(const char *game) {
  int8_t game_index = -1; // location of game in EEPROM array of structs 
  GameT gametmp;
  for (uint8_t i = 0; i < 5; i++) {
      eeprom_read_block((void*)&gametmp, (const void*)&eep_games[i], sizeof(GameT));
      if (!strcmp(game, gametmp.name)) { // returns 0 when match found
        game_index = i;
        gameidx = i;
        return gametmp;
      } else if (strcmp(GAME_HWID, gametmp.id) != 0) { // free spot for empty initialization
        game_index = i;
      }
  }
  // not found, create empty
  if (game_index == -1) // no space, overwrite first game entry
    game_index = 0;
  vis_score_reset(game, game_index);
  return vis_score_read(game);
}

// writes the score/achievements back to the EEPROM
void vis_score_write(GameT game, int8_t game_index)
{
  eeprom_write_block((const void*)&game, (void*)&eep_games[game_index], sizeof(GameT));
}

// resets/inits the scores/achievements for a specific game
void vis_score_reset(const char *game, int8_t game_index)
{
  GameT gametmp;
  ScoreT scoretmp = {0, "unknown", 0};
  AchievT achievtmp = {"unknown", "unknown"};
  strcpy(gametmp.name, game);
  strcpy(gametmp.id, GAME_HWID);
  for (uint8_t i = 0; i < 10; i++) {
    gametmp.scores[i] = scoretmp;
    if (i < 6) {
      gametmp.achievements[i] = achievtmp;
    }
  }
  vis_score_write(gametmp, game_index);
}

