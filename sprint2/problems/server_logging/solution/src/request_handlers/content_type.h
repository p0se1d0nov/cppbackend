#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <string>
#include <string_view>
#include <unordered_map>

namespace http_handler {

using namespace std::literals;

struct ContentType {
  ContentType() = delete;
  constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
  constexpr static std::string_view TEXT_HTML = "text/html"sv;
  constexpr static std::string_view TEXT_JSON = "application/json"sv;
  constexpr static std::string_view TEXT_CSS = "text/css"sv;
  constexpr static std::string_view TEXT_JS = "text/javascript"sv;
  constexpr static std::string_view TEXT_XML = "application/xml"sv;
  constexpr static std::string_view IMAGE_PNG = "image/png"sv;
  constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
  constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
  constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
  constexpr static std::string_view IMAGE_MS = "image/vnd.microsoft"sv;
  constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
  constexpr static std::string_view IMAGE_SVG = "image/svg+xml"sv;
  constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;

  inline static const std::unordered_map<std::string, std::string_view> Types{
      // Текстовые форматы
      {".htm"s, ContentType::TEXT_HTML},
      {".html"s, ContentType::TEXT_HTML},
      {".css"s, ContentType::TEXT_CSS},
      {".txt"s, ContentType::TEXT_PLAIN},
      {".js"s, ContentType::TEXT_JS},
      {".json"s, ContentType::TEXT_JSON},
      {".xml"s, ContentType::TEXT_XML},

      // Изображения
      {".png"s, ContentType::IMAGE_PNG},
      {".jpg"s, ContentType::IMAGE_JPEG},
      {".jpe"s, ContentType::IMAGE_JPEG},
      {".jpeg"s, ContentType::IMAGE_JPEG},
      {".gif"s, ContentType::IMAGE_GIF},
      {".bmp"s, ContentType::IMAGE_BMP},
      {".ico"s, ContentType::IMAGE_MS},
      {".tiff"s, ContentType::IMAGE_TIFF},
      {".tif"s, ContentType::IMAGE_TIFF},
      {".svg"s, ContentType::IMAGE_SVG},
      {".svgz"s, ContentType::IMAGE_SVG},

      // Аудио
      {".mp3"s, ContentType::AUDIO_MPEG}};
};
} // namespace http_handler