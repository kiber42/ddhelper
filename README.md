Prerequisites
-------------
sudo apt install clang
sudo apt install cmake
sudo apt install libsdl2-dev

git clone git@github.com:ocornut/imgui.git
cp imgui/examples/imgui_impl_sdl.* imgui
cp imgui/examples/imgui_impl_opengl3.* imgui

git clone git@github.com:banditcpp/bandit.git
git clone git@github.com:banditcpp/snowhouse.git
cp -r snowhouse/include/snowhouse/* bandit/bandit/assertion_frameworks/snowhouse
rm -rf snowhouse

For Vulkan build:
sudo apt install libvulkan-dev
cp imgui/examples/imgui_impl_vulkan.* imgui


Visual Studio Code
+ C/C++ Extension
+ CMake Tools Extension
+ Clang-Format Extension

Ctrl+Shift+P CMake: Configure
Ctrl+Shift+P CMake: Build
