#!/usr/bin/env python3
"""
IPML Compiler — 将 .ipml 声明式 UI 源码编译为 .ipb 二进制格式

用法:
    python ipml_compiler.py main.ipml [main.ipb]

输入: .ipml 文本文件
输出: .ipb 二进制文件（可直接嵌入 MCU 固件或从文件系统加载）

============ IPML 语法规范 ============

Widget [name:"名称",] attr1=val1, attr2=val2, ... {
    Widget attr1=val1 { ... }
    Widget attr1=val1
}

支持的属性:
    x, y, w, h     — 位置和尺寸 (整数, 像素)
    name           — 调试名称 (字符串, 双引号可选)
    render         — 渲染回调函数名 (字符串)
    event          — 事件处理回调函数名 (字符串)
    flags          — 位掩码标志 (整数)
    scroll_dir     — 滚动方向: x | y | gesture (字符串枚举)

示例:
    Widget x=10, y=20, w=100, h=200, render=my_render {
        Widget x=5, y=5, w=80, h=80, render=child_render
    }
"""

import sys
import struct
import re

VERSION = 1

# 属性位掩码
ATTR_X          = 0
ATTR_Y          = 1
ATTR_W          = 2
ATTR_H          = 3
ATTR_NAME       = 4
ATTR_RENDER     = 5
ATTR_EVENT      = 6
ATTR_FLAGS      = 7
ATTR_SCROLL_DIR = 8

SCROLL_DIR_MAP = {'x': 0, 'y': 1, 'gesture': 2}


class WidgetNode:
    def __init__(self):
        self.attrs = {}   # {attr_name: value}
        self.children = []

    def has(self, key):
        return key in self.attrs

    def get(self, key, default=None):
        return self.attrs.get(key, default)


class IPMLParser:
    """递归下降解析器"""

    def __init__(self, source, filename='<input>'):
        self.src = source
        self.pos = 0
        self.filename = filename

    def error(self, msg):
        line = self.src[:self.pos].count('\n') + 1
        ctx = self.src[max(0, self.pos-20):self.pos+20].replace('\n','\\n')
        raise SyntaxError(f"{self.filename}:{line}: {msg}  near \"...{ctx}...\"")

    def skip_ws(self):
        while self.pos < len(self.src) and self.src[self.pos] in ' \t\n\r':
            self.pos += 1

    def skip_ws_and_comments(self):
        while self.pos < len(self.src):
            c = self.src[self.pos]
            if c in ' \t\n\r':
                self.pos += 1
            elif c == '/' and self.pos + 1 < len(self.src):
                if self.src[self.pos + 1] == '/':
                    self.pos += 2
                    while self.pos < len(self.src) and self.src[self.pos] != '\n':
                        self.pos += 1
                elif self.src[self.pos + 1] == '*':
                    self.pos += 2
                    while self.pos + 1 < len(self.src):
                        if self.src[self.pos] == '*' and self.src[self.pos + 1] == '/':
                            self.pos += 2
                            break
                        self.pos += 1
                else:
                    break
            else:
                break

    def peek(self):
        return self.src[self.pos] if self.pos < len(self.src) else ''

    def expect(self, ch):
        if self.peek() == ch:
            self.pos += 1
            return
        self.error(f"expected '{ch}'")

    def read_identifier(self):
        start = self.pos
        while self.pos < len(self.src) and (self.src[self.pos].isalnum() or self.src[self.pos] == '_'):
            self.pos += 1
        if self.pos == start:
            self.error("expected identifier")
        return self.src[start:self.pos]

    def read_integer(self):
        start = self.pos
        if self.peek() == '-':
            self.pos += 1
        while self.pos < len(self.src) and self.src[self.pos].isdigit():
            self.pos += 1
        if (self.pos - start) == 0 or ((self.pos - start) == 1 and self.src[start] == '-'):
            self.error("expected integer")
        return int(self.src[start:self.pos])

    def read_string(self):
        """读取字符串: "quoted" 或 bare_identifier"""
        self.skip_ws_and_comments()
        if self.peek() == '"':
            self.pos += 1
            start = self.pos
            while self.pos < len(self.src) and self.src[self.pos] != '"':
                if self.src[self.pos] == '\\':
                    self.pos += 1
                self.pos += 1
            val = self.src[start:self.pos]
            self.expect('"')
            return val
        else:
            return self.read_identifier()

    def read_attr_val(self):
        """读取属性值: 字符串或整数"""
        self.skip_ws_and_comments()
        ch = self.peek()

        if ch == '"':
            return self.read_string()
        elif ch == '-' or ch.isdigit():
            return self.read_integer()
        elif ch.isalpha() or ch == '_':
            # 可能是枚举字符串或标识符
            ident = self.read_identifier()
            return ident
        else:
            self.error("expected attribute value")

    def parse_attrs(self):
        """解析逗号分隔的属性列表: name:val, name2=val2, ..."""
        attrs = {}
        self.skip_ws_and_comments()

        while self.peek() not in ('{', '}', ''):
            # 遇到下一个 Widget 关键字则停止（防止把兄弟 Widget 当属性名）
            if self.src[self.pos:self.pos+6] == 'Widget' and (
                    self.pos+6 >= len(self.src) or not self.src[self.pos+6].isalnum()):
                break
            # 读取属性名
            key = self.read_identifier()

            # 检查冒号语法 name:"value"
            self.skip_ws_and_comments()
            if self.peek() == ':':
                self.pos += 1
                val = self.read_attr_val()
                attrs[key] = val
            elif self.peek() == '=':
                self.pos += 1
                val = self.read_attr_val()
                attrs[key] = val
            else:
                self.error("expected ':' or '=' after attribute name")

            # 跳过逗号
            self.skip_ws_and_comments()
            if self.peek() == ',':
                self.pos += 1
            self.skip_ws_and_comments()
        return attrs

    def parse_widget(self):
        """解析一个 Widget 节点"""
        self.skip_ws_and_comments()
        kw = self.read_identifier()
        if kw != 'Widget':
            self.error(f"expected 'Widget', got '{kw}'")

        node = WidgetNode()
        node.attrs = self.parse_attrs()

        # 检查子节点
        self.skip_ws_and_comments()
        if self.peek() == '{':
            self.pos += 1
            self.skip_ws_and_comments()
            while self.peek() != '}' and self.peek() != '':
                child = self.parse_widget()
                node.children.append(child)
                self.skip_ws_and_comments()
            self.expect('}')
        return node

    def parse(self):
        """解析整个文件，返回 WidgetNode 列表（顶层可能有多个）"""
        widgets = []
        self.skip_ws_and_comments()
        while self.pos < len(self.src):
            widgets.append(self.parse_widget())
            self.skip_ws_and_comments()
        return widgets


class IPBEncoder:
    """IPB 二进制编码器"""

    def __init__(self):
        self.buf = bytearray()

    def u8(self, v):
        self.buf.append(v & 0xFF)

    def u16(self, v):
        self.buf.append(v & 0xFF)
        self.buf.append((v >> 8) & 0xFF)

    def s16(self, v):
        if v < 0:
            v = (1 << 16) + v
        self.u16(v)

    def str_u8(self, s):
        b = s.encode('utf-8')[:255]
        self.u8(len(b))
        self.buf.extend(b)

    def encode_node(self, node):
        mask = 0
        body = bytearray()

        # 按位序编码各属性
        def _enc(bit, fmt_fn, *args):
            nonlocal mask
            mask |= (1 << bit)
            fmt_fn(*args)

        body_enc = type('_enc_ctx', (), {
            'u8': lambda v: body.extend([v & 0xFF]),
            'u16': lambda v: (body.extend([v & 0xFF, (v>>8) & 0xFF])),
            's16': lambda v: (struct.pack_into('<h', body, len(body), *[v]),
                              body.extend([0,0])) if False else (
                                  body.extend(struct.pack('<h', v))),
        })

        # Use a simpler approach
        def _append_u16(buf, v):
            buf.append(v & 0xFF)
            buf.append((v >> 8) & 0xFF)

        def _append_s16(buf, v):
            b = struct.pack('<h', v)
            buf.extend(b)

        def _append_u8(buf, v):
            buf.append(v & 0xFF)

        def _append_str(buf, s):
            b = s.encode('utf-8')[:254]
            buf.append(len(b))
            buf.extend(b)
            buf.append(0)  # null terminator

        if node.has('x'):
            mask |= (1 << ATTR_X)
            _append_s16(body, int(node.get('x')))
        if node.has('y'):
            mask |= (1 << ATTR_Y)
            _append_s16(body, int(node.get('y')))
        if node.has('w'):
            mask |= (1 << ATTR_W)
            _append_s16(body, int(node.get('w')))
        if node.has('h'):
            mask |= (1 << ATTR_H)
            _append_s16(body, int(node.get('h')))
        if node.has('name'):
            mask |= (1 << ATTR_NAME)
            _append_str(body, str(node.get('name')))
        if node.has('render'):
            mask |= (1 << ATTR_RENDER)
            _append_str(body, str(node.get('render')))
        if node.has('event'):
            mask |= (1 << ATTR_EVENT)
            _append_str(body, str(node.get('event')))
        if node.has('flags'):
            mask |= (1 << ATTR_FLAGS)
            _append_u16(body, int(node.get('flags')))
        if node.has('scroll_dir'):
            mask |= (1 << ATTR_SCROLL_DIR)
            sd = node.get('scroll_dir')
            if isinstance(sd, str):
                sd_val = SCROLL_DIR_MAP.get(sd.lower().strip(), 0)
            else:
                sd_val = int(sd)
            _append_u8(body, sd_val)

        # 写入 mask
        _append_u16(self.buf, mask)
        self.buf.extend(body)

        # 子节点
        self.u8(len(node.children))
        for child in node.children:
            self.encode_node(child)

    def encode(self, widgets):
        # Magic
        self.buf = bytearray(b'IPB\x00')
        # Version
        self.u16(VERSION)

        # 预留 total_size 占位（稍后回填）
        size_pos = len(self.buf)
        self.u16(0)

        # 编码所有顶层节点
        data_start = len(self.buf)
        for w in widgets:
            self.encode_node(w)

        # 回填 total_size
        total = len(self.buf) - 8
        self.buf[size_pos] = total & 0xFF
        self.buf[size_pos + 1] = (total >> 8) & 0xFF

        return bytes(self.buf)


def main():
    if len(sys.argv) < 2:
        print(f"用法: {sys.argv[0]} <input.ipml> [output.ipb]")
        print()
        print("将 .ipml 声明式 UI 源码编译为 .ipb 二进制格式。")
        print("如不指定输出文件，默认输出为 <input>.ipb")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else input_path.rsplit('.', 1)[0] + '.ipb'

    with open(input_path, 'r', encoding='utf-8') as f:
        source = f.read()

    parser = IPMLParser(source, filename=input_path)
    try:
        widgets = parser.parse()
    except SyntaxError as e:
        print(f"Parse error: {e}", file=sys.stderr)
        sys.exit(1)

    if not widgets:
        print("Warning: no widgets found in input file", file=sys.stderr)

    encoder = IPBEncoder()
    data = encoder.encode(widgets)

    with open(output_path, 'wb') as f:
        f.write(data)

    widget_count = count_widgets(widgets)
    print(f"Compiled: {input_path} → {output_path}")
    print(f"  {widget_count} widgets, {len(data)} bytes IPB binary")


def count_widgets(nodes):
    n = len(nodes)
    for node in nodes:
        n += count_widgets(node.children)
    return n


if __name__ == '__main__':
    main()
