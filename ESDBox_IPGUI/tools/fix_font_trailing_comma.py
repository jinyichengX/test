"""Fix quicksand font files: remove trailing comma after inline row comments.

Bug: The font generation script produces lines like:
    0x00, 0x00, /* 行 0 */,
The trailing comma after */ causes GCC to see two consecutive commas
(0x00, ,) after comment removal, which is invalid C syntax.

Fix: Remove the trailing comma, resulting in:
    0x00, 0x00, /* 行 0 */
"""

import os
import re
import glob

FONT_DIR = os.path.join(os.path.dirname(__file__), '..', 'font', 'quicksand')


def fix_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern: hex values, followed by /* 行 N */,
    # Replace trailing comma after the comment
    # Looking for: /* 行 N */, (with optional whitespace before comma)
    pattern = re.compile(r'(/\*\s*行\s*\d+\s*\*/)\s*,')
    fixed, count = pattern.subn(r'\1', content)

    if count > 0:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(fixed)
        print(f'{os.path.basename(filepath)}: {count} fixes')
        return count
    return 0


def main():
    files = sorted(glob.glob(os.path.join(FONT_DIR, 'quicksand_medium_*px.c')))
    print(f'Processing {len(files)} font files...')
    print()

    total = 0
    for filepath in files:
        count = fix_file(filepath)
        total += count

    print()
    print(f'Total fixes: {total}')


if __name__ == '__main__':
    main()
