// Include OpenCV before any X11 headers
#include <opencv2/opencv.hpp>

#include "bridge/capture.hpp"

#include <functional>
#include <map>
#include <string_view>
#include <vector>

auto getTile(const cv::Mat& dungeon, int x, int y)
{
  if (y > 0)
    return dungeon(cv::Rect(x * 30, y * 30 - 1, 30, 30));
  auto partialTile = dungeon(cv::Rect(x * 30, 0, 30, 29));
  cv::Mat tile = cv::Mat::zeros(cv::Size(30, 30), CV_8UC4);
  partialTile.rowRange(0, 29).copyTo(tile.rowRange(1, 30));
  return tile;
}

auto getTileCropped(const cv::Mat& dungeon, int x, int y)
{
  return dungeon(cv::Rect(x * 30 + 1, y * 30, 22, 25));
}

std::size_t getTileHash(const cv::Mat& tile)
{
  const auto hash = std::hash<std::string_view>();
  const auto size = static_cast<int>(tile.channels() * tile.total());
  const cv::Mat flattened = tile.isContinuous() ? tile.reshape(1, size) : tile.clone().reshape(1, size);
  auto start = reinterpret_cast<const char*>(flattened.data);
  const std::string_view data(start, size);
  return hash(data);
}

const std::map<std::size_t, std::string> tileName = {
    {12171726562143104985ull, "     "}, // unrevealed tile
    { 9173380990794071178ull, "ur ms"}, // unrevealed monster
    {15573335119931461304ull, "attck"}, // attack booster
    {  791283006467486013ull, "_____"}, // empty tile
    { 9141210660063553429ull, "_____"}, // empty tile
    { 2331902878712028713ull, "_____"}, // empty tile
    {16740581902444378552ull, "_____"}, // empty tile
    {15895078020171200905ull, "_____"}, // empty tile
    { 9835723397204991232ull, "_____"}, // empty tile
    {11362091924657848982ull, "_____"}, // empty tile
    {12347129595014082498ull, "_____"}, // empty tile
    {17767323230370704233ull, "_____"}, // empty tile
    {16997596562712833847ull, "HERO1"}, // Human Fighter (male) level 1
    { 8714229552769317059ull, "HERO1"}, // Human Fighter (male) level 1
    {12051280772885469698ull, "HERO1"}, // Human Fighter (male) level 1
    { 3839866967667015288ull, "HERO1"}, // Human Fighter (male) level 1
    {11052318805234942455ull, "HERO1"}, // Human Fighter (male) level 1
    { 3018641671536505710ull, "HERO1"}, // Human Fighter (male) level 1
    {14636700972353946929ull, "HERO1"}, // Human Fighter (male) level 1
    {16050501458243163274ull, "HERO1"}, // Human Fighter (male) level 1
    { 2997121012634161362ull, "HERO1"}, // Human Fighter (male) level 1
    { 2127649111672635980ull, "H shp"}, // Human Fighter (male) level 1 on regular shop
    {14277029564032197858ull, "mana+"}, // mana booster
    {17630348201889708526ull, "laddr"}, // ladder
    {12737238288011505541ull, "wall "}, // wall, NW corner
    { 3155065951784691632ull, "wall "}, // wall, N edge
    {17991808385274740339ull, "wall "}, // wall, N edge
    {11634847330714352923ull, "wall "}, // wall, NE corner
    { 2297961571120147521ull, "wall "}, // wall E edge
    { 2289393347464944253ull, "wall "}, // wall E edge
    {11643363257577987010ull, "wall "}, // wall E edge
    {17910414140143923424ull, "wall "}, // wall E edge
    {16062608555017525063ull, "wall "}, // wall W edge
    { 7654736843175325693ull, "wall "}, // wall W edge
    {15358100886186825766ull, "wall "}, // wall W edge
    {13515795412378215403ull, "wall "}, // wall W edge
    {18083812515344936243ull, "wall "}, // wall W edge
    {12155507682267015243ull, "wall "}, // wall S edge
    {12586797759138074376ull, "wall "}, // wall SE corner
    {14584233861971122521ull, "wall "}, // wall SE corner
    { 4394708166481901527ull, "wall "}, // wall SW corner
    { 9995732377330130570ull, "wall "}, // wall NE corner
    { 7508802750427637019ull, "gold "}, // gold pile
    { 1049221031221769068ull, "shop "}, // regular shop
    {14746801269410754160ull, "h pot"}, // health potion
    { 7611154429272924676ull, "wall "}, // wall inner corner SW
    { 2031433610599097615ull, "wall "}, // wall inner corner NE
    { 1188637556663082676ull, "wall "}, // wall inner corner NE
    { 8756565251768472314ull, "wall "}, // wall single end S
    { 8724638554463298136ull, "wall "}, // wall single end N
    { 5566645017322722726ull, "wall "}, // wall single end W
    {15480795221730993915ull, "wall "}, // wall single corner connecting S and W

    {16073343456878459014ull, "DrSp1"}, // Dragon Spawn level 1
    { 2860958847580806170ull, "Srpt1"}, // Serpent level 1
    { 8293133935876979174ull, "Wlck1"}, // Warlock level 1
    { 7991578473569262129ull, "Wlck2"}, // Warlock level 2
    { 3082185347476175328ull, "Golm3"}, // Golem level 3

    {  635487067432940066ull, "blood"}, // Blood pool
};

int main()
{
  int numFrames = 0;
  ImageCapture capture;
  while (auto ximage = capture.acquire())
  {
    if (ximage->width != 800 || ximage->height != 600)
    {
      printf("Please resize game window to 800x600 (current size: %ix%i)\n", ximage->width, ximage->height);
      cv::waitKey(1000);
      continue;
    }
    printf("Processing image...");
    auto image = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
    for (int index = 0; index < 400; ++index)
    {
      if (index % 20 == 0)
        printf("\n");
      auto tile = getTileCropped(image, index % 20, index / 20);
      auto hash = getTileHash(tile);
      if (auto name = tileName.find(hash); name != end(tileName))
      {
        printf("%s, ", name->second.data());
      }
      else
      {
        printf("%lu", hash);
        auto tile = getTile(image, index % 20, index / 20);
        cv::imshow("dungeon", tile);
        break;
      }
    }
    puts("");
    cv::waitKey(1000);
    ++numFrames;
  }

  return numFrames != 0;
}
