#include <iostream>
#include "memory"
#include <SDL_mixer.h>

#define SDL_MAIN_HANDLED

#include <SDL.h>

using namespace std;

int ballx = 420;
int bally = 500;
int ballvelx = 5;
int ballvely = -5;

int bkw = 800;
int bkh= 600;
int bkwmin = 0;
int bkhmin = 0;

int batx = bkw/2;
int baty = bkh - 30;
int batSpeed = 10;

int brickw = 80;
int brickh = 35;
int deleted_bricks = 20;
int number_of_bricks = 21;
SDL_Rect brickrect [3][7];
void InitializeBrick(){
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 7 ; ++j) {
            int x = 80 + j * 90;
            int y = 50 + i * 100;
            brickrect[i][j] = {x, y, brickw, brickh};
        }
    }
}

bool handle_events() {
    SDL_Event e;
    auto *key_state = SDL_GetKeyboardState(nullptr);
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT:
                return false;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_q) {
                    std::cout << "lag..." << std::endl;
                    SDL_Delay(500);
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
                break;
        }
    }
//    if (key_state[SDL_SCANCODE_UP]) rect.y--;
//    if (key_state[SDL_SCANCODE_DOWN]) rect.y++;
    if (key_state[SDL_SCANCODE_LEFT] && batx > 0 ) batx -= batSpeed;
    if (key_state[SDL_SCANCODE_RIGHT] && batx < bkw - 60 ) batx += batSpeed;

    return true;
}

void moveBall()
{
    ballx += ballvelx;
    bally += ballvely;
}

bool ball_brick_collision_detect(SDL_Rect  rect1, SDL_Rect rect2){
    if (SDL_HasIntersection(&rect1,&rect2)){
        return true;
    }else
        return false;
}
void ball_brick_collision(SDL_Rect ball){
    bool a;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            a = ball_brick_collision_detect(brickrect[i][j], ball);
            if (a){
                brickrect[i][j].x=3000;
                ballvely= -ballvely;
                deleted_bricks++;
            }a = false;
        }

    }
}

void ball_wall_colision(){
    if (ballx < bkwmin || ballx > bkw - 30){
        ballvelx = -ballvelx;
    }
    if (bally < bkhmin || bally > bkh - 30){
        ballvely = -ballvely;
    }
    int ballscaling = 25;
    if (bally + ballscaling >= baty && bally + ballscaling <= baty+30 && ballx + ballscaling >=batx && ballx + ballscaling <= batx + 70){
        ballvely = -ballvely;
    }
}

std::pair<std::shared_ptr<SDL_Window>, std::shared_ptr<SDL_Renderer>> create_context() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(
            800, 600,
            SDL_WINDOW_SHOWN, &window, &renderer);

    std::shared_ptr<SDL_Window> w(window, [](SDL_Window *window) {
        std::cout << "Destroying window" << std::endl;
        SDL_DestroyWindow(window);
    });
    std::shared_ptr<SDL_Renderer> r(renderer, [](SDL_Renderer *renderer) {
        std::cout << "Destroying renderer" << std::endl;
        SDL_DestroyRenderer(renderer);
    });
    return {w, r};
};

std::shared_ptr<SDL_Texture> load_texture(shared_ptr<SDL_Renderer> renderer, const std::string name) {
    auto surface = SDL_LoadBMP(("assets/" + name).c_str());
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0x0ff, 0x0, 0x0ff));

    auto texture = SDL_CreateTextureFromSurface(renderer.get(), surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return {texture, [](auto *p) { SDL_DestroyTexture(p); }};
}

void renderBricks(SDL_Renderer* renderer, SDL_Rect brick[][7]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            SDL_RenderFillRect(renderer, &brick[i][j]);
        }
    }
}
void win(SDL_Renderer* renderer){
  SDL_Surface *win = SDL_LoadBMP("./assets/cup.bmp");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_Texture *wintexture = SDL_CreateTextureFromSurface(renderer,win);
  SDL_Rect winrect = {300,200,200,200};
    SDL_RenderCopy(renderer,wintexture, nullptr,&winrect);
    SDL_RenderPresent(renderer);
    SDL_Delay(10000);
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    {
        auto [window_p, renderer_p] = create_context();
        SDL_SetRenderDrawColor(renderer_p.get(), 255, 100, 50, 255);
        InitializeBrick();
//        auto winner = load_texture(renderer_p,"clouds.bmp");
        auto background = load_texture(renderer_p, "bg_space_seamless.bmp");
        auto prev_tick = SDL_GetTicks();
        int frame_dropped = 0;
        while (handle_events()) {
            if (!frame_dropped) {

                SDL_Rect ballrect = {ballx,bally,20,30};
                moveBall();
                SDL_Rect batrect = {batx, baty, 60, 30};
                ball_wall_colision();
                ball_brick_collision(ballrect);
                if (deleted_bricks>= number_of_bricks){
                    win(renderer_p.get());
                }

                SDL_RenderCopy(renderer_p.get(), background.get(), nullptr, nullptr);
                SDL_RenderFillRect(renderer_p.get(),&ballrect );
                SDL_RenderFillRect(renderer_p.get(),&batrect );
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 7; j++) {
                        SDL_RenderFillRect(renderer_p.get(),&brickrect[i][j]);
                    }
                }
                SDL_RenderPresent(renderer_p.get());
                SDL_RenderClear(renderer_p.get());
                int ticks = SDL_GetTicks();
                if ((ticks - prev_tick) < 33) {
                    SDL_Delay(33 - (ticks - prev_tick));
                    frame_dropped = 0;
                } else { frame_dropped = 1; }
                prev_tick += 33;
            }
        }
        SDL_Quit();

        return 0;
    }
}