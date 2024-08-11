#pragma once
#include <ft2build.h>
#include <shared_mutex>
#include <string>
#include <vector>
#include FT_FREETYPE_H
#include FT_CACHE_H

class fontmanager {
  typedef struct MyFaceRec_ {
    const char *file_path;
    int face_index;
  } MyFaceRec;

public:
  static fontmanager *instance();

  bool get_font_buffer(uint32_t unicode, int font_size);

private:
  fontmanager();
  ~fontmanager();

  static FT_Error face_requester(FTC_FaceID face_id, FT_Library library,
                                 FT_Pointer request_data, FT_Face *aface);

private:
  static MyFaceRec s_faces[1];
  FT_Library m_library;
  FTC_Manager m_manager;
  FTC_SBitCache m_sbitcache;
  FTC_CMapCache m_cmapcache;
  std::shared_mutex m_rwmutex;
};