#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <assert.h>

#define TICTAC_B2_SQUARES_PER_BYTE 4
#define TICTAC_B3_SQUARES_PER_BYTE 5

#define TICTACTOE_BOARD 30 // one side of the board
#define TICTACTOE_N (TICTACTOE_BOARD * TICTACTOE_BOARD) // number of squares
#define TICTACTOE_N2 (TICTACTOE_N / TICTAC_B2_SQUARES_PER_BYTE) // data size with 4 squares-per-byte
#define TICTACTOE_N3 (TICTACTOE_N / TICTAC_B3_SQUARES_PER_BYTE) // data size with 5 squares-per-byte (=30^2/5)

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
 *  3  2  1  0
 */
typedef struct
{
	uint8_t data[TICTACTOE_N2];
} tictactoe_t;

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


int tictac_encode(const tictactoe_t* board, tictactoe_payload_t* p);
int tictac_decode(const tictactoe_payload_t* p, tictactoe_t* board);

int tictac_game_over(const tictactoe_t* board);
int tictac_auto(const tictactoe_t* b, uint8_t* x, uint8_t* y, uint8_t s);

int tictac_set(tictactoe_t* b, uint8_t i, uint8_t j, square_value_t s);
square_value_t tictac_get(const tictactoe_t* b, uint8_t i, uint8_t j);

#endif
