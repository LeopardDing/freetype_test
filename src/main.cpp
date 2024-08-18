#include "fontmanager.h"
#include <cairo.h>
#include <codecvt>
#include <cstdio>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <iostream>
#include <locale>
#include <string>
#include <thread>

struct Color {
  float red;
  float green;
  float blue;
  float alpha;
};

void draw_pixel(cairo_t *context, int x, int y, Color color) {
  cairo_set_source_rgba(context, color.red, color.green, color.blue,
                        color.alpha);
  cairo_rectangle(context, x, y, 1, 1);
  cairo_fill(context);
}

std::u32string utf8_to_unicode(const std::string &utf8_str) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert_utf32;
  return convert_utf32.from_bytes(utf8_str);
}

int draw_font(cairo_t *context, FT_Face face, int x, int y, FT_Glyph aglyph) {
  FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(aglyph);
  FT_Bitmap bitmap = bitmap_glyph->bitmap;
  int left = face->glyph->bitmap_left;
  int top = face->glyph->bitmap_top;

  for (int j = 0; j < bitmap.rows; j++, y++) {
    int x_tmp = x;
    for (int i = 0; i < bitmap.width; i++, x_tmp++) {
      float alpha = bitmap.buffer[j * bitmap.width + i] / 255.0;
      draw_pixel(context, x_tmp + left, y - top, Color{1.0, 1.0, 1.0, alpha});
    }
  }

  return (bitmap_glyph->root.advance.x >> 16) - face->glyph->bitmap_left;
}

int main() {
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 400);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    std::cerr << "Error: cannot create cairo surface" << std::endl;
    return 1;
  }

  // 创建一个cairo上下文
  cairo_t *context = cairo_create(surface);

  // 显示文字
  int x = 0;
  auto unicode_str = utf8_to_unicode("你好 Hello backup");
  for (const auto &unicode : unicode_str) {
    FT_Glyph aglyph = nullptr;
    FTC_Node anode = nullptr;
    FT_Face face;
    int font_size = 18;
    fontmanager::instance()->start_image(unicode, font_size, face, aglyph,
                                         anode);

    FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(aglyph);
    FT_Bitmap bitmap = bitmap_glyph->bitmap;

    int y = face->height >> 6;
    x += draw_font(context, face, x, y, aglyph);

    printf("=========================================================\n");
    printf("unicode = 0x%04X, advance=(%ld,%ld), x_offset = %ld\n", unicode,
           bitmap_glyph->root.advance.x, bitmap_glyph->root.advance.y,
           bitmap_glyph->root.advance.x >> 16);
    printf("units_per_EM=%d, ascender=%d, descender=%d\n", face->units_per_EM,
           face->ascender, face->descender);
    printf("bitmap_left/top=(%d,%d)\n", face->glyph->bitmap_left,
           face->glyph->bitmap_top);
    printf(
        "width=%ld height=%ld\nhoriBearingX=%ld horiBearingY=%ld "
        "horiAdvance=%ld\n vertBearingX=%ld vertBearingY=%ld vertAdvance=%ld\n",
        face->glyph->metrics.width, face->glyph->metrics.height,
        face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY,
        face->glyph->metrics.horiAdvance, face->glyph->metrics.vertBearingX,
        face->glyph->metrics.vertBearingY, face->glyph->metrics.vertAdvance);
    printf("face h=%d\n", face->height >> 6);

    fontmanager::instance()->end_image(anode);
  }

  // 画点
  // draw_pixel(context, 0, 0, Color{1.0, 1.0, 1.0, 1.0});
  // draw_pixel(context, 10, 0, Color{1.0, 1.0, 1.0, 1.0});

  // 提交所有绘制操作
  cairo_surface_write_to_png(surface, "output.png");

  // 清理资源
  cairo_destroy(context);
  cairo_surface_destroy(surface);

  return 0;
}
