// Termm--Winter 2021

#include <iostream>
#include "scene_lua.hpp"
#include "A5.hpp"
#include "cs488-framework/CS488Window.hpp"

int main(int argc, char** argv)
{
  std::string filename = "simple.lua";
  if (argc >= 2) {
    filename = argv[1];
  }


  if (!run_lua(filename)) {
    std::cerr << "Could not open " << filename <<
                 ". Try running the executable from inside of" <<
                 " the Assets/ directory" << std::endl;
    return 1;
  }

  CS488Window::launch( argc, argv, A5::window, 1024, 1024, "W21 Assignment 5" );
}
