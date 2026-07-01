#!/usr/bin/env python3
"""
将 PNG 图标转为 IPGUI icon mask C 数组。

用法:
    python png_to_icon_mask.py icon.png icon_play [output.h]

    第三个参数指定输出文件路径，缺省则打印到终端。
    输出文件名建议用 .c 或 .h，内容可直接 #include 到项目中。

依赖: pip install Pillow
"""

import sys
import os
from PIL import Image


def png_to_mask_c_array(png_path: str, var_name: str) -> str:
    img = Image.open(png_path).convert("RGBA")
    w, h = img.size
    pixels = img.load()

    # 提取 alpha 通道 → u8 数组，行主序
    mask = []
    for y in range(h):
        for x in range(w):
            _, _, _, a = pixels[x, y]
            mask.append(a)

    # 生成 C 数组字符串
    lines = []
    lines.append(f"/* Auto-generated from {os.path.basename(png_path)}, w={w} h={h} */")
    lines.append(f"static const u8_t {var_name}_mask[{w} * {h}] = {{")

    hex_bytes = [f"0x{b:02X}" for b in mask]
    row_width = 16
    for i in range(0, len(hex_bytes), row_width):
        lines.append("    " + ", ".join(hex_bytes[i:i + row_width]) + ",")

    lines.append("};")
    lines.append("")
    lines.append(f"/* 使用方法:")
    lines.append(f"   ipgui_icon_data_t {var_name} = {{")
    lines.append(f"       .w = {w}, .h = {h},")
    lines.append(f"       .mask = (u8_t *){var_name}_mask,")
    lines.append(f"   }};")
    lines.append(f"*/")

    return "\n".join(lines)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    png_path  = sys.argv[1]
    var_name  = sys.argv[2] if len(sys.argv) > 2 else "icon"
    out_path  = sys.argv[3] if len(sys.argv) > 3 else None

    if not os.path.exists(png_path):
        print(f"错误: 文件不存在 — {png_path}")
        sys.exit(1)

    output = png_to_mask_c_array(png_path, var_name)

    if out_path:
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(output)
        print(f"已生成: {out_path} ({os.path.getsize(out_path)} bytes)")
    else:
        print(output)


if __name__ == "__main__":
    main()
