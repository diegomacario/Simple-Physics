#ifndef GAME_H
#define GAME_H

#include "Window.h"
#include "State.h"
#include "FiniteStateMachine.h"

class Game
{
public:

   Game();
   ~Game();

   Game(const Game&) = delete;
   Game& operator=(const Game&) = delete;

   Game(Game&&) = delete;
   Game& operator=(Game&&) = delete;

   bool  initialize(const std::string& title);
   void  executeGameLoop();

#ifdef __EMSCRIPTEN__
   void  updateWindowDimensions(int width, int height);
#endif

private:

   std::shared_ptr<FiniteStateMachine> mFSM;

   std::shared_ptr<Window>             mWindow;
};

#endif
