/* ==========================================================================
   PX4 Offboard 消息接口实战手册 — 交互引擎
   全部为 DOM/SVG 实现，不使用 <canvas>，可脱离网络离线运行。
   ========================================================================== */
(function () {
  "use strict";

  var PAGES = [];
  var current = 0;

  /* 标记 JS 可用：CSS 据此切换为单页翻页模式；脚本被禁用时全部页面依旧可见 */
  document.documentElement.classList.add("js");

  function $(sel, root) { return (root || document).querySelector(sel); }
  function $$(sel, root) { return Array.prototype.slice.call((root || document).querySelectorAll(sel)); }

  function parseJSONScript(node) {
    if (!node) return null;
    try { return JSON.parse(node.textContent); } catch (e) { return null; }
  }

  /* ------------------------------------------------------------ Navigation */

  function collectPages() {
    PAGES = $$(".page");
    PAGES.forEach(function (page, i) {
      page.dataset.index = String(i);
      var num = page.querySelector(".page-num-badge");
      if (num) num.textContent = "第 " + (i + 1) + " / " + PAGES.length + " 页";
    });
  }

  function buildTOC() {
    var nav = $("#toc");
    if (!nav) return;
    var frag = document.createDocumentFragment();
    var group = null, groupBox = null;

    PAGES.forEach(function (page, i) {
      var g = page.dataset.group || "其他";
      if (g !== group) {
        group = g;
        groupBox = document.createElement("div");
        groupBox.className = "toc-group";
        var h = document.createElement("p");
        h.className = "toc-group__title";
        h.textContent = g;
        groupBox.appendChild(h);
        frag.appendChild(groupBox);
      }
      var a = document.createElement("a");
      a.className = "toc-link";
      a.href = "#p" + (i + 1);
      a.dataset.index = String(i);
      a.innerHTML = '<span class="toc-link__num">' + String(i + 1).padStart(2, "0") +
        '</span><span class="toc-link__t">' + (page.dataset.title || "未命名") + "</span>";
      a.addEventListener("click", function (ev) {
        ev.preventDefault();
        go(i);
        closeDrawer();
      });
      groupBox.appendChild(a);
    });
    nav.innerHTML = "";
    nav.appendChild(frag);
  }

  function buildPagers() {
    PAGES.forEach(function (page, i) {
      if (page.querySelector(".pager")) return;
      var sheet = page.querySelector(".sheet") || page;
      var pager = document.createElement("nav");
      pager.className = "pager";
      pager.setAttribute("aria-label", "翻页");
      var prev = i > 0
        ? '<a href="#p' + i + '" data-go="' + (i - 1) + '"><span class="pager__dir">上一页</span><span class="pager__title">' + (PAGES[i - 1].dataset.title || "") + "</span></a>"
        : '<span><span class="pager__dir">上一页</span><span class="pager__title">已是第一页</span></span>';
      var next = i < PAGES.length - 1
        ? '<a class="pager__next" href="#p' + (i + 2) + '" data-go="' + (i + 1) + '"><span class="pager__dir">下一页</span><span class="pager__title">' + (PAGES[i + 1].dataset.title || "") + "</span></a>"
        : '<span class="pager__next"><span class="pager__dir">下一页</span><span class="pager__title">已是最后一页</span></span>';
      pager.innerHTML = prev + next;
      $$("a[data-go]", pager).forEach(function (a) {
        a.addEventListener("click", function (ev) { ev.preventDefault(); go(parseInt(a.dataset.go, 10)); });
      });
      sheet.appendChild(pager);
    });
  }

  function go(i) {
    if (i < 0 || i >= PAGES.length) return;
    current = i;
    PAGES.forEach(function (p, k) { p.classList.toggle("is-active", k === i); });
    var links = $$(".toc-link");
    links.forEach(function (a) {
      var on = parseInt(a.dataset.index, 10) === i;
      a.classList.toggle("is-active", on);
      if (on && a.scrollIntoView) a.scrollIntoView({ block: "nearest" });
    });
    var loc = $("#loc");
    if (loc) {
      loc.innerHTML = "<b>" + (PAGES[i].dataset.title || "") + "</b> · " +
        (PAGES[i].dataset.group || "");
    }
    var cnt = $("#count");
    if (cnt) cnt.textContent = (i + 1) + " / " + PAGES.length;
    var bar = $("#bar");
    if (bar) bar.style.inlineSize = (((i + 1) / PAGES.length) * 100).toFixed(2) + "%";
    if (location.hash !== "#p" + (i + 1)) {
      try { history.replaceState(null, "", "#p" + (i + 1)); } catch (e) { location.hash = "#p" + (i + 1); }
    }
    window.scrollTo({ top: 0, behavior: "auto" });
  }

  function fromHash() {
    var m = /^#p(\d+)$/.exec(location.hash || "");
    if (m) {
      var i = parseInt(m[1], 10) - 1;
      if (i >= 0 && i < PAGES.length) return i;
    }
    return 0;
  }

  function openDrawer() { $("#sidebar").classList.add("is-open"); $("#backdrop").classList.add("is-open"); }
  function closeDrawer() { $("#sidebar").classList.remove("is-open"); $("#backdrop").classList.remove("is-open"); }

  /* ----------------------------------------------------------------- Search */

  function initSearch() {
    var input = $("#search");
    if (!input) return;
    var timer = null;
    input.addEventListener("input", function () {
      clearTimeout(timer);
      timer = setTimeout(function () {
        var q = input.value.trim().toLowerCase();
        var links = $$(".toc-link");
        var hits = 0;
        links.forEach(function (a) {
          var page = PAGES[parseInt(a.dataset.index, 10)];
          var hay = ((page.dataset.title || "") + " " + (page.dataset.group || "") + " " +
            (page.dataset.kw || "") + " " + (page.textContent || "")).toLowerCase();
          var ok = !q || hay.indexOf(q) !== -1;
          a.style.display = ok ? "" : "none";
          a.classList.toggle("is-hit", !!q && ok && hits < 400);
          if (ok) hits++;
        });
        $$(".toc-group").forEach(function (g) {
          var visible = $$(".toc-link", g).some(function (a) { return a.style.display !== "none"; });
          g.style.display = visible ? "" : "none";
        });
        var empty = $("#tocEmpty");
        if (empty) empty.style.display = hits ? "none" : "block";
      }, 120);
    });
    input.addEventListener("keydown", function (ev) {
      if (ev.key === "Escape") { input.value = ""; input.dispatchEvent(new Event("input")); input.blur(); }
      if (ev.key === "Enter") {
        var first = $$(".toc-link").filter(function (a) { return a.style.display !== "none"; })[0];
        if (first) { go(parseInt(first.dataset.index, 10)); closeDrawer(); }
      }
    });
  }

  /* ------------------------------------------------------------------ Theme */

  function initTheme() {
    var KEY = "px4offboard.theme";
    var root = document.documentElement;
    var saved = null;
    try { saved = localStorage.getItem(KEY); } catch (e) { saved = null; }
    if (saved === "dark" || saved === "light") root.dataset.theme = saved;
    var btn = $("#theme");
    if (!btn) return;
    function sync() { btn.textContent = root.dataset.theme === "dark" ? "浅色" : "深色"; }
    sync();
    btn.addEventListener("click", function () {
      var next = root.dataset.theme === "dark" ? "light" : "dark";
      root.dataset.theme = next;
      try { localStorage.setItem(KEY, next); } catch (e) { /* ignore */ }
      sync();
      $$("[data-widget]").forEach(function (w) { refreshWidget(w); });
    });
  }

  /* -------------------------------------------------------------- Copy code */

  function initCopyButtons() {
    $$(".code").forEach(function (block) {
      var head = $(".code__head", block);
      if (!head || $(".code__copy", head)) return;
      var name = $(".code__name", head);
      var btn = document.createElement("button");
      btn.type = "button";
      btn.className = "code__copy";
      btn.textContent = "复制";
      btn.addEventListener("click", function () {
        var pre = $("pre", block);
        var text = pre ? pre.innerText : "";
        var done = function () { btn.textContent = "已复制"; setTimeout(function () { btn.textContent = "复制"; }, 1400); };
        if (navigator.clipboard && navigator.clipboard.writeText) {
          navigator.clipboard.writeText(text).then(done, function () { fallback(text, done); });
        } else { fallback(text, done); }
      });
      head.appendChild(btn);
      if (!name) { /* keep layout */ }
    });
  }

  function fallback(text, done) {
    var ta = document.createElement("textarea");
    ta.value = text; ta.setAttribute("readonly", "");
    ta.style.position = "fixed"; ta.style.opacity = "0";
    document.body.appendChild(ta); ta.select();
    try { document.execCommand("copy"); } catch (e) { /* ignore */ }
    document.body.removeChild(ta);
    done();
  }

  /* ------------------------------------------------------------------- Tabs */

  function initTabs(scope) {
    var tabs = $$(".tab", scope);
    tabs.forEach(function (tab) {
      if (tab.dataset.tabInit === "1") return;
      tab.dataset.tabInit = "1";
      tab.addEventListener("click", function () {
        var panelId = tab.getAttribute("aria-controls");
        $$(".tab", scope).forEach(function (t) { t.setAttribute("aria-selected", t === tab ? "true" : "false"); });
        $$(".tabpanel", scope).forEach(function (p) {
          p.classList.toggle("is-active", p.id === panelId);
          p.hidden = p.id !== panelId;
        });
      });
    });
  }

  /* -------------------------------------------------------------- Checklist */

  function initChecklist(widget) {
    var boxes = $$('input[type="checkbox"]', widget);
    var out = $("[data-cl-progress]", widget);
    function sync() {
      var n = boxes.filter(function (b) { return b.checked; }).length;
      if (out) out.textContent = "已完成 " + n + " / " + boxes.length + (n === boxes.length && boxes.length ? " · 全部通过" : "");
    }
    boxes.forEach(function (b) { b.addEventListener("change", sync); });
    sync();
  }

  /* --------------------------------------------------------- Decision tree */

  function initTree(widget) {
    var cfg = parseJSONScript($('script[type="application/json"]', widget));
    var body = $(".widget__body", widget);
    var out = $("[data-tree-out]", widget);
    if (!cfg || !body || !out) return;

    var host = $("[data-tree-steps]", widget) || body;
    host.innerHTML = "";

    cfg.steps.forEach(function (step, si) {
      var wrap = document.createElement("div");
      wrap.style.marginBlockEnd = "14px";
      var lab = document.createElement("p");
      lab.className = "small";
      lab.style.margin = "0 0 7px";
      lab.style.fontWeight = "700";
      lab.textContent = (si + 1) + ". " + step.label;
      wrap.appendChild(lab);
      var box = document.createElement("div");
      box.className = "opts";
      box.style.marginBlockEnd = "0";
      step.options.forEach(function (op, oi) {
        var l = document.createElement("label");
        l.className = "opt";
        var inp = document.createElement("input");
        inp.type = "radio"; inp.name = "tree-" + (widget.dataset.id || Math.random().toString(36).slice(2)) + "-" + step.k;
        inp.value = op.v;
        inp.addEventListener("change", function () {
          $$(".opt", box).forEach(function (el) { el.classList.toggle("is-on", $("input", el).checked); });
          render();
        });
        if (si === 0 && oi === 0) { inp.checked = true; l.classList.add("is-on"); }
        l.appendChild(inp);
        l.appendChild(document.createTextNode(op.t));
        box.appendChild(l);
      });
      wrap.appendChild(box);
      host.appendChild(wrap);
    });

    function render() {
      var picks = {};
      cfg.steps.forEach(function (step) {
        var sel = widget.querySelector('input[name="tree-' + (widget.dataset.id || "") + "-" + step.k + '"]:checked');
        if (!sel) {
          var radios = $$('input[type="radio"]', widget).filter(function (r) {
            return r.name.slice(-(step.k.length + 1)) === "-" + step.k;
          });
          sel = radios.filter(function (r) { return r.checked; })[0];
        }
        if (sel) picks[step.k] = sel.value;
      });
      var hit = null;
      (cfg.rules || []).forEach(function (r) {
        if (hit) return;
        var ok = Object.keys(r.match).every(function (k) { return picks[k] === r.match[k]; });
        if (ok) hit = r;
      });
      var res = hit || cfg.default || { verdict: "暂无匹配结论", detail: "" };
      var tone = res.tone || "info";
      var colorVar = tone === "danger" ? "--danger" : tone === "warn" ? "--warning" : tone === "ok" ? "--success" : "--page-brand";
      out.innerHTML =
        '<h5 style="color:var(' + colorVar + ')">' + res.verdict + "</h5>" +
        (res.detail ? "<p>" + res.detail + "</p>" : "") +
        (res.msgs && res.msgs.length
          ? '<p class="small" style="margin:0"><strong>要用的消息：</strong>' +
            res.msgs.map(function (m) { return '<code>' + m + "</code>"; }).join(" ") + "</p>"
          : "");
    }
    render();
  }

  /* --------------------------------------------------------- Field explorer */

  function initFields(widget) {
    var cfg = parseJSONScript($('script[type="application/json"]', widget));
    var out = $("[data-fields-out]", widget);
    var list = $(".fields", widget);
    if (!cfg || !out || !list) return;

    var rows = $$("li", list);
    var state = {};
    rows.forEach(function (li) {
      var nameEl = $(".fname", li);
      if (!nameEl) return;
      var name = nameEl.textContent.trim();
      var btns = $$(".fctl button", li);
      var on = btns.filter(function (b) { return b.classList.contains("is-on"); })[0] || btns[1];
      state[name] = on ? on.dataset.v : "nan";
      btns.forEach(function (b) {
        b.addEventListener("click", function () {
          state[name] = b.dataset.v;
          btns.forEach(function (x) { x.classList.toggle("is-on", x === b); });
          render();
        });
      });
    });

    /* 组合匹配：combo 的 key 里列出的 k=v 必须全部满足；
       多个命中时取最具体（条件最多）的一条；都不命中则用 default。 */
    function matchCombo() {
      var best = null, bestN = -1;
      var keys = Object.keys(cfg.combos || {});
      keys.forEach(function (k) {
        var parts = k.split(";").map(function (s) { return s.trim(); }).filter(Boolean);
        if (!parts.length) return;
        var ok = parts.every(function (p) {
          var i = p.indexOf("=");
          if (i < 0) return false;
          var name = p.slice(0, i).trim(), want = p.slice(i + 1).trim();
          return state[name] === want;
        });
        if (ok && parts.length > bestN) { best = cfg.combos[k]; bestN = parts.length; }
      });
      return best || cfg.default || null;
    }

    function render() {
      var lines = rows.map(function (li) {
        var nameEl = $(".fname", li);
        if (!nameEl) return "";
        var n = nameEl.textContent.trim();
        var spec = (cfg.fields || {})[n] || {};
        var txt = spec[state[n]] || "";
        var isNan = state[n] === "nan";
        return '<li><code>' + n + "</code> → <span style=\"color:var(" +
          (isNan ? "--page-text-muted" : "--page-brand-text") + ')\">' +
          (isNan ? "NaN（不控制该状态）" : "给定值（控制该状态）") + "</span>" +
          (txt ? " · " + txt : "") + "</li>";
      }).join("");

      var combo = matchCombo();
      var head = "";
      if (combo) {
        var tone = combo.tone || "info";
        var colorVar = tone === "danger" ? "--danger" : tone === "warn" ? "--warning" : tone === "ok" ? "--success" : "--page-brand";
        head = '<h5 style="color:var(' + colorVar + ')">' + combo.title + "</h5><p>" + combo.body + "</p>";
      }
      out.innerHTML = head + '<ul style="margin:0;padding-inline-start:18px;font-size:14px">' + lines + "</ul>";
    }
    render();
  }

  /* -------------------------------------------------------- Frame converter */

  function initFrame(widget) {
    var cfg = parseJSONScript($('script[type="application/json"]', widget));
    var out = $("[data-frame-out]", widget);
    if (!out) return;

    var inputs = $$('input[type="range"]', widget);
    var svg = $("svg", widget);

    function val(name) {
      var el = widget.querySelector('[data-axis="' + name + '"]');
      return el ? parseFloat(el.value) : 0;
    }
    function show() {
      var roll = val("roll"), pitch = val("pitch"), yaw = val("yaw");
      var px = val("px"), py = val("py"), pz = val("pz");

      var r = roll * Math.PI / 180, p = pitch * Math.PI / 180, y = yaw * Math.PI / 180;
      var cy = Math.cos(y / 2), sy = Math.sin(y / 2);
      var cp = Math.cos(p / 2), sp = Math.sin(p / 2);
      var cr = Math.cos(r / 2), sr = Math.sin(r / 2);
      var qw = cr * cp * cy + sr * sp * sy;
      var qx = sr * cp * cy - cr * sp * sy;
      var qy = cr * sp * cy + sr * cp * sy;
      var qz = cr * cp * sy - sr * sp * cy;

      var f = function (v) { return (Math.round(v * 1000) / 1000).toFixed(3); };

      var rows = [
        ["NED 位置 (北, 东, 下) m", "(" + f(px) + ", " + f(py) + ", " + f(pz) + ")"],
        ["ENU 位置 (东, 北, 上) m", "(" + f(py) + ", " + f(px) + ", " + f(-pz) + ")"],
        ["ENU→NED 换算", "N = E, E = N, D = −U"],
        ["四元数 (w, x, y, z)", "(" + f(qw) + ", " + f(qx) + ", " + f(qy) + ", " + f(qz) + ")"],
        ["yaw 弧度 / 角度", f(y) + " rad / " + f(yaw) + "°"],
        ["机身 FRD 前向在 NED 中的指向", "(" + f(Math.cos(p) * Math.cos(y)) + ", " +
          f(Math.cos(p) * Math.sin(y)) + ", " + f(-Math.sin(p)) + ")"]
      ];

      out.innerHTML = '<table class="tbl tbl--compact" style="margin:0"><tbody>' +
        rows.map(function (rr) { return "<tr><td>" + rr[0] + '</td><td class="mono fit">' + rr[1] + "</td></tr>"; }).join("") +
        "</tbody></table>";

      if (svg) {
        var plane = $("#plane", svg);
        if (plane) plane.setAttribute("transform", "translate(160 118) rotate(" + yaw + ")");
        var body = $("#bodytilt", svg);
        if (body) body.setAttribute("transform", "translate(160 118) rotate(" + yaw + ") skewY(" + (-pitch) + ")");
        var comp = $("#compass", svg);
        if (comp) comp.setAttribute("transform", "rotate(" + (-yaw) + " 160 118)");
      }
    }
    inputs.forEach(function (i) { i.addEventListener("input", show); });
    show();
  }

  /* -------------------------------------------------------------- Timeline */

  function initTimeline(widget) {
    var cfg = parseJSONScript($('script[type="application/json"]', widget));
    var steps = $$(".step", widget);
    var out = $("[data-tl-out]", widget);
    var idx = -1;

    function paint() {
      steps.forEach(function (s, i) {
        s.classList.toggle("is-on", i === idx);
        s.classList.toggle("is-past", i < idx);
      });
      if (out && cfg && cfg.steps[idx]) {
        var st = cfg.steps[idx];
        out.innerHTML = "<h5>t = " + st.t + "</h5><p>" + st.body + "</p>" +
          (st.msgs && st.msgs.length
            ? '<p class="small" style="margin:0"><strong>此阶段涉及的消息：</strong>' +
              st.msgs.map(function (m) { return "<code>" + m + "</code>"; }).join(" ") + "</p>"
            : "");
      }
    }
    function goTo(i) { idx = Math.max(0, Math.min(steps.length - 1, i)); paint(); }

    var prev = $("[data-tl-prev]", widget), next = $("[data-tl-next]", widget), play = $("[data-tl-play]", widget);
    if (prev) prev.addEventListener("click", function () { goTo(idx - 1); });
    if (next) next.addEventListener("click", function () { goTo(idx + 1); });
    if (play) play.addEventListener("click", function () {
      if (idx >= steps.length - 1) idx = -1;
      paint();
      var timer = setInterval(function () {
        if (idx >= steps.length - 1) { clearInterval(timer); play.textContent = "从头播放"; return; }
        goTo(idx + 1);
      }, 1500);
      play.textContent = "播放中…";
      setTimeout(function () { play.textContent = idx >= steps.length - 1 ? "从头播放" : "播放"; }, 1200);
    });
    steps.forEach(function (s, i) { s.style.cursor = "pointer"; s.addEventListener("click", function () { goTo(i); }); });
    if (steps.length) goTo(0);
  }

  /* ---------------------------------------------------- Message comparator */

  function initCompare(widget) {
    var boxes = $$('input[type="checkbox"][data-msg]', widget);
    var out = $("[data-cmp-out]", widget);
    var cfg = parseJSONScript($('script[type="application/json"]', widget));
    if (!out || !cfg) return;
    function render() {
      var sel = boxes.filter(function (b) { return b.checked; }).map(function (b) { return b.dataset.msg; });
      if (!sel.length) { out.innerHTML = '<p class="small" style="margin:0">请至少勾选一条消息。</p>'; return; }
      var common = cfg.rows.filter(function (r) { return sel.every(function (m) { return r.v[m] && r.v[m] === r.v[sel[0]]; }); });
      var diff = cfg.rows.filter(function (r) { return r.v[sel[0]] && common.indexOf(r) === -1; });
      out.innerHTML =
        "<p><strong>相同点（" + common.length + " 项）：</strong>" +
        (common.length ? common.map(function (r) { return "<code>" + r.k + "</code>"; }).join(" ") : "无") + "</p>" +
        '<div class="table-wrap" style="margin:0"><table class="tbl tbl--compact"><caption>差异对照</caption>' +
        "<thead><tr><th>字段 / 维度</th>" + sel.map(function (m) { return "<th>" + m + "</th>"; }).join("") + "</tr></thead><tbody>" +
        diff.map(function (r) {
          return "<tr><td><code>" + r.k + "</code></td>" +
            sel.map(function (m) { return "<td>" + (r.v[m] || '<span class="small">—（无此字段）</span>') + "</td>"; }).join("") + "</tr>";
        }).join("") +
        "</tbody></table></div>";
    }
    boxes.forEach(function (b) { b.addEventListener("change", function () {
      var lab = b.closest(".opt"); if (lab) lab.classList.toggle("is-on", b.checked);
      render();
    }); });
    render();
  }

  function refreshWidget(w) { /* reserved for theme-dependent re-render */ }

  /* ------------------------------------------------------------------- Boot */

  function boot() {
    collectPages();
    buildTOC();
    buildPagers();
    initSearch();
    initTheme();
    initCopyButtons();

    $$("[data-widget]").forEach(function (w) {
      var kind = w.dataset.widget;
      if (kind === "tabs") initTabs(w);
      else if (kind === "checklist") initChecklist(w);
      else if (kind === "tree") initTree(w);
      else if (kind === "fields") initFields(w);
      else if (kind === "frame") initFrame(w);
      else if (kind === "timeline") initTimeline(w);
      else if (kind === "compare") initCompare(w);
    });
    /* tabs placed outside widgets: bind once per parent container */
    var seenScopes = [];
    $$(".tabs").forEach(function (t) {
      if (t.closest("[data-widget]")) return;
      var scope = t.parentElement || document.body;
      if (seenScopes.indexOf(scope) !== -1) return;
      seenScopes.push(scope);
      initTabs(scope);
    });

    go(fromHash());
    window.addEventListener("hashchange", function () { go(fromHash()); });
    document.addEventListener("keydown", function (ev) {
      if (ev.target && /INPUT|TEXTAREA|SELECT/.test(ev.target.tagName)) return;
      if (ev.key === "ArrowRight") { go(current + 1); }
      else if (ev.key === "ArrowLeft") { go(current - 1); }
      else if (ev.key === "/") { ev.preventDefault(); if (window.innerWidth <= 900) openDrawer(); var s = $("#search"); if (s) s.focus(); }
    });
    var menu = $("#menu");
    if (menu) menu.addEventListener("click", openDrawer);
    var bd = $("#backdrop");
    if (bd) bd.addEventListener("click", closeDrawer);
    var printBtn = $("#print");
    if (printBtn) printBtn.addEventListener("click", function () { window.print(); });
  }

  if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", boot);
  else boot();
})();
