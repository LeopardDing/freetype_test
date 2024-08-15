#include "fontmanager.h"
#include <cairo.h>
#include <iostream>
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

int main() {
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 50);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    std::cerr << "Error: cannot create cairo surface" << std::endl;
    return 1;
  }

  // 创建一个cairo上下文
  cairo_t *context = cairo_create(surface);

  // 画点
  draw_pixel(context, 0, 0, Color{1.0, 1.0, 1.0, 1.0});
  draw_pixel(context, 10, 0, Color{1.0, 1.0, 1.0, 1.0});

  // 提交所有绘制操作
  cairo_surface_write_to_png(surface, "output.png");

  // 清理资源
  cairo_destroy(context);
  cairo_surface_destroy(surface);

  return 0;
}
