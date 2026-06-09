#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
ESDBox_IPGUI 技术书籍格式转换脚本

功能:
  1. 将 Markdown 源文件转换为 DOCX（主要交付格式）
  2. 尝试将 DOCX 转换为 PDF（需要 Word 或 LibreOffice）
  3. 生成 HTML 版本（浏览器可打印为 PDF）

使用方法:
  python build_all.py            # 生成所有格式
  python build_all.py --docx     # 仅生成 DOCX
  python build_all.py --pdf       # 仅生成 PDF（需先有 DOCX）
  python build_all.py --html      # 仅生成 HTML
"""

import os
import sys
import subprocess
import argparse

BOOK_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(BOOK_DIR, "src")
OUTPUT_DIR = os.path.join(BOOK_DIR, "output")

DOCX_FILE = os.path.join(OUTPUT_DIR, "ESDBox_IPGUI_嵌入式图形系统技术手册.docx")
PDF_FILE = os.path.join(OUTPUT_DIR, "ESDBox_IPGUI_嵌入式图形系统技术手册.pdf")
HTML_FILE = os.path.join(OUTPUT_DIR, "ESDBox_IPGUI_嵌入式图形系统技术手册.html")

SOURCE_FILES = ["00-preface.md", "01-gfx-engine.md", "02-composite-system.md"]


def build_docx():
    """生成 DOCX"""
    import build_docx
    return build_docx.build()


def docx_to_pdf_word(docx_path, pdf_path):
    """通过 Microsoft Word COM 将 DOCX 转换为 PDF"""
    try:
        import win32com.client
        word = win32com.client.Dispatch("Word.Application")
        word.Visible = False
        doc = word.Documents.Open(docx_path)
        doc.ExportAsFixedFormat(pdf_path, 17)  # 17 = wdExportFormatPDF
        doc.Close()
        word.Quit()
        return True
    except ImportError:
        print("  [提示] 需要 pywin32: pip install pywin32")
        return False
    except Exception as e:
        print(f"  [错误] Word COM 转换失败: {e}")
        return False


def docx_to_pdf_libreoffice(docx_path, pdf_dir):
    """通过 LibreOffice 将 DOCX 转换为 PDF"""
    try:
        result = subprocess.run(
            ["soffice", "--headless", "--convert-to", "pdf",
             "--outdir", pdf_dir, docx_path],
            capture_output=True, text=True, timeout=120)
        if result.returncode == 0:
            return True
        else:
            print(f"  [错误] LibreOffice: {result.stderr}")
            return False
    except FileNotFoundError:
        return False
    except Exception as e:
        print(f"  [错误] LibreOffice 转换失败: {e}")
        return False


def build_pdf():
    """生成 PDF（尝试多种方法）"""
    if not os.path.exists(DOCX_FILE):
        print("[错误] 请先生成 DOCX: python build_all.py --docx")
        return None

    print("[构建] 尝试将 DOCX 转换为 PDF...")

    # 方法1: Microsoft Word COM
    print("  [尝试] Microsoft Word COM...")
    if docx_to_pdf_word(DOCX_FILE, PDF_FILE):
        print(f"  [成功] PDF 已生成: {PDF_FILE}")
        return PDF_FILE

    # 方法2: LibreOffice
    print("  [尝试] LibreOffice...")
    if docx_to_pdf_libreoffice(DOCX_FILE, OUTPUT_DIR):
        pdf_output = os.path.join(OUTPUT_DIR,
            os.path.splitext(os.path.basename(DOCX_FILE))[0] + ".pdf")
        if pdf_output != PDF_FILE and os.path.exists(pdf_output):
            os.rename(pdf_output, PDF_FILE)
        print(f"  [成功] PDF 已生成: {PDF_FILE}")
        return PDF_FILE

    print("\n[备选方案] 无法通过 Word/LibreOffice 生成 PDF。")
    print("  请手动操作:")
    print(f"    1. 用 Microsoft Word 打开: {DOCX_FILE}")
    print(f'    2. 文件 → 另存为 → PDF → 保存到: {PDF_FILE}')
    print("  或运行: python build_all.py --html （生成可打印的 HTML）")
    return None


def build_html():
    """将 Markdown 源文件合并为单个 HTML 文件"""
    print("[构建] 生成 HTML...")

    css = """
    <style>
        body { max-width: 860px; margin: 2em auto; padding: 0 1.5em;
               font-family: 'Times New Roman', '宋体', serif; font-size: 13px;
               line-height: 1.8; color: #1a1a2e; }
        h1 { font-size: 1.8em; color: #1a3c6d; border-bottom: 2px solid #2c5f9e;
             padding-bottom: 0.3em; margin-top: 2em; }
        h2 { font-size: 1.4em; color: #2c5f9e; margin-top: 1.8em;
             border-bottom: 1px solid #ddd; padding-bottom: 0.2em; }
        h3 { font-size: 1.15em; color: #3a7cbf; margin-top: 1.3em; }
        h4 { font-size: 1.05em; color: #444; }
        pre { background: #f5f5f5; border-left: 3px solid #2c5f9e;
              padding: 0.8em 1em; overflow-x: auto; font-size: 0.88em;
              line-height: 1.5; font-family: 'Consolas', 'Courier New', monospace; }
        code { font-family: 'Consolas', 'Courier New', monospace;
               background: #f0f0f0; padding: 1px 4px; border-radius: 2px;
               font-size: 0.92em; color: #c7254e; }
        pre code { background: none; padding: 0; color: #1a1a2e; }
        table { border-collapse: collapse; margin: 1em 0; width: 100%; }
        th, td { border: 1px solid #ccc; padding: 8px 12px; text-align: left; }
        th { background: #2c5f9e; color: white; font-weight: bold; }
        blockquote { border-left: 4px solid #ddd; margin: 1em 0;
                     padding: 0.5em 1em; color: #555; font-style: italic;
                     background: #f9f9f9; }
        hr { border: none; border-top: 1px solid #ddd; margin: 2em 0; }
        .page-break { page-break-after: always; }
        @media print {
            body { max-width: 100%; font-size: 12px; }
            .page-break { page-break-after: always; }
            @page { margin: 2cm; }
        }
    </style>
    """

    import markdown
    md = markdown.Markdown(extensions=['extra', 'codehilite', 'toc'])

    parts = ['<!DOCTYPE html><html><head><meta charset="utf-8">',
             '<title>ESDBox_IPGUI 嵌入式图形系统技术手册</title>', css,
             '</head><body>\n']

    for sf in SOURCE_FILES:
        src_path = os.path.join(SRC_DIR, sf)
        if not os.path.exists(src_path):
            continue
        print(f"  [处理] {sf}")
        with open(src_path, 'r', encoding='utf-8') as f:
            md_content = f.read()
        html_body = md.convert(md_content)
        parts.append(html_body)
        parts.append('\n<div class="page-break"></div>\n')

    parts.append('</body></html>')

    with open(HTML_FILE, 'w', encoding='utf-8') as f:
        f.write('\n'.join(parts))

    print(f"  [完成] HTML 已生成: {HTML_FILE}")
    print(f"         文件大小: {os.path.getsize(HTML_FILE) / 1024:.1f} KB")
    print("  提示: 用浏览器打开此文件，Ctrl+P 即可打印/保存为 PDF。")
    return HTML_FILE


def main():
    parser = argparse.ArgumentParser(description="ESDBox_IPGUI 技术书籍构建工具")
    parser.add_argument("--docx", action="store_true", help="仅生成 DOCX")
    parser.add_argument("--pdf", action="store_true", help="仅生成 PDF")
    parser.add_argument("--html", action="store_true", help="仅生成 HTML")
    args = parser.parse_args()

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # 默认生成全部
    build_all = not (args.docx or args.pdf or args.html)

    results = {}

    if build_all or args.docx:
        results['DOCX'] = build_docx()

    if build_all or args.pdf:
        results['PDF'] = build_pdf()

    if build_all or args.html:
        results['HTML'] = build_html()

    # 总结
    print("\n" + "=" * 55)
    print("  构建总结")
    print("=" * 55)
    for fmt, path in results.items():
        if path and os.path.exists(path):
            size_kb = os.path.getsize(path) / 1024
            print(f"  [{fmt:6s}] ✓ {os.path.basename(path)} ({size_kb:.1f} KB)")
        else:
            print(f"  [{fmt:6s}] ✗ 未生成")

if __name__ == "__main__":
    main()
