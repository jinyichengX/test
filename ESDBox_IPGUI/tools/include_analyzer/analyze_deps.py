"""
追踪 gfx/ 和 composite/ 的完整传递依赖，输出移植所需文件清单。

用法: py analyze_deps.py
"""
import os, re, json
from pathlib import Path
from collections import defaultdict

ROOT = Path(r"m:\test\ESDBox_IPGUI")
GFX_DIR = ROOT / "core/gfx"
COMPOSITE_DIR = ROOT / "core/composite"

# 收集 target 目录下的所有文件
target_files = set()
for d in [GFX_DIR, COMPOSITE_DIR]:
    for f in d.rglob("*"):
        if f.suffix in ('.c', '.h'):
            target_files.add(f.relative_to(ROOT).as_posix())

# 项目所有头文件索引
all_headers = {}
for p in ROOT.rglob("*.h"):
    rel = p.relative_to(ROOT).as_posix()
    all_headers[rel] = str(p)

# 文件名反向查表
name_to_rel = {}
for rel in all_headers:
    name = rel.split("/")[-1]
    prev = name_to_rel.get(name)
    if prev is None or rel.count("/") < prev.count("/"):
        name_to_rel[name] = rel

def resolve(inc_str):
    if inc_str in all_headers:
        return inc_str
    name = inc_str.split("/")[-1]
    if name in name_to_rel:
        return name_to_rel[name]
    for rel in all_headers:
        if rel.endswith("/" + inc_str):
            return rel
    return None

def parse_includes(filepath):
    incs = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                m = re.match(r'^\s*#include\s+"([^"]+)"', line)
                if m:
                    incs.append(m.group(1))
    except:
        pass
    return incs

def trace(rel, visited=None):
    """返回 rel 及其所有传递 include 的项目内头文件"""
    if visited is None:
        visited = set()
    if rel in visited or rel not in all_headers:
        return visited
    visited.add(rel)
    for inc in parse_includes(all_headers[rel]):
        r = resolve(inc)
        if r and r not in visited:
            trace(r, visited)
    return visited

# 所有 gfx/composite 文件
all_target = set()
for f in target_files:
    all_target.add(f)

# 追踪依赖
external_deps = {}  # target_file -> set of external files it depends on

for f in sorted(target_files):
    if not f.endswith('.h'):
        continue  # 只追踪 .h，因为 .c 的 include 也走 .h
    all_reachable = trace(f)
    ext = all_reachable - all_target
    if ext:
        external_deps[f] = ext

# 也追踪 .c 文件直接 include 的但不在 target 中的
for f in sorted(target_files):
    if not f.endswith('.c'):
        continue
    abs_path = all_headers.get(f)  # .c 不在 all_headers 里
    if not abs_path:
        abs_path = str(ROOT / f)
    if not os.path.isfile(abs_path):
        continue
    for inc in parse_includes(abs_path):
        r = resolve(inc)
        if r and r not in all_target:
            external_deps.setdefault(f, set()).add(r)
        
# 收集所有外部依赖（去重），按目录分组
all_ext = set()
for ext_set in external_deps.values():
    all_ext |= ext_set

# 分类
by_dir = defaultdict(list)
for e in sorted(all_ext):
    d = e.split("/")[0]
    by_dir[d].append(e)

# 打印
print("=" * 70)
print(f"gfx/composite 内部文件: {len(all_target)}")
print(f"外部依赖文件: {len(all_ext)}")
print("=" * 70)

for d, files in sorted(by_dir.items()):
    print(f"\n### {d}/ ({len(files)} files)")
    for f in sorted(files):
        # 哪些内部文件依赖它
        users = [k for k, v in external_deps.items() if f in v]
        print(f"  {f}")
        for u in sorted(users)[:5]:
            print(f"    <- {u}")
        if len(users) > 5:
            print(f"    ... and {len(users)-5} more")

# 输出 JSON
output = {
    "target_files": sorted(all_target),
    "external_deps": {k: sorted(v) for k, v in external_deps.items()},
    "all_external": sorted(all_ext),
    "by_directory": {d: sorted(v) for d, v in by_dir.items()},
}
with open(ROOT / "tools" / "dep_analysis.json", "w", encoding="utf-8") as f:
    json.dump(output, f, ensure_ascii=False, indent=2)
print(f"\n结果已写入 tools/dep_analysis.json")
