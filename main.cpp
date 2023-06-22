#include <iostream>
#include <memory>
#include <SDL_mixer.h>
#include <cmath>
#define SDL_MAIN_HANDLED
#include <SDL.h>

Mix_Music* gMusic = nullptr;
Mix_Chunk* bounce = nullptr;

void quit();

using namespace std;

int ballX = 420;
int ballY = 500;
float ballVelocitX = 7;
float ballVelocityY = -7;
int ballwidth = 30;
int ballheight = 30;

int bkw = 800;
int bkh = 600;
int bkwmin = 0;
int bkhmin = 0;

int batX = bkw / 2;
int batY = bkh - 10;
int batSpeed = 10;

int brickw = 80;
int brickh = 35;
int deleted_bricks = 0;
int number_of_bricks = 21;
SDL_Rect brickrect[3][7];

void InitializeBrick()
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            int x = 80 + j * 90;
            int y = 50 + i * 100;
            brickrect[i][j] = {x, y, brickw, brickh};
        }
    }
}


bool handle_events()
{
    SDL_Event e;
    auto *key_state = SDL_GetKeyboardState(nullptr);
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT:{}
                std::cout << "lag..." << std::endl;
                return false;


            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_q)
                {
                    std::cout << "lag..." << std::endl;
                    SDL_Delay(500);
                }
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    return false;
                }
                break;
        }
    }
    if (key_state[SDL_SCANCODE_LEFT] && batX > 0)
        batX -= batSpeed;
    if (key_state[SDL_SCANCODE_RIGHT] && batX < bkw - 60)
        batX += batSpeed;

    return true;
}

void moveBall()
{
    ballX += ballVelocitX;
    ballY += ballVelocityY;
}

bool ball_brick_collision_detect(SDL_Rect &rect1, SDL_Rect &rect2)
{
    if (SDL_HasIntersection(&rect1, &rect2))
    {
        return true;
    }
    else
        return false;
}

void ball_brick_collision(SDL_Rect& ball)
{
    bool collisionDetected = false;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            SDL_Rect& brick = brickrect[i][j];
            if (SDL_HasIntersection(&brick, &ball))
            {
                int ballCenterX = ball.x + ball.w / 2;
                int ballCenterY = ball.y + ball.h / 2;

                int brickCenterX = brick.x + brick.w / 2;
                int brickCenterY = brick.y + brick.h / 2;

                int distX = ballCenterX - brickCenterX;
                int distY = ballCenterY - brickCenterY;

                int combinedHalfWidths = (ball.w + brick.w) / 2;
                int combinedHalfHeights = (ball.h + brick.h) / 2;

                int absDistX = std::abs(distX);
                int absDistY = std::abs(distY);

                if (absDistX <= combinedHalfWidths && absDistY <= combinedHalfHeights)
                {
                    // Collision occurred
                    if (absDistX >= absDistY)
                    {
                        // Horizontal collision
                        ballVelocitX *= -1; // Invert horizontal velocity


                    }
                    else
                    {
                        // Vertical collision
                        ballVelocityY *= -1; // Invert vertical velocity


                    }
                    Mix_PlayChannel( -1, bounce, 0 );
                    collisionDetected = true;
                    brick.x = 3000; // Remove brick from the game
                    deleted_bricks++;

                } if (collisionDetected){
                    break;
                }

            }
        }
    }
}

void closeSound(){
    Mix_FreeChunk( bounce );
    bounce = nullptr;
    Mix_Quit();
    std::cout<<"Destroy sound\n";
}


void game_over(SDL_Renderer *renderer)
{
    SDL_Surface *over = SDL_LoadBMP("./assets/game_over.bmp");
    SDL_RenderClear(renderer);

    SDL_Texture *wintexture = SDL_CreateTextureFromSurface(renderer, over);
    SDL_Rect winrect = {300, 200, 200, 200};
    SDL_RenderCopy(renderer, wintexture, nullptr, &winrect);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
    SDL_Quit();
}


bool ball_wall_collision(SDL_Renderer *renderer,SDL_Rect &ball, SDL_Rect &bat)
{
    if (ballX < bkwmin || ballX > bkw - 30)
    {
        ballVelocitX = -ballVelocitX;
    }
    if (ballY < bkhmin)
    {
        ballVelocityY = -ballVelocityY;
    }
    if (ballY > bkh + 60)
    {
        game_over(renderer);
        return false;
    }

    if (SDL_HasIntersection(&ball,&bat))

    {
        Mix_PlayChannel( -1, bounce, 0 );
        // Calculate the distance between the center of the bat and the center of the ball
        float batCenterX = batX + 30;
        float ballCenterX = ballX + ballwidth / 2;
        float distance = ballCenterX - batCenterX;

        // Calculate the angle modifier based on the distance
        float angleModifier = distance / 30;

        // Calculate the new angle of reflection
        float angle = atan2(ballVelocityY, ballVelocitX) + angleModifier;

        // Calculate the new velocity components
        float speed = sqrt(ballVelocitX * ballVelocitX + ballVelocityY * ballVelocityY);
        ballVelocitX = -speed * cos(angle);
        ballVelocityY = -speed * sin(angle);
    }
    return true;
}

std::pair<std::shared_ptr<SDL_Window>, std::shared_ptr<SDL_Renderer>> create_context()
{
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
}

std::shared_ptr<SDL_Texture> load_texture(shared_ptr<SDL_Renderer> renderer, const std::string name)
{
    auto surface = SDL_LoadBMP(("assets/" + name).c_str());
    if (!surface)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0x0ff, 0x0, 0x0ff));

    auto texture = SDL_CreateTextureFromSurface(renderer.get(), surface);
    if (!texture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        throw std::invalid_argument(SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return {texture, [](auto *p) { SDL_DestroyTexture(p); }};
}

void renderBricks(SDL_Renderer *renderer, SDL_Rect brick[][7])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            SDL_RenderFillRect(renderer, &brick[i][j]);
        }
    }
}

void win(SDL_Renderer *renderer)
{
    SDL_Surface *win = SDL_LoadBMP("./assets/cup.bmp");
    SDL_RenderClear(renderer);

    SDL_Texture *wintexture = SDL_CreateTextureFromSurface(renderer, win);
    SDL_Rect winrect = {300, 200, 200, 200};
    SDL_RenderCopy(renderer, wintexture, nullptr, &winrect);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
}

void quit(){
    SDL_Event event;
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_ESCAPE;
    event.key.keysym.mod = 0;
    event.key.repeat = 0;

    SDL_PushEvent(&event);
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    {
        auto [window_p, renderer_p] = create_context();
        SDL_SetRenderDrawColor(renderer_p.get(), 255, 100, 50, 255);
        InitializeBrick();

        //Sound init

        {bool success = true;
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
                printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
                success = false;
            }
            bounce = Mix_LoadWAV("./assets/tyry.mp3");
            if (bounce == NULL) {
                printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
                success = false;
            }
        }
        auto background = load_texture(renderer_p, "bg_space_seamless.bmp");
        auto prev_tick = SDL_GetTicks();
        int frame_dropped = 0;
        int running = true;
        while (handle_events())
        {
            if (!frame_dropped)
            {

                SDL_Rect ballrect = {ballX, ballY, ballwidth, ballheight};
                SDL_Rect batrect = {batX, batY, 60, 10};
                running  = ball_wall_collision(renderer_p.get(),ballrect,batrect);
                ball_brick_collision(ballrect);

                moveBall();

                if (::abs(ballVelocityY) < 1){
                    ballVelocityY = 3;
                }
                if (deleted_bricks >= number_of_bricks)
                {
                    SDL_SetRenderDrawColor(renderer_p.get(), 0, 0, 0, 255);
                    win(renderer_p.get());
                    break;
                }

                SDL_RenderCopy(renderer_p.get(), background.get(), nullptr, nullptr);
                SDL_RenderFillRect(renderer_p.get(), &ballrect);
                SDL_RenderFillRect(renderer_p.get(), &batrect);
                for (int i = 0; i < 3; i++)
                {
                    for (int j = 0; j < 7; j++)
                    {
                        SDL_RenderFillRect(renderer_p.get(), &brickrect[i][j]);
                    }
                }
                SDL_RenderPresent(renderer_p.get());
                SDL_RenderClear(renderer_p.get());
                float ticks = SDL_GetTicks();
                if ((ticks - prev_tick) < 33)
                {
                    SDL_Delay(33 - (ticks - prev_tick));
                    frame_dropped = 0;
                }
                else
                {
                    frame_dropped = 1;
                }
                prev_tick += 33;


            }
            if (!running){
                break;
            }

        }
        closeSound();
        SDL_Quit();

        return 0;
    }
}
