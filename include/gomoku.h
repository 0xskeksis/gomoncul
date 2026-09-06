#ifndef _GOMOKU_H
# define _GOMOKU_H

# include <stdint.h>
# include <stdbool.h>
# include <stdio.h>
# include <string.h>

# define BOARD_SIZE		19
# define BORDER_SIZE	1
# define GRID_SIZE		(BOARD_SIZE + 2 * BORDER_SIZE)
# define GRID_CELLS		(GRID_SIZE * GRID_SIZE)
# define BOARD_AREA		(BOARD_SIZE * BOARD_SIZE)

# define WIN_ALIGNMENT	5
# define WIN_CAPTURED	10

# define AXIS_COUNT			4
# define DIRECTION_COUNT	(AXIS_COUNT * 2)

// Captured cells can be played again, so a game
// can have more moves than the number of cells on the board.
# define MAX_MOVES				(BOARD_AREA + 2 * WIN_CAPTURED)

// One stone can capture at most two stones in each direction
# define MAX_CAPTURED_PER_MOVE	(DIRECTION_COUNT * 2)

typedef uint8_t	cell;

// A square represents a position on the grid, not an (x, y) pair.
typedef int16_t	square;

enum
{
	CELL_BLACK = 0,
	CELL_WHITE = 1,
	CELL_EMPTY = 2,
	CELL_OUT = 3 // Outside the board
};

typedef struct
{
	square		square;
	cell		cell;
	uint8_t		captured_count;
	square		captured[MAX_CAPTURED_PER_MOVE];
}	Move;

typedef struct
{
	cell	cells[GRID_CELLS];
	uint8_t	captured_stones[2];
	Move	history[MAX_MOVES];
	int		move_count;
}	Board;

extern const square	g_axes[AXIS_COUNT];
extern const square	g_directions[DIRECTION_COUNT];

static inline bool
board_in_bounds(int x, int y)
{
	return (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE);
}

// Convert board coordinates (x, y) to a linear index in the grid array.
// right is +1,
// down is +GRID_SIZE
// up-left is -GRID_SIZE - 1
static inline square
board_to_index(int x, int y)
{
	return ((square)((y + BORDER_SIZE) * GRID_SIZE + (x + BORDER_SIZE)));
}

static inline int
board_x(square _square)
{
	return (int)(_square % GRID_SIZE) - BORDER_SIZE;
}

static inline int
board_y(square _square)
{
	return (int)(_square / GRID_SIZE) - BORDER_SIZE;
}

static inline cell
board_cell(const Board *board, square _square)
{
	return board->cells[_square];
}

static inline bool
board_is_free(const Board *board, square _square)
{
	cell	c;

	c = board_cell(board, _square);
	return (c == CELL_EMPTY);
}

static inline bool
board_is_out(const Board *board, square _square)
{
	cell	c;

	c = board_cell(board, _square);
	return (c == CELL_OUT);
}

static inline bool
board_has_stone(const Board *board, square _square)
{
	cell	c;

	c = board_cell(board, _square);
	return (c == CELL_BLACK || c == CELL_WHITE);
}

// Get the cell at a certain number of steps away from the origin in the given direction.
static inline cell
board_cell_at(const Board *board, square origin, square direction, int steps)
{
	square	cursor;

	cursor = origin;
	while (steps > 0)
	{
		cursor = (square)(cursor + direction);
		if (board_is_out(board, cursor))
			return (CELL_OUT);
		steps--;
	}

	return (board_cell(board, cursor));
}

// Only valid for CELL_BLACK and CELL_WHITE.
// XOR with 1 will flip between CELL_BLACK (0) and CELL_WHITE (1).
static inline cell
opponent_of(cell c)
{
	return (c ^ 1);
}

static inline const Move
*board_last_move(const Board *board)
{
	if (board->move_count == 0)
		return (NULL);
	return (&board->history[board->move_count - 1]);
}

void	demo_start(void);
void	board_init(Board *board);
bool	board_play(Board *board, square _square, cell c,
			const square *captured, int captured_count);
bool	board_undo(Board *board);
int		board_count_rays(const Board *board, square _square, square direction,
			cell player, int max_steps);
int		board_count_line(const Board *board, square _square, square axis,
			cell player);

#endif
