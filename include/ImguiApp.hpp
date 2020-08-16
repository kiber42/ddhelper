#pragma once

#include <string>
#include <memory>

class ImguiApp
{
public:
  ImguiApp(const std::string& windowTitle);
  ImguiApp(const ImguiApp&) = delete;
  ImguiApp& operator=(const ImguiApp&) = delete;
  ~ImguiApp();

  void run();

private:
  struct Data;
  std::unique_ptr<Data> data;
};
