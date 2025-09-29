//-----------------------------------------------------------------------
                            Nest Engine
+------+.      +------+       +------+       +------+      .+------+
|`.    | `.    |\     |\      |      |      /|     /|    .' |    .'|
|  `+--+---+   | +----+-+     +------+     +-+----+ |   +---+--+'  |
|   |  |   |   | |    | |     |      |     | |    | |   |   |  |   |
+---+--+.  |   +-+----+ |     +------+     | +----+-+   |  .+--+---+
 `. |    `.|    \|     \|     |      |     |/     |/    |.'    | .'
   `+------+     +------+     +------+     +------+     +------+'

   Cloning Instructions:
   I removed some of the git-ignores for my added libraries to help 
   prevent the need of running cmake as of now, until I get around
   to just writing a tool to do so. Cloning is as simple as clicking
   the clone button on GitHub. I prefer to click the *send to GitHubDesktop*
   button and using the app to make a local repository, but there are no
   added steps.

   Compiling:
   When you first open the project, it defaults to the engine itself as the start up project.
   The engine is entirely made as a back-end program and cannot be the primary start up,
   so you have to switch the start-up project to "The Tower" which is a placeholder
   name for the game project. 
   The engine does take a little while to build (~1 minute) and due to a new bug
   after the implementation of PBR materials, it will not run in release mode. 
   You must have the vulkan-sdk installed in order to run the game. You shouldn't need
   to include SDL with the installation due to it already being in my lib folder
   but I haven't tested it to know. 

   Navigation:
   All of the important code is contained in the NestEngine project. I did rush
   this last update due to some time constraints, so there are a handful of
   bad coding practices you might see snooping around but they are only temporary
   until I get some free time to optimize and clean up the new PBR stuff. 
   The Engine folder contains the main Engine class that allocates, initiates, and 
   holds all of the "Managers" that make up the flow of the engine. Managers in the 
   context of Nest Engine are primarily classes that control a certain area indicated
   by their names. SkyboxManager for example, holds the skybox mesh and the images that go
   with it to draw the skybox you see in game. 
   The Managers are a good place to start if you want to see the gears that make up
   the engine itself as they all have important roles they perform from the very
   start of the program till the end. 
   The GlobalConstants folder has many lazily set variables I used to initiate differnet things
   such as relevant asset paths, camera settings, and Hashed Ids. These are of course, temporary,
   and were added purely as a time saver.
   VulkanWrappers and SDLWrappers contain a bunch of third party stuff I didn't want to include
   in header files. I made an exception to that rule for glm
   because it makes less sense to write an entire math library
   than to just allow the extra build time.
   Finally, all of my assets, shaders, and XMLFiles are all located in the game folder (TheTower) where
   the .sln is located at;
   
   Other Necessary Information:
   Currently my lighting is a bit bugged, it's something thats always been a little bit wonky
   but has yet to end up as a priority to fix. I think its mostly due to the lighting
   we implemented last semester that forces the light to look at origin {0, 0, 0}
   which causes some weirdness with moving the light as you'll see.
