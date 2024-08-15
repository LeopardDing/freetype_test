from fontTools.ttLib import TTFont
from fontTools.ttLib.woff2 import compress
 
ttf_file_path = 'build/SourceHanSansCN-Normal.otf'
woff2_file_path = 'build/SourceHanSansCN-Normal.woff2'

# 转换方法1
# compress(ttf_file_path, woff2_file_path)

# 转换方法2
f = TTFont(ttf_file_path)
f.flavor='woff2'
f.save(woff2_file_path)
