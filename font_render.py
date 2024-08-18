from PIL import Image, ImageDraw
import freetype

# 创建一个图像对象
width, height = 800, 200
image = Image.new('RGB', (width, height), color=(255, 255, 255))
draw = ImageDraw.Draw(image)

# 加载字体
face = freetype.Face('OPPOSans-R.ttf')  # 替换为实际的字体文件路径
face.set_char_size(48 * 64)  # 设置字体大小

# 设定要渲染的文本
text = "你好 Hello world"
x, y = 50, 100  # 文本起始位置

# 渲染文本
pen_x = x
pen_y = y

for char in text:
    face.load_char(char)
    bitmap = face.glyph.bitmap
    top = face.glyph.bitmap_top
    left = face.glyph.bitmap_left
    
    # 将字符位图绘制到图像中
    for i in range(bitmap.rows):
        for j in range(bitmap.width):
            px = pen_x + left + j
            py = pen_y - top + i
            if bitmap.buffer[i * bitmap.width + j]:
                draw.point((px, py), fill=(0, 0, 0))
    
    # 移动画笔位置
    pen_x += face.glyph.advance.x // 64  # 位移单位是1/64像素

# 保存图像
image.save('freetype_text.png')
image.show()
