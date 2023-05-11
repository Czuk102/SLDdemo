#include <iostream>
#include "memory"
#include <SDL_mixer.h>

#define SDL_MAIN_HANDLED

#include <SDL.h>

using namespace std;

int ballVelocitX = 5;
int ballVelocitY = 5;
int ballX = 10;
int ballY = 10;
int bkw = 800;
int bkh = 600;
int bkwmin = 0;
int bkhmin = 0;
int ball_width = 20;
int ball_height = 20;

int platf_weight = 100;
int platf_height = 20;
int platf_speed = 10;

bool handle_events(SDL_Rect &rect) {
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
    if (key_state[SDL_SCANCODE_LEFT] && rect.x> bkwmin)  rect.x+= -platf_speed;
    if (key_state[SDL_SCANCODE_RIGHT] && rect.x<bkw - platf_weight) rect.x+= platf_speed;

    return true;
}

void moveBall(SDL_Rect &ball) {
    ball.x += ballVelocitX;
    ball.y += ballVelocitY;
}

void ball_collision(SDL_Rect &ball) {
    if (ball.x < bkwmin || ball.x > bkw - ball_width) {
        ballVelocitX = -ballVelocitX;
    }
    if (ball.y < bkhmin || ball.y > bkh - ball_height)
        ballVelocitY = -ballVelocitY;
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

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    {
        auto [window_p, renderer_p] = create_context();
        SDL_Rect ball = {ballX, ballY, ball_width, ball_height};
        SDL_Rect platform = {350, 550, platf_weight, platf_height};
        auto background = load_texture(renderer_p, "bg_space_seamless.bmp");
        auto prev_tick = SDL_GetTicks();
        int frame_dropped = 0;
        while (handle_events(platform)) {
            if (!frame_dropped) {
//                SDL_SetRenderDrawColor(renderer_p.get(), 0, 0, 0, 255);
//                SDL_RenderClear(renderer_p.get());
                SDL_RenderCopy(renderer_p.get(), background.get(), nullptr, nullptr);
                moveBall(ball);
                ball_collision(ball);
                SDL_SetRenderDrawColor(renderer_p.get(), 255, 100, 50, 255);
                SDL_RenderFillRect(renderer_p.get(), &ball);
                SDL_RenderFillRect(renderer_p.get(), &platform);
                SDL_RenderPresent(renderer_p.get());
                SDL_RenderClear(renderer_p.get());
            }
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
