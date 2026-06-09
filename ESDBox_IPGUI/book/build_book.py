#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
ESDBox_IPGUI 技术书籍构建器（Markdown → HTML → DOCX/PDF）

不依赖 python-docx，使用纯 Python 标准库将 Markdown 渲染为专业化排版的
HTML 文件，然后通过 Microsoft Word COM 自动化或浏览器另存为来生成 DOCX/PDF。

使用方法:
  python build_book.py            生成 HTML 和 DOCX
  python build_book.py --html     仅生成 HTML
  python build_book.py --pdf      仅生成 PDF (需先有 DOCX)
"""

import os
import re
import sys
import subprocess

BOOK_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(BOOK_DIR, "src")
OUTPUT_DIR = os.path.join(BOOK_DIR, "output")

DOCX_FILE = os.path.join(OUTPUT_DIR, "ESDBox_IPGUI_嵌入式图形系统技术手册.docx")
HTML_FILE = os.path.join(OUTPUT_DIR, "ESDBox_IPGUI_嵌入式图形系统技术手册.html")

SOURCE_FILES = [
    ("00-preface.md", "前言"),
    ("01-gfx-engine.md", "第一章 嵌入式绘图引擎完全解析"),
    ("02-composite-system.md", "第二章 混合合成 Composite 系统"),
]


def escape_html(text):
    return text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')


def md_to_html(md_text):
    """简易 Markdown → HTML 转换器（纯 Python，无外部依赖）"""
    lines = md_text.split('\n')
    out = []
    in_code = False
    in_table = False
    table_rows = []
    in_quote = False
    i = 0

    while i < len(lines):
        line = lines[i]

        # 代码块
        if line.strip().startswith('```'):
            if in_code:
                out.append('</code></pre>')
                in_code = False
            else:
                out.append('<pre><code>')
                in_code = True
            i += 1
            continue

        if in_code:
            out.append(escape_html(line))
            i += 1
            continue

        # 引用块
        if line.startswith('> '):
            if not in_quote:
                out.append('<blockquote>')
                in_quote = True
            content = line[2:]
            content = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', content)
            content = re.sub(r'`([^`]+)`', r'<code>\1</code>', content)
            out.append(f'<p>{content}</p>')
            i += 1
            continue
        elif in_quote and not line.startswith('> '):
            out.append('</blockquote>')
            in_quote = False

        # 表格
        if line.strip().startswith('|') and line.strip().endswith('|'):
            cells = [c.strip() for c in line.split('|')[1:-1]]
            if not in_table:
                in_table = True
                out.append('<table>')
                out.append('<thead><tr>' + ''.join(f'<th>{c}</th>' for c in cells) + '</tr></thead>')
                i += 1
                # 跳过分隔行
                if i < len(lines) and '---' in lines[i]:
                    i += 1
                out.append('<tbody>')
                continue
            else:
                out.append('<tr>' + ''.join(f'<td>{c}</td>' for c in cells) + '</tr>')
                i += 1
                # 检查是否还有表格行
                if i < len(lines) and lines[i].strip().startswith('|'):
                    continue
                out.append('</tbody></table>')
                in_table = False
                continue

        if in_table and not line.strip():
            out.append('</tbody></table>')
            in_table = False
            i += 1
            continue

        # 标题
        if line.startswith('# ') and not line.startswith('## '):
            out.append(f'<h1>{line[2:].strip()}</h1>')
        elif line.startswith('## '):
            out.append(f'<h2>{line[3:].strip()}</h2>')
        elif line.startswith('### '):
            out.append(f'<h3>{line[4:].strip()}</h3>')
        elif line.startswith('#### '):
            out.append(f'<h4>{line[5:].strip()}</h4>')

        # 水平线
        elif line.strip() == '---':
            out.append('<hr>')

        # 普通段落
        elif line.strip():
            if re.match(r'^\[.+\]:\s*$', line.strip()):
                i += 1
                continue

            content = line
            # 粗体
            content = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', content)
            # 行内代码
            content = re.sub(r'`([^`]+)`', r'<code>\1</code>', content)
            # 斜体
            content = re.sub(r'\*([^*]+)\*', r'<em>\1</em>', content)
            out.append(f'<p>{content}</p>')

        i += 1

    # 清理未关闭的标签
    if in_code:
        out.append('</code></pre>')
    if in_table:
        out.append('</tbody></table>')
    if in_quote:
        out.append('</blockquote>')

    return '\n'.join(out)


def build_html():
    """生成 HTML 文件"""
    print("[构建] 生成 HTML...")

    css = """
    <style>
        /* === 页面设置 === */
        @page { size: 185mm 260mm; margin: 22mm 20mm 20mm 22mm; }

        /* === 封面样式 === */
        .cover { text-align: center; padding: 8em 0; page-break-after: always; }
        .cover .book-title-en { font-size: 2.4em; font-weight: bold;
            color: #1a3c6d; letter-spacing: 0.05em; margin-bottom: 0.3em; }
        .cover .book-title-cn { font-size: 1.8em; font-weight: bold;
            color: #2c5f9e; margin-bottom: 0.8em; }
        .cover .book-subtitle { font-size: 1.05em; color: #666; margin-bottom: 5em;
            font-style: italic; }
        .cover .book-info { font-size: 0.95em; color: #888; margin-top: 0.8em; }

        /* === 正文 === */
        body { font-family: 'Times New Roman', '宋体', SimSun, serif;
               font-size: 11.5pt; line-height: 1.85;
               color: #1a1a2e; max-width: 100%; padding: 0 1em; }

        h1 { font-size: 1.55em; color: #1a3c6d; border-bottom: 2px solid #2c5f9e;
             padding-bottom: 0.35em; margin: 2em 0 0.8em 0;
             page-break-before: always; }
        h1:first-of-type { page-break-before: auto; }

        h2 { font-size: 1.28em; color: #2c5f9e; margin: 1.6em 0 0.6em 0;
             border-bottom: 1px solid #ddd; padding-bottom: 0.25em; }

        h3 { font-size: 1.1em; color: #3a7cbf; margin: 1.3em 0 0.5em 0; }

        h4 { font-size: 1.02em; color: #444; margin: 1em 0 0.4em 0; }

        p { margin: 0.4em 0; text-align: justify; }

        /* 代码块 */
        pre { background: #f8f8f8; border-left: 4px solid #2c5f9e;
              padding: 0.7em 1em; margin: 0.6em 0;
              font-family: Consolas, 'Courier New', monospace;
              font-size: 0.82em; line-height: 1.45;
              overflow-x: auto; white-space: pre-wrap; word-wrap: break-word; }
        code { font-family: Consolas, 'Courier New', monospace;
               background: #f0f0f0; padding: 1px 5px;
               font-size: 0.88em; color: #c7254e; }
        pre code { background: none; padding: 0; color: #1a1a2e; font-size: inherit; }

        /* 表格 */
        table { border-collapse: collapse; margin: 0.8em 0; width: 100%; }
        th, td { border: 1px solid #bbb; padding: 6px 10px; font-size: 0.92em; }
        th { background: #2c5f9e; color: white; font-weight: bold; }
        tr:nth-child(even) { background: #f5f8fc; }

        /* 引用 */
        blockquote { border-left: 4px solid #ccc; margin: 0.8em 0;
                     padding: 0.4em 1em; color: #555;
                     background: #f9f9f9; font-style: italic; }
        blockquote p { margin: 0.3em 0; }

        /* 分隔线 */
        hr { border: none; border-top: 1px solid #ddd; margin: 1.5em 0; }

        /* 图 */
        .figure { text-align: center; margin: 1em 0; font-size: 0.92em;
                  color: #555; font-style: italic; }
        .figure pre { display: inline-block; text-align: left;
                      background: #fafafa; border: 1px solid #ddd;
                      border-left: none; font-style: normal;
                      color: #333; line-height: 1.3; }
        .figure .caption { margin-top: 0.3em; }

        /* 打印 */
        @media print {
            body { font-size: 11pt; padding: 0; }
            pre { page-break-inside: avoid; }
            h1, h2, h3 { page-break-after: avoid; }
            .page-break { page-break-before: always; }
            .no-break { page-break-before: auto; }
        }
    </style>
    """

    parts = [
        '<!DOCTYPE html><html lang="zh-CN"><head><meta charset="utf-8">',
        '<title>ESDBox_IPGUI 嵌入式图形系统技术手册</title>',
        css, '</head><body>\n',
    ]

    # 封面
    parts.append("""
    <div class="cover">
        <div class="book-title-en">ESDBox_IPGUI</div>
        <div class="book-title-cn">嵌入式图形系统技术手册</div>
        <div class="book-subtitle">—— 绘图引擎与颜色合成系统深度解析 ——</div>

        <div class="book-info">模块聚焦: core/gfx（图形渲染）与 core/composite（颜色合成）</div>
        <div class="book-info">适用环境: 嵌入式 MCU / 低功耗处理器</div>
        <div class="book-info">语言栈: 纯 C99，无运行时依赖</div>
    </div>
    """)

    # 各章节
    for sf, chapter_title in SOURCE_FILES:
        src_path = os.path.join(SRC_DIR, sf)
        if not os.path.exists(src_path):
            print(f"  [警告] 未找到 {src_path}")
            continue
        print(f"  [处理] {sf}")
        with open(src_path, 'r', encoding='utf-8') as f:
            md = f.read()
        html_body = md_to_html(md)
        parts.append(f'<!-- 章节: {chapter_title} -->\n')
        parts.append(html_body)
        parts.append('\n<div class="page-break"></div>\n')

    parts.append('</body></html>')

    with open(HTML_FILE, 'w', encoding='utf-8') as f:
        f.write('\n'.join(parts))

    size_kb = os.path.getsize(HTML_FILE) / 1024
    print(f"\n[完成] HTML 已生成: {HTML_FILE}")
    print(f"        文件大小: {size_kb:.1f} KB")
    return HTML_FILE


def html_to_docx_powershell(html_path, docx_path):
    """通过 PowerShell Word COM 将 HTML 转为 DOCX"""
    ps_script = os.path.join(BOOK_DIR, "convert_to_docx.ps1")
    cmd = [
        "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
        "-File", ps_script,
        "-HtmlPath", html_path,
        "-DocxPath", docx_path
    ]
    print(f"[构建] 通过 Word COM 生成 DOCX...")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        print(result.stdout)
        if result.returncode != 0:
            print(result.stderr)
            return False
        return os.path.exists(docx_path)
    except subprocess.TimeoutExpired:
        print("  [错误] Word COM 转换超时")
        return False
    except Exception as e:
        print(f"  [错误] {e}")
        return False


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    use_com = "--no-com" not in sys.argv

    # 1. 生成 HTML
    html_path = build_html()

    # 2. 尝试 Word COM → DOCX
    docx_ok = False
    if use_com:
        docx_ok = html_to_docx_powershell(html_path, DOCX_FILE)

    # 3. 总结
    print("\n" + "=" * 60)
    print("  构建总结")
    print("=" * 60)
    print(f"  [HTML]  OK  {os.path.basename(HTML_FILE)} ({os.path.getsize(HTML_FILE)/1024:.1f} KB)")

    if docx_ok:
        print(f"  [DOCX]  OK  {os.path.basename(DOCX_FILE)} ({os.path.getsize(DOCX_FILE)/1024:.1f} KB)")
    else:
        print(f"  [DOCX]  --  Word COM 不可用或转换失败")
        print(f"           -> 手动操作: 用 Word 打开 HTML -> 另存为 DOCX")
        print(f"           -> 或运行: powershell -File book/convert_to_docx.ps1")

    print(f"\n  输出目录: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
