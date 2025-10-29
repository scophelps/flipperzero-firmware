// app.c — Minimal "Dodge" game (educational skeleton)
// Place in applications_user/dodge_game/

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <input/input.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_W 128
#define SCREEN_H 64

// Game settings
#define PLAYER_W    8
#define PLAYER_H    4
#define ENEMY_W     8
#define ENEMY_H     4
#define MAX_ENEMIES 6
#define TICK_MS     120

typedef struct {
    int x, y;
    int active;
} Enemy;

typedef struct {
    int x, y;
    Enemy enemies[MAX_ENEMIES];
    int score;
    int running;
    ViewPort* view_port;
    ViewDispatcher* dispatcher;
} GameState;

static void spawn_enemy(GameState* g) {
    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(!g->enemies[i].active) {
            g->enemies[i].active = 1;
            g->enemies[i].x = rand() % (SCREEN_W - ENEMY_W);
            g->enemies[i].y = -ENEMY_H; // start above screen
            return;
        }
    }
}

static void update_game(GameState* g) {
    // move enemies down
    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(g->enemies[i].active) {
            g->enemies[i].y += 4; // speed
            if(g->enemies[i].y > SCREEN_H) {
                g->enemies[i].active = 0;
                g->score++;
            }
        }
    }
    // occasionally spawn
    if((rand() % 10) < 3) spawn_enemy(g);
}

static int check_collision(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return !(ax + aw < bx || bx + bw < ax || ay + ah < by || by + bh < ay);
}

static void draw_game(Canvas* canvas, void* ctx) {
    GameState* g = (GameState*)ctx;
    canvas_clear(canvas);

    // Draw header
    canvas_set_font(canvas, FontPrimary);
    char buf[32];
    snprintf(buf, sizeof(buf), "Score: %d", g->score);
    canvas_draw_str_aligned(canvas, SCREEN_W / 2, 6, AlignCenter, AlignCenter, buf);

    // Draw player
    canvas_draw_box(canvas, g->x, g->y, PLAYER_W, PLAYER_H);

    // Draw enemies
    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(g->enemies[i].active) {
            canvas_draw_box(canvas, g->enemies[i].x, g->enemies[i].y, ENEMY_W, ENEMY_H);
        }
    }
}

static bool input_handler(InputEvent* ev, void* ctx) {
    GameState* g = (GameState*)ctx;
    if(ev->type == InputTypeShort) {
        if(ev->key == InputKeyBack) {
            // exit the app
            view_dispatcher_stop(g->dispatcher);
            return 1;
        } else if(ev->key == InputKeyRight) {
            g->x += 8;
            if(g->x > SCREEN_W - PLAYER_W) g->x = SCREEN_W - PLAYER_W;
            view_port_update(g->view_port);
            return 1;
        } else if(ev->key == InputKeyLeft) {
            g->x -= 8;
            if(g->x < 0) g->x = 0;
            view_port_update(g->view_port);
            return 1;
        } else if(ev->key == InputKeyOk) {
            // restart if not running
            if(!g->running) {
                g->score = 0;
                memset(g->enemies, 0, sizeof(g->enemies));
                g->running = 1;
                view_port_update(g->view_port);
            }
            return 1;
        }
    }
    return 0;
}

static void game_loop(GameState* gs) {
    ViewPort* view_port = view_port_alloc();
    ViewDispatcher* dispatcher = view_dispatcher_alloc();
    gs->view_port = view_port;
    gs->dispatcher = dispatcher;
    View* view = view_alloc();
    view_set_draw_callback(view, draw_game);
    view_set_input_callback(view, input_handler);

    view_dispatcher_add_view(dispatcher, 0, view);

    // Start the game loop in the same thread — when dispatcher exits, we break out.
    // The dispatcher will process input and redraw requests internally.
    // Use view_dispatcher_run to start event processing (this blocks until exit).
    // NOTE: some SDK versions expect you to run the loop differently; consult docs if issues occur.
    view_dispatcher_run(dispatcher);

    // Basic tick loop — runs until dispatcher exits
    while(1) {
        furi_delay_ms(TICK_MS); // SDK's sleep function; adjust if your SDK differs
        if(!gs->running) continue;
        update_game(gs);

        // collision check
        for(int i = 0; i < MAX_ENEMIES; i++) {
            if(gs->enemies[i].active) {
                if(check_collision(
                       gs->x,
                       gs->y,
                       PLAYER_W,
                       PLAYER_H,
                       gs->enemies[i].x,
                       gs->enemies[i].y,
                       ENEMY_W,
                       ENEMY_H)) {
                    // game over
                    gs->running = 0;
                    break;
                }
            }
        }
        view_port_update(gs->view_port);
    }
    // cleanup (never reached if above loop is blocking; adapt per SDK)
    view_dispatcher_remove_view(dispatcher, 0);
    view_port_free(view_port);
    view_dispatcher_free(dispatcher);
}

int32_t dodge_app(void* p) {
    UNUSED(p);

    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.x = (SCREEN_W - PLAYER_W) / 2;
    gs.y = SCREEN_H - PLAYER_H - 6;
    gs.score = 0;
    gs.running = 1;

    game_loop(&gs);

    return 0;
}
