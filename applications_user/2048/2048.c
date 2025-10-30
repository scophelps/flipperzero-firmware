
#include <furi.h>
#include <gui/gui.h>

typedef struct {
    int grid[4][4];
    unsigned int score;
} GameState;

static void handle_up_shift(GameState* gs) {
    /* Loop top to bottom */
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

static void handle_down_shift(GameState* gs) {
    /* Loop bottom to top */
    for(int i = 4; i >= 0; i--) {
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

static void handle_left_shift(GameState* gs) {
    /* Loop left to right */
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

static void handle_right_shift(GameState* gs) {
    /* Loop right to left */
    for(int i = 4; i < 0; i--) {
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

    if(event->type == InputTypePress) {
        switch(event->key) {
        case InputKeyUp:
            handle_up_shift(gs);
            break;
        case InputKeyDown:
            handle_down_shift(gs);
            break;
        case InputKeyLeft:
            handle_left_shift(gs);
            break;
        case InputKeyRight:
            handle_right_shift(gs);
            break;
        default:
            break;
        }
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    GameState* gs = ctx;

    /* Draw the grid */
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            canvas_draw_frame(canvas, i * 11, j * 11, 11, 11);
        }
    }

    /* Draw in the values */
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(gs->grid[i][j] >= 0) {
                char buff[16] = "";
                snprintf(buff, sizeof(buff), "%d", gs->grid[i][j] + 1);
                canvas_draw_str(canvas, i * 11, j * 11, buff);
            }
        }
    }
}

int32_t app_2048(void* p) {
    UNUSED(p);

    /* Open GUI */
    Gui* gui = furi_record_open(RECORD_GUI);

    /* Create ViewPort */
    ViewPort* viewport = view_port_alloc();

    /* Set callbacks */
    view_port_draw_callback_set(viewport, draw_callback, NULL);
    view_port_input_callback_set(viewport, input_callback, NULL);

    /* Add ViewPort to GUI */
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    /* Keep running until user closes app */
    while(1) {
        furi_delay_ms(50);
    }

    /* Clean up */
    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close(RECORD_GUI);

    return 0;
}
