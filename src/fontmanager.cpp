#include "fontmanager.h"
#include <assert.h>
#include <chrono>
#include <cstdio>
#include <freetype/ftcache.h>
#include <freetype/fttypes.h>
#include <thread>

void print_bitmap(FTC_SBit sbit) {
  for (int y = 0; y < sbit->height; y++) {
    for (int x = 0; x < sbit->width; x++) {
      int byte_index =
          (y * sbit->pitch) + (x / 8); // 计算当前像素所在的字节位置
      int bit_index = x % 8; // 计算当前像素在字节中的位置
      unsigned char bit =
          sbit->buffer[byte_index] & (0x80 >> bit_index); // 取出对应的位
      if (bit) {
        printf("#"); // 如果该位为1，打印'#'
      } else {
        printf(" "); // 如果该位为0，打印空格
      }
    }
    printf("\n"); // 换行符，移动到下一行
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

fontmanager::fontmanager() {
  FT_Error error = FT_Init_FreeType(&m_library);
  assert(error == FT_Err_Ok);

  error =
      FTC_Manager_New(m_library, 0, 0, 0, &face_requester, NULL, &m_manager);
  assert(error == FT_Err_Ok);

  error = FTC_CMapCache_New(m_manager, &m_cmapcache);
  assert(error == FT_Err_Ok);

  error = FTC_SBitCache_New(m_manager, &m_sbitcache);
  assert(error == FT_Err_Ok);
}

fontmanager::~fontmanager() {
  FTC_Manager_Done(m_manager);
  FT_Done_FreeType(m_library);
}

bool fontmanager::get_font_buffer(uint32_t unicode, int font_size) {
  FT_Face face;
  FTC_FaceID face_id = &s_faces[0];
  FT_Error error = FT_Err_Ok;
  FT_UInt glyph_index = 0;

std::unique_lock<std::shared_mutex> lock(m_rwmutex);

  {
    // std::shared_lock<std::shared_mutex> lock(m_rwmutex);
    error = FTC_Manager_LookupFace(m_manager, face_id, &face);
    if (error != FT_Err_Ok) {
      printf("FTC_Manager_LookupFace fail\n");
    }

    FT_UInt charmap_index = FT_Get_Charmap_Index(face->charmap);
    glyph_index =
        FTC_CMapCache_Lookup(m_cmapcache, face_id, charmap_index, unicode);
  }

  FTC_ImageTypeRec type;
  type.face_id = &s_faces[0];
  type.flags = FT_LOAD_RENDER | FT_LOAD_TARGET_MONO;
  type.width = font_size;
  type.height = font_size;

  FTC_SBit sbit = NULL;
  FTC_Node anode = NULL;
  {
    // std::unique_lock<std::shared_mutex> lock(m_rwmutex);
    error =
        FTC_SBitCache_Lookup(m_sbitcache, &type, glyph_index, &sbit, &anode);
    if (error != FT_Err_Ok) {
      printf("FTC_SBitCache_Lookup fail\n");
    }
  }

  // print_bitmap(sbit);
  // printf("FTC_SBitCache_Lookup success. sbit=%p, anode=%p\n", sbit, anode);
  

  if (anode) {
    // std::unique_lock<std::shared_mutex> lock(m_rwmutex);
    FTC_Node_Unref(anode, m_manager);
  }

  return true;
}