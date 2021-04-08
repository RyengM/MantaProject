# MantaflowProject

## Compile and Install

### Preparation
- Install **Visual Studio**

### My Configuration
- Visual Studio 2019

### Compilation
- git clone https://github.com/demonhub/FluidSolver.git
- cd FluidSolver
- mkdir Build
- cd Build
- cmake .. -G “Visual Studio 16 2019” (for example)
- open MantaflowProject.sln
- build solution
- set Renderer as startup project

### Use TBB
- download tbb from https://github.com/jckarter/tbb
- put file into C:/Program Files/Intel/
- rename folder as TBB
- cd Build & cmake .. -G "Visual Studio 16 2019" -DTBB="ON"
- manta -> VC++ Dictionaries -> Library Directories -> C:\Program Files\Intel\TBB\lib\intel64\vc14

### Texture
- download from git repository d3d12book
- put textures into Renderer/Texture