#include "fontmanager.h"
#include <assert.h>
#include <chrono>
#include <cstdio>
#include <freetype/ftcache.h>
#include <freetype/fttypes.h>
#include <thread>

void print_bitmap(FTC_SBit sbit) {
  printf("================ %dx%d ===============\n", sbit->width, sbit->height);
  for (int y = 0; y < sbit->height; y++) {
    for (int x = 0; x < sbit->width; x++) {
      int byte_index =
          (y * sbit->pitch) + (x / 8); // 计算当前像素所在的字节位置
      int bit_index = x % 8; // 计算当前像素在字节中的位置
      unsigned char bit =
          sbit->buffer[byte_index] & (0x80 >> bit_index); // 取出对应的位
      if (bit) {
        putchar('#'); // 如果该位为1，打印'#'
      } else {
        putchar(' '); // 如果该位为0，打印空格
      }
    }
    putchar('\n'); // 换行符，移动到下一行
  }
}

const char *gray_levels = " .:=agd#%@";
void print_bitmap_glyph(FT_BitmapGlyph bitmap_glyph) {
  FT_Bitmap bitmap = bitmap_glyph->bitmap;
  printf("================ %dx%d ===============\n", bitmap.width, bitmap.rows);

  int num_levels = strlen(gray_levels);

  for (int y = 0; y < bitmap.rows; y++) {
    for (int x = 0; x < bitmap.width; x++) {
      unsigned char pixel = bitmap.buffer[y * bitmap.pitch + x];

      // 根据位图深度进行不同处理
      if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        // 灰度位图
        char c = gray_levels[(pixel * (num_levels - 1)) / 255];
        printf("%c", c);
      } else if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        // 单色位图
        int byte_index = x / 8;
        int bit_index = 7 - (x % 8);
        unsigned char bit =
            bitmap.buffer[y * bitmap.pitch + byte_index] & (1 << bit_index);
        printf("%c", bit ? '#' : ' ');
      } else {
        // 其他格式可以根据需要进行处理
        printf("?");
      }
    }
    printf("\n");
  }
}

fontmanager::MyFaceRec fontmanager::s_faces[] = {
    {
        "simhei.ttf",
        0,
    },
};

FT_Error fontmanager::face_requester(FTC_FaceID face_id, FT_Library library,
                                     FT_Pointer request_data, FT_Face *aface) {
  MyFaceRec *face = static_cast<MyFaceRec *>(face_id);
  return FT_New_Face(library, face->file_path, face->face_index, aface);
}

fontmanager *fontmanager::instance() {
  static fontmanager instance;
  return &instance;
}

fontmanager::fontmanager()
    : m_library(NULL), m_manager(NULL), m_sbitcache(NULL), m_imagecache(NULL),
      m_cmapcache(NULL) {
  FT_Error error = FT_Init_FreeType(&m_library);
  assert(error == FT_Err_Ok);

  error =
      FTC_Manager_New(m_library, 0, 0, 0, &face_requester, NULL, &m_manager);
  assert(error == FT_Err_Ok);

  error = FTC_CMapCache_New(m_manager, &m_cmapcache);
  assert(error == FT_Err_Ok);

  error = FTC_SBitCache_New(m_manager, &m_sbitcache);
  assert(error == FT_Err_Ok);

  error = FTC_ImageCache_New(m_manager, &m_imagecache);
  assert(error == FT_Err_Ok);
}

fontmanager::~fontmanager() {
  FTC_Manager_Done(m_manager);
  FT_Done_FreeType(m_library);
}

bool fontmanager::get_sbit_buffer(uint32_t unicode, int font_size) {
  FT_Face face;
  FTC_FaceID face_id = &s_faces[0];
  FT_Error error = FT_Err_Ok;
  FT_UInt glyph_index = 0;

  std::unique_lock<std::shared_mutex> lock(m_rwmutex);

  error = FTC_Manager_LookupFace(m_manager, face_id, &face);
  if (error != FT_Err_Ok) {
    printf("FTC_Manager_LookupFace fail\n");
  }

  FT_Int charmap_index = FT_Get_Charmap_Index(face->charmap);
  glyph_index =
      FTC_CMapCache_Lookup(m_cmapcache, face_id, charmap_index, unicode);

  FTC_ImageTypeRec type;
  type.face_id = &s_faces[0];
  type.flags = FT_LOAD_RENDER | FT_LOAD_TARGET_MONO;
  type.width = font_size;
  type.height = font_size;

  FTC_SBit sbit = NULL;
  FTC_Node anode = NULL;

  error = FTC_SBitCache_Lookup(m_sbitcache, &type, glyph_index, &sbit, &anode);
  if (error != FT_Err_Ok) {
    printf("FTC_SBitCache_Lookup fail\n");
  }

  print_bitmap(sbit);
  // printf("FTC_SBitCache_Lookup success. sbit=%p, anode=%p\n", sbit, anode);

  if (anode) {
    FTC_Node_Unref(anode, m_manager);
  }

  return true;
}

bool fontmanager::get_image_buffer(uint32_t unicode, int font_size) {
  FT_Face face;
  FTC_FaceID face_id = &s_faces[0];
  FT_Error error = FT_Err_Ok;
  FT_UInt glyph_index = 0;

  std::unique_lock<std::shared_mutex> lock(m_rwmutex);

  error = FTC_Manager_LookupFace(m_manager, face_id, &face);
  if (error != FT_Err_Ok) {
    printf("FTC_Manager_LookupFace fail\n");
  }

  FT_Int charmap_index = FT_Get_Charmap_Index(face->charmap);
  glyph_index =
      FTC_CMapCache_Lookup(m_cmapcache, face_id, charmap_index, unicode);

  FTC_ImageTypeRec type;
  type.face_id = &s_faces[0];
  type.flags = FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL;
  type.width = font_size;
  type.height = font_size;

  FT_Glyph aglyph = NULL;
  FTC_Node anode = NULL;

  error =
      FTC_ImageCache_Lookup(m_imagecache, &type, glyph_index, &aglyph, &anode);
  if (error != FT_Err_Ok) {
    printf("FTC_ImageCache_Lookup fail\n");
  }

  print_bitmap_glyph((FT_BitmapGlyph)aglyph);

  if (anode) {
    FTC_Node_Unref(anode, m_manager);
  }
  return true;
}