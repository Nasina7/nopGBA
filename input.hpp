#include "include.hpp"
int handleControlsr()
{
    switch( SDL_EVENT_HANDLING.key.keysym.sym )
        {

        case SDLK_z:
            controlBuffer[0] = 1;
        break;

        case SDLK_x:
            controlBuffer[1] = 1;
        break;

        case SDLK_SPACE:
            controlBuffer[2] = 1;
        break;

        case SDLK_RETURN:
            controlBuffer[3] = 1;
        break;

        case SDLK_UP:
            controlBuffer[6] = 1;
        break;

        case SDLK_DOWN:
            controlBuffer[7] = 1;
        break;

        case SDLK_LEFT:
            controlBuffer[5] = 1;
        break;

        case SDLK_RIGHT:
            controlBuffer[4] = 1;
        break;

        case SDLK_a:
            controlBuffer[9] = 1;
        break;

        case SDLK_s:
            controlBuffer[8] = 1;
        break;

        }
    return 0;
}
int handleControls()
{
    switch( SDL_EVENT_HANDLING.key.keysym.sym )
        {

        case SDLK_z:
            controlBuffer[0] = 0;
        break;

        case SDLK_x:
            controlBuffer[1] = 0;
        break;

        case SDLK_SPACE:
            controlBuffer[2] = 0;
        break;

        case SDLK_RETURN:
            controlBuffer[3] = 0;
        break;

        case SDLK_UP:
            controlBuffer[6] = 0;
        break;

        case SDLK_DOWN:
            controlBuffer[7] = 0;
        break;

        case SDLK_LEFT:
            controlBuffer[5] = 0;
        break;

        case SDLK_RIGHT:
            controlBuffer[4] = 0;
        break;

        case SDLK_a:
            controlBuffer[9] = 0;
        break;

        case SDLK_s:
            controlBuffer[8] = 0;
        break;

        }
    return 0;
}
int handleSDLcontrol()
{
    while( SDL_PollEvent( &SDL_EVENT_HANDLING)) // While Event to handle Random Stuff
        {
            if (SDL_EVENT_HANDLING.type == SDL_QUIT) // If the SDL Window is Closed, close the program.
            {
                opcodeError = true;
            }
            if (SDL_EVENT_HANDLING.type == SDL_KEYDOWN) // If a key is being pressed, handle controls.
            {   // Handle Controls
                handleControls();
            }
            if (SDL_EVENT_HANDLING.type == SDL_KEYUP)
            {
                handleControlsr();
            }
        }
    return 0;
}
