#include "gomoku.h"

#define FREE_THREE_COUNT    3
#define PATTERN_MAX_LEN     6

static const int g_free_threes[FREE_THREE_COUNT][PATTERN_MAX_LEN + 1] = {
    {5, 0, 1, 1, 1, 0, 0},
    {6, 0, 1, 1, 0, 1, 0},
    {6, 0, 1, 0, 1, 1, 0}
};

const square g_axes[AXIS_COUNT] = {
    1,
    GRID_SIZE,
    GRID_SIZE + 1,
    GRID_SIZE - 1
};

const square g_directions[DIRECTION_COUNT] = {
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
    int x;
    int y;
    square pos;

    memset(board, 0, sizeof(*board));

    for (y = 0; y < GRID_SIZE; y++)
    {
        for (x = 0; x < GRID_SIZE; x++)
        {
            pos = (square)(y * GRID_SIZE + x);

            if (x == 0 || x == GRID_SIZE - 1 ||
                y == 0 || y == GRID_SIZE - 1)
                board->cells[pos] = CELL_OUT;
            else
                board->cells[pos] = CELL_EMPTY;
        }
    }
}

int
board_count_rays(const Board *board,
                 square _square,
                 square direction,
                 cell player,
                 int max_steps)
{
    int count;
    square cursor;

    count = 0;
    cursor = _square;

    while (count < max_steps)
    {
        cursor = (square)(cursor + direction);

        if (board_is_out(board, cursor))
            break;

        if (board_cell(board, cursor) != player)
            break;

        count++;
    }

    return count;
}

int
board_count_line(const Board *board,
                 square _square,
                 square axis,
                 cell player)
{
    int count;

    count = 1;

    count += board_count_rays(
        board,
        _square,
        axis,
        player,
        BOARD_SIZE
    );

    count += board_count_rays(
        board,
        _square,
        -axis,
        player,
        BOARD_SIZE
    );

    return count;
}

static int
find_captures(const Board *board,
              square pos,
              cell player,
              square *captured)
{
    int count;
    int i;
    square dir;
    square a;
    square b;
    square c;

    count = 0;

    for (i = 0; i < DIRECTION_COUNT; i++)
    {
        dir = g_directions[i];

        a = (square)(pos + dir);
        b = (square)(pos + dir * 2);
        c = (square)(pos + dir * 3);

        if (board_is_out(board, a) ||
            board_is_out(board, b) ||
            board_is_out(board, c))
            continue;

        if (board_cell(board, a) != opponent_of(player))
            continue;

        if (board_cell(board, b) != opponent_of(player))
            continue;

        if (board_cell(board, c) != player)
            continue;

        captured[count++] = a;
        captured[count++] = b;
    }

    return count;
}

static void
remove_captures(Board *board,
                const square *captured,
                int captured_count)
{
    int i;

    for (i = 0; i < captured_count; i++)
        board->cells[captured[i]] = CELL_EMPTY;
}

static void
restore_captures(Board *board,
                 const square *captured,
                 int captured_count,
                 cell player)
{
    int i;

    for (i = 0; i < captured_count; i++)
        board->cells[captured[i]] = opponent_of(player);
}

static bool
has_alignment(const Board *board,
              square pos,
              cell player)
{
    int i;

    for (i = 0; i < AXIS_COUNT; i++)
    {
        if (board_count_line(
                board,
                pos,
                g_axes[i],
                player) >= WIN_ALIGNMENT)
            return true;
    }

    return false;
}

static bool
has_capture_win(const Board *board,
                cell player)
{
    return (board->captured_stones[player] >= WIN_CAPTURED);
}

static bool
match_free_three(const Board *board,
                 square start,
                 square direction,
                 square played,
                 cell player,
                 const int *pattern)
{
    int len;
    int i;
    square cursor;
    cell expected;

    len = pattern[0];
    cursor = start;

    for (i = 0; i < len; i++)
    {
        if (cursor == played && pattern[i + 1] != 1)
            return false;

        if (pattern[i + 1] == 0)
            expected = CELL_EMPTY;
        else
            expected = player;

        if (board_is_out(board, cursor))
            return false;

        if (board_cell(board, cursor) != expected)
            return false;

        cursor = (square)(cursor + direction);
    }

    return true;
}

static int
count_free_threes(const Board *board,
                  square pos,
                  cell player)
{
    int count;
    int axis;
    int pattern;
    int offset;
    int len;
    square start;

    count = 0;

    for (axis = 0; axis < AXIS_COUNT; axis++)
    {
        for (pattern = 0; pattern < FREE_THREE_COUNT; pattern++)
        {
            len = g_free_threes[pattern][0];

            for (offset = 0; offset < len; offset++)
            {
                start = (square)(
                    pos - g_axes[axis] * offset
                );

                if (match_free_three(
                        board,
                        start,
                        g_axes[axis],
                        pos,
                        player,
                        g_free_threes[pattern]))
                {
                    count++;
                    break;
                }
            }
        }
    }

    return count;
}

bool
is_legal_move(const Board *board,
              square pos,
              cell player)
{
    Board copy;
    square captured[MAX_CAPTURED_PER_MOVE];
    int captured_count;
    int free_three_count;

    if (player != CELL_BLACK && player != CELL_WHITE)
        return false;

    if (pos < 0 || pos >= GRID_CELLS)
        return false;

    if (!board_is_free(board, pos))
        return false;

    memcpy(&copy, board, sizeof(copy));

    captured_count = find_captures(
        &copy,
        pos,
        player,
        captured
    );

    copy.cells[pos] = player;

    remove_captures(
        &copy,
        captured,
        captured_count
    );

    free_three_count = count_free_threes(
        &copy,
        pos,
        player
    );

    if (free_three_count >= 2)
        return false;

    return true;
}

static bool
capture_breaks_alignment(const Board *board,
                         square pos,
                         cell player,
                         const square *captured,
                         int captured_count,
                         square winning_pos)
{
    Board copy;
    int i;

    memcpy(&copy, board, sizeof(copy));

    copy.cells[pos] = player;

    for (i = 0; i < captured_count; i++)
        copy.cells[captured[i]] = CELL_EMPTY;

    if (!board_has_stone(&copy, winning_pos))
        return true;

    if (copy.cells[winning_pos] != opponent_of(player))
        return true;

    return !has_alignment(
        &copy,
        winning_pos,
        opponent_of(player)
    );
}

static bool
can_break_alignment(const Board *board,
                    square winning_pos,
                    cell winner)
{
    cell opponent;
    int x;
    int y;
    square pos;
    square captured[MAX_CAPTURED_PER_MOVE];
    int captured_count;
    Board copy;

    opponent = opponent_of(winner);

    for (y = 0; y < BOARD_SIZE; y++)
    {
        for (x = 0; x < BOARD_SIZE; x++)
        {
            pos = board_to_index(x, y);

            if (!board_is_free(board, pos))
                continue;

            memcpy(&copy, board, sizeof(copy));

            captured_count = find_captures(
                &copy,
                pos,
                opponent,
                captured
            );

            if (captured_count == 0)
                continue;

            if (capture_breaks_alignment(
                    board,
                    pos,
                    opponent,
                    captured,
                    captured_count,
                    winning_pos))
                return true;
        }
    }

    return false;
}

static bool
can_capture_pair(const Board *board,
                 cell player)
{
    int x;
    int y;
    square pos;
    square captured[MAX_CAPTURED_PER_MOVE];

    for (y = 0; y < BOARD_SIZE; y++)
    {
        for (x = 0; x < BOARD_SIZE; x++)
        {
            pos = board_to_index(x, y);

            if (!board_is_free(board, pos))
                continue;

            if (find_captures(
                    board,
                    pos,
                    player,
                    captured) >= 2)
                return true;
        }
    }

    return false;
}

static bool
is_alignment_winnable(const Board *board,
                      square pos,
                      cell player)
{
    return !can_break_alignment(
        board,
        pos,
        player
    );
}

cell
board_winner(const Board *board)
{
    int x;
    int y;
    int player;
    square pos;

    for (player = CELL_BLACK; player <= CELL_WHITE; player++)
    {
        if (has_capture_win(board, (cell)player))
            return (cell)player;
    }

    for (y = 0; y < BOARD_SIZE; y++)
    {
        for (x = 0; x < BOARD_SIZE; x++)
        {
            pos = board_to_index(x, y);

            if (board_cell(board, pos) != CELL_BLACK &&
                board_cell(board, pos) != CELL_WHITE)
                continue;

            if (!has_alignment(
                    board,
                    pos,
                    board_cell(board, pos)))
                continue;

            if (is_alignment_winnable(
                    board,
                    pos,
                    board_cell(board, pos)))
                return board_cell(board, pos);
        }
    }

    return CELL_EMPTY;
}

bool
board_undo(Board *board)
{
    Move *move;
    int i;

    if (board->move_count <= 0)
        return false;

    move = &board->history[board->move_count - 1];

    board->cells[move->square] = CELL_EMPTY;

    for (i = 0; i < move->captured_count; i++)
        board->cells[move->captured[i]] =
            opponent_of(move->cell);

    board->captured_stones[move->cell] -=
        move->captured_count;

    board->move_count--;

    return true;
}

bool
board_play(Board *board,
           square _square,
           cell c,
           const square *captured,
           int captured_count)
{
    Move *move;
    square found[MAX_CAPTURED_PER_MOVE];
    int found_count;
    int free_three_count;
    int i;

    if (c != CELL_BLACK && c != CELL_WHITE)
        return false;

    if (_square < 0 || _square >= GRID_CELLS)
        return false;

    if (!board_is_free(board, _square))
        return false;

    if (board->move_count >= MAX_MOVES)
        return false;

    found_count = find_captures(
        board,
        _square,
        c,
        found
    );

    board->cells[_square] = c;

    remove_captures(
        board,
        found,
        found_count
    );

    /*
     * TODO:
     * No double-three.
     */

    free_three_count = count_free_threes(
        board,
        _square,
        c
    );

    if (free_three_count >= 2)
    {
        restore_captures(
            board,
            found,
            found_count,
            c
        );

        board->cells[_square] = CELL_EMPTY;

        return false;
    }

    move = &board->history[board->move_count];

    move->square = _square;
    move->cell = c;
    move->captured_count = (uint8_t)found_count;

    for (i = 0; i < found_count; i++)
        move->captured[i] = found[i];

    board->captured_stones[c] +=
        (uint8_t)found_count;

    board->move_count++;

    (void)captured;
    (void)captured_count;

    /*
     * TODO:
     * Endgame capture.
     */

    if (has_capture_win(board, c))
        return true;

    if (has_alignment(board, _square, c))
    {
        if (is_alignment_winnable(board, _square, c))
            return true;
    }

    if (board->captured_stones[opponent_of(c)] >= WIN_CAPTURED - 2)
    {
        if (can_capture_pair(board, c))
            return true;
    }

    return true;
}
