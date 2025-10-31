
#include <time.h>
#include <stdlib.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>

typedef struct {
    int grid[4][4];
    unsigned int score;
    int running;
} GameState;

void update_gamestate(GameState* state) {
    if(!state) return;

    int full = 1;
    for(int i = 0; i < 16; i++) {
        if(state->grid[i / 4][i % 4] == -1) {
            full = 0;
            break;
        }
    }
    if(full) {
        state->running = 0;
        return;
    }

    int new;
    do {
        new = rand() % 16;
    } while(state->grid[new / 4][new % 4] != -1);
    state->grid[new / 4][new % 4] = rand() % 2;
}

static void handle_left_shift(GameState* gs) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            /* Handle non-empty cells */
            if(gs->grid[i][j] >= 0) {
                for(int k = i; k > 0; k--) {
                    /* If it's empty, just shift it over */
                    if(gs->grid[k - 1][j] == -1) {
                        gs->grid[k - 1][j] = gs->grid[k][j];
                        gs->grid[k][j] = -1;
                    }
                    /* Collision cases */
                    else {
                        /* Combine blocks if they're the same value */
                        if(gs->grid[k - 1][j] == gs->grid[k][j]) {
                            gs->grid[k - 1][j]++;
                            gs->grid[k][j] = -1;
                            /* Preserves 2048-like scoring even with the display differences */
                            gs->score += (1 << gs->grid[k - 1][j]);
                        }
                        break;
                    }
                }
            }
        }
    }
}

static void handle_right_shift(GameState* gs) {
    for(int i = 3; i >= 0; i--) {
        for(int j = 0; j < 4; j++) {
            /* Handle non-empty cells */
            if(gs->grid[i][j] >= 0) {
                for(int k = i; k < 3; k++) {
                    /* If it's empty, just shift it over */
                    if(gs->grid[k + 1][j] == -1) {
                        gs->grid[k + 1][j] = gs->grid[k][j];
                        gs->grid[k][j] = -1;
                    }
                    /* Collision cases */
                    else {
                        /* Combine blocks if they're the same value */
                        if(gs->grid[k + 1][j] == gs->grid[k][j]) {
                            gs->grid[k + 1][j]++;
                            gs->grid[k][j] = -1;
                            /* Preserves 2048-like scoring even with the display differences */
                            gs->score += (1 << gs->grid[k + 1][j]);
                        }
                        break;
                    }
                }
            }
        }
    }
}

static void handle_up_shift(GameState* gs) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            /* Handle non-empty cells */
            if(gs->grid[j][i] >= 0) {
                for(int k = i; k > 0; k--) {
                    /* If it's empty, just shift it over */
                    if(gs->grid[j][k - 1] == -1) {
                        gs->grid[j][k - 1] = gs->grid[j][k];
                        gs->grid[j][k] = -1;
                    }
                    /* Collision cases */
                    else {
                        /* Combine blocks if they're the same value */
                        if(gs->grid[j][k - 1] == gs->grid[k][j]) {
                            gs->grid[j][k - 1]++;
                            gs->grid[j][k] = -1;
                            /* Preserves 2048-like scoring even with the display differences */
                            gs->score += (1 << gs->grid[j][k - 1]);
                        }
                        break;
                    }
                }
            }
        }
    }
}

static void handle_down_shift(GameState* gs) {
    for(int i = 3; i >= 0; i--) {
        for(int j = 0; j < 4; j++) {
            /* Handle non-empty cells */
            if(gs->grid[j][i] >= 0) {
                for(int k = i; k < 3; k++) {
                    /* If it's empty, just shift it over */
                    if(gs->grid[j][k + 1] == -1) {
                        gs->grid[j][k + 1] = gs->grid[j][k];
                        gs->grid[j][k] = -1;
                    }
                    /* Collision cases */
                    else {
                        /* Combine blocks if they're the same value */
                        if(gs->grid[j][k + 1] == gs->grid[k][j]) {
                            gs->grid[j][k + 1]++;
                            gs->grid[j][k] = -1;
                            /* Preserves 2048-like scoring even with the display differences */
                            gs->score += (1 << gs->grid[j][k + 1]);
                        }
                        break;
                    }
                }
            }
        }
    }
}

static void input_callback(InputEvent* event, void* ctx) {
    UNUSED(ctx);
    GameState* gs = ctx;

    int update = 0;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp:
            handle_up_shift(gs);
            update = 1;
            break;
        case InputKeyDown:
            handle_down_shift(gs);
            update = 1;
            break;
        case InputKeyLeft:
            handle_left_shift(gs);
            update = 1;
            break;
        case InputKeyRight:
            handle_right_shift(gs);
            update = 1;
            break;
        case InputKeyBack:
            gs->running = 0;
            break;
        default:
            break;
        }
    }
    if(update) {
        update_gamestate(gs);
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    GameState* gs = ctx;

    /* Draw the grid */
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            canvas_draw_frame(canvas, i * 16, j * 16, 16, 16);
        }
    }

    /* Draw in the values */
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(gs->grid[i][j] >= 0) {
                char buff[16] = "";
                snprintf(buff, sizeof(buff), "%d", gs->grid[i][j] + 1);
                canvas_draw_str(canvas, i * 16 + 7, j * 16 + 11, buff);
            }
        }
    }
}

void init_gamestate(GameState* state) {
    if(!state) return;
    uint32_t idxs[2];
    idxs[0] = rand() % 16;
    do {
        idxs[1] = rand() % 16;
    } while(idxs[1] == idxs[0]);

    for(uint32_t i = 0; i < 16; i++) {
        if(i == idxs[0] || i == idxs[1]) {
            int is_1 = rand() % 2;
            if(is_1) {
                state->grid[i / 4][i % 4] = 1;
            } else {
                state->grid[i / 4][i % 4] = 0;
            }
        } else {
            state->grid[i / 4][i % 4] = -1;
        }
    }
    state->running = 1;
    state->score = 0;
}

int32_t app_2048(void* p) {
    UNUSED(p);

    /* Open GUI */
    Gui* gui = furi_record_open(RECORD_GUI);

    /* Create ViewPort */
    ViewPort* viewport = view_port_alloc();
    GameState init;
    init_gamestate(&init);
    /* Set callbacks */
    view_port_draw_callback_set(viewport, draw_callback, &init);
    view_port_input_callback_set(viewport, input_callback, &init);

    /* Add ViewPort to GUI */
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    /* Keep running until user closes app */
    while(init.running) {
        view_port_update(viewport);
        furi_delay_ms(50);
    }

    /* Clean up */
    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close(RECORD_GUI);

    return 0;
}
