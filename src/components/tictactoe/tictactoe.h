#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <assert.h>

#define TICTACTOE_BOARD 30 // one side of the board
#define TICTACTOE_N (30*30) // number of squares
#define TICTACTOE_N2 225 // data size with 4 squares-per-byte
#define TICTACTOE_N3 180 // data size with 5 squares-per-byte (=30^2/5)

#define TICTAC_B2_SQUARES_PER_BYTE (TICTACTOE_N / TICTACTOE_N2)
#define TICTAC_B3_SQUARES_PER_BYTE (TICTACTOE_N / TICTACTOE_N3)

static_assert(TICTAC_B2_SQUARES_PER_BYTE == 4);
static_assert(TICTAC_B3_SQUARES_PER_BYTE == 5);

/*
 * Data invariant:
 *
 * A board is stored in memory as an array of (TICTACTOE_N2 = 225)
 * bytes, each byte stores 4 squares of the board, with 2 bits per
 * square.
 *
 * Each byte is laid out as follows:
 * 00|00|00|00
 *  ^  ^  ^  ^
 *  0  1  2  3
 */
typedef struct
{
	uint8_t data[TICTACTOE_N2];
} tictactoe_board_t;

typedef enum
{
	EMPTY = 0b00,
	PLAYER1 = 0b01,
	PLAYER2 = 0b10,
} square_value_t;

/*
 * Compact representation for data packets
 * - five squares per octet (base 3)
 */
typedef struct __attribute__((__packed__))
{
	uint8_t bdata[TICTACTOE_N3];
} tictactoe_payload_t;


int tictac_encode(const tictactoe_board_t* board, tictactoe_payload_t* p);
int tictac_decode(const tictactoe_payload_t* p, tictactoe_board_t* board);

int tictac_game_over(const tictactoe_board_t* board);
int tictac_auto(const tictactoe_board_t* b, int* x, int* y, uint8_t s);

int tictac_set(tictactoe_board_t* b, uint8_t i, uint8_t j, square_value_t s);
square_value_t tictac_get(const tictactoe_board_t* b, uint8_t i, uint8_t j);

#endif
