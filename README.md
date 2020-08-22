Prerequisites
-------------
sudo apt install clang
sudo apt install cmake
sudo apt install libsdl2-dev
sudo apt install libvulkan-dev

git clone git@github.com:banditcpp/bandit.git
git clone git@github.com:ocornut/imgui.git
cp imgui/examples/imgui_impl_sdl.* imgui
cp imgui/examples/imgui_impl_vulkan.* imgui

Visual Studio Code
+ CMake Tools Extension
+ Clang-Format Extension

Ctrl+Shift+P CMake: Configure
Ctrl+Shift+P CMake: Build
