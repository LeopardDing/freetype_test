#include "fontmanager.h"
#include <thread>

int main() {
//   for (int i = 0; i < 40000; i++)
//   {
//       fontmanager::instance()->get_font_buffer(0x4E2D, 16);
//       fontmanager::instance()->get_font_buffer(0x4E2D, 16);
//   }

  std::thread thread1([]() {
    for (int i = 0; i < 10000; i++) {
      fontmanager::instance()->get_font_buffer(0x4E2D, 16);
      fontmanager::instance()->get_font_buffer(0x4E2D, 16);
    }
  });

  std::thread thread2([]() {
    for (int i = 0; i < 10000; i++) {
      fontmanager::instance()->get_font_buffer(0x4E2D, 20);
      fontmanager::instance()->get_font_buffer(0x4E2D, 20);
    }
  });

  std::thread thread3([]() {
    for (int i = 0; i < 10000; i++) {
      fontmanager::instance()->get_font_buffer(0x4E2D, 32);
      fontmanager::instance()->get_font_buffer(0x4E2D, 32);
    }
  });

  std::thread thread4([]() {
    for (int i = 0; i < 10000; i++) {
      fontmanager::instance()->get_font_buffer(0x4E2D, 28);
      fontmanager::instance()->get_font_buffer(0x4E2D, 28);
    }
  });

  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
}
