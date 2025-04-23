# 052211-task02
built using Vienna Vulkan Engine
only contains files that are modified from the original Vienna Vulkan Engine: https://github.com/hlavacs/ViennaVulkanEngine
to run, clone the Vienna Vulkan Engine and replace the 3 folders:
* ViennaVulkanEngine\assets
* ViennaVulkanEngine\examples
* ViennaVulkanEngine\src

run the following command in ViennaVulkanEngine:
build_MVC.cmd\n
cd build\n
cmake --build . --config Debug\n
cd ..\n
build\examples\game\Debug\game.exe

the game should launch and look like this:
![image](https://github.com/user-attachments/assets/1c4e1b0c-3b71-44f5-b4d0-23bcdf9d4afe)
![image](https://github.com/user-attachments/assets/142c27f5-9388-4616-8b17-23744273ed81)
![image](https://github.com/user-attachments/assets/c4ec698f-d742-45ca-ad24-366544677566)

### controls
`w`, `a`, `s`, `d` - player movements
`left`, `right` - rotates player to the left / right
`spacebar` - places a cube in front of the player if any cube has been collected

### gameplay
* player can collect up to 10 cubes
* player can also place the cubes anywhere they want
