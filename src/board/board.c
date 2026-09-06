#include "gomoku.h"

const square	g_axes[AXIS_COUNT] = {
	1,
	GRID_SIZE,
	GRID_SIZE + 1,
	GRID_SIZE - 1,
};

const square	g_directions[DIRECTION_COUNT] = {
	1,
	-1,
	GRID_SIZE,
	-GRID_SIZE,
	GRID_SIZE + 1,
	-(GRID_SIZE + 1),
	GRID_SIZE - 1,
	-(GRID_SIZE - 1)
};

void
board_init(Board *board)
{
	int	x;
	int	y;

	memset(board->cells, CELL_OUT, sizeof(board->cells));
	y = 0;
	while (y < BOARD_SIZE)
	{
		x = 0;
		while (x < BOARD_SIZE)
		{
			board->cells[board_to_index(x, y)] = CELL_EMPTY;
			x++;
		}
		y++;
	}

	board->captured_stones[CELL_BLACK] = 0;
	board->captured_stones[CELL_WHITE] = 0;
	board->move_count = 0;
}

static void
record_move(Move *move, square _square, cell player,
	const square *captured, int captured_count)
{
	int	i;

	move->square = _square;
	move->cell = player;
	move->captured_count = (uint8_t)captured_count;
	i = 0;
	while (i < captured_count)
	{
		move->captured[i] = captured[i];
		i++;
	}
}

static void
remove_captured(Board *board, const Move *move)
{
	int	i;

	i = 0;
	while (i < move->captured_count)
	{
		board->cells[move->captured[i]] = CELL_EMPTY;
		i++;
	}
	board->captured_stones[move->cell] += move->captured_count;
}

bool
board_play(Board *board, square _square, cell player,
	const square *captured, int captured_count)
{
	Move	*move;

	if (player != CELL_BLACK && player != CELL_WHITE)
		return (false);
	if (board->move_count >= MAX_MOVES)
		return (false);
	if (captured_count < 0 || captured_count > MAX_CAPTURED_PER_MOVE)
		return (false);
	if (!board_is_free(board, _square))
		return (false);
	move = &board->history[board->move_count];
	record_move(move, _square, player, captured, captured_count);
	board->cells[_square] = player;
	remove_captured(board, move);
	board->move_count++;
	return (true);
}

static void
restore_captured(Board *board, const Move *move)
{
	cell	opponent;
	int		i;

	opponent = opponent_of(move->cell);
	i = 0;
	while (i < move->captured_count)
	{
		board->cells[move->captured[i]] = opponent;
		i++;
	}
	board->captured_stones[move->cell] -= move->captured_count;
}

bool
board_undo(Board *board)
{
	Move	*move;

	if (board->move_count == 0)
		return (false);
	board->move_count--;
	move = &board->history[board->move_count];
	board->cells[move->square] = CELL_EMPTY;
	restore_captured(board, move);
	return (true);
}

int
board_count_rays(const Board *board, square _square, square direction,
	cell player, int max_steps)
{
	square	cursor;
	int		count;

	cursor = _square;
	count = 0;
	while (count < max_steps)
	{
		cursor = (square)(cursor + direction);
		if (board_cell(board, cursor) != player)
			break ;
		count++;
	}
	return (count);
}

int
board_count_line(const Board *board, square _square, square axis,
	cell player)
{
	int	count;

	count = 1;
	count += board_count_rays(board, _square, axis, player, BOARD_SIZE - 1);
	count += board_count_rays(board, _square, (square)(-axis), player, BOARD_SIZE - 1);
	return (count);
}