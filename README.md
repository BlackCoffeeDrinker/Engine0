# Engine00
Basic engine for doing ALTTP types of games for DOS & SDL

Very much WIP

## Building the project
### Prerequisites
You will need:

* A modern C/C++ compiler
* CMake 3.1+ installed (on a Mac, run `brew install cmake`)
* If you prefer to code in a great IDE, I highly recommend [Jetbrains CLion](https://www.jetbrains.com/clion/). It is fully compatible with this project.

### Building The Project
#### Git Clone
First we need to check out the git repo:

```bash
❯ mkdir ~/workspace
❯ cd ~/workspace
❯ git clone \
    https://github.com/BlackCoffeeDrinker/Engine0.git \
    my-project
❯ cd my-project
❯ git submodule init && git submodule update
```

#### Run cmake to build the makefile
```bash
❯ cmake -B build-dir
❯ cd build-dir
❯ make
```

### Building in CLion
> **NOTE**: Since JetBrains software [does not officially support git submodules](https://youtrack.jetbrains.com/issue/IDEA-64024), you must run `git submodule init && git submodule update` before starting CLion on a freshly checked-out repo.

Assuming you've done the above two steps, you can start CLion, and open the project's top level folder. CLion should automatically detect the top level `CMakeLists.txt` file and provide you with the full set of build targets.

Select menu option **Build   ➜ Build Project**.

## Making your own game
> TODO
