/*
 * This file will contain the student's implementations for P4 (Fall 2023)
 *
 * - isolated so that the other team can work on the game protocols
 *
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "tictactoe.h"

/*
 * Identical to lownet_crc()
 *
 * - there is no need to touch this!
 */
uint32_t crc24(const uint8_t* buf, size_t len)
{
	uint32_t reg = 0x00777777; // shift register
	static const uint32_t poly = 0x1800463ul;  // G(x)

	void process_byte(uint8_t b)
	{
		for(int i=0; i<8; i++)
			{
				reg = (reg<<1) | (b&1);
				b = b>>1;
				if ( reg & 0x1000000ul )
					reg = (reg^poly); // take mod G(x)
			}
	}

	for(size_t i = 0; i < len; ++i)
		process_byte(buf[i]);
	return reg;
}

uint32_t tictac_checksum(const tictactoe_t* board)
{
	return crc24((const uint8_t*) &board->data, sizeof(tictactoe_t));
}

/*
 * Milestone II:
 *
 * - Implement some super duper strategy for player s!
 * - Time budget is given in time_ms (milliseconds).
 * You can assume that it is in [2,10] seconds
 * - Specify the move by updating *x and *y (pointers)
 * - Return value 0 on making a succesful move (always!)
 */
typedef struct
{
	uint8_t x;
	uint8_t y;
} point_t;

typedef struct
{
	point_t top_left;
	uint8_t width;
	uint8_t height;
} attention_field_t;

uint8_t left_bound(const attention_field_t* field)
{
	return field->top_left.x;
}

uint8_t right_bound(const attention_field_t* field)
{
	return left_bound(field) + field->width;
}

uint8_t upper_bound(const attention_field_t* field)
{
	return field->top_left.y;
}

uint8_t lower_bound(const attention_field_t* field)
{
	return upper_bound(field) + field->height;
}

size_t field_squares(const attention_field_t* field)
{
	return (right_bound(field) - left_bound(field)) * (lower_bound(field) - upper_bound(field));
}

void extend_left(attention_field_t* field)
{
	field->top_left.x -= 1;
	field->width += 1;
}

void extend_right(attention_field_t* field)
{
	field->width += 1;
}

void extend_up(attention_field_t* field)
{
	field->top_left.y -= 1;
	field->height += 1;
}

void extend_down(attention_field_t* field)
{
	field->height += 1;
}

// Usage: square_free(BOARD, X, Y)
// Pre:   0 <= X < TICTACTOE_BOARD
//        0 <= Y < TICTACTOE_BOARD
// Value: true if square X,Y is free on BOARD
bool square_free(const tictactoe_t* board, uint8_t x, uint8_t y)
{
	return tictac_get(board, x, y) == EMPTY;
}

void field_expand_maybe(const tictactoe_t* board, attention_field_t* attention)
{
	size_t free_squares = 0;
	for (uint8_t i = upper_bound(attention), l = lower_bound(attention); i < l; ++i)
		for (uint8_t j = left_bound(attention), r = right_bound(attention); j < r; ++j)
			if (square_free(board, i, j))
				++free_squares;

	if ((float) free_squares / field_squares(attention) < 0.5)
		{
			if (left_bound(attention) != 0)
				extend_left(attention);
			if (right_bound(attention) != TICTACTOE_BOARD)
				extend_right(attention);
			if (upper_bound(attention) != 0)
				extend_up(attention);
			if (lower_bound(attention) != TICTACTOE_BOARD)
				extend_down(attention);
		}
}

void init_attention_field(const tictactoe_t* board, attention_field_t* attention)
{
	attention->width = 8;
	attention->height = 8;
	for (uint8_t i = 0; i < TICTACTOE_BOARD; ++i)
		{
			for (uint8_t j = 0; j < TICTACTOE_BOARD; ++j)
				{
					if (!square_free(board, i, j))
						{
							attention->top_left.x = (j == 0) ? j : j - 1;
							attention->top_left.y = (i == 0) ? i : i - 1;
							return;
						}
				}
		}
}

int tictac_move(const tictactoe_t* board, int* xp, int* yp, uint8_t s, uint32_t time_ms)
{
	/*
	 * TODO: figure out a better move!
	 */
	return tictac_auto(board, xp, yp, s);
}
