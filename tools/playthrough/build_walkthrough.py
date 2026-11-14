#!/usr/bin/env python3
"""Build the walkthrough as one self-contained HTML page.

usage: build_walkthrough.py OUT.html

Sources, all in docs/walkthrough/:
  NN_area.md        the area's text: "Arrival" (the lede), "Map notes" (bullets), "Story" (the numbered playthrough log
                    with its screenshots), "Party after", "Bugs" (the closing box)
  data/NN_area.json the panels: "sub", "title"/"tab" (optional), "map" {name, markers [[kind,x,y,label]]},
                    "items" [{kind,n,text,chip,chipText}], "wild" [[MAP_X, "land_mons"|"water_mons"|..., label]],
                    "trainers" [{name, hint, team [[species, lv, moves]]}]
  data/team.json    the team page
  img/              the screenshots (inlined as data: URIs)
and docs/playthrough_bugs.md for the bug page. Wild encounters come from src/data/wild_encounters.json, so the
tables are the game's own. Maps and sprites are drawn by assets.py from the game's data. STATUS below is the state
of the run at the last save: update it when a leg ends. The look is walkthrough_template.html."""
import base64, glob, html, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
WT = os.path.join(ROOT, "docs", "walkthrough")

STATUS = {
    "where": "Viridian City",
    "party": "Mankey L13, Pikachu L10, Pidgey L5, Caterpie L4",
    "badges": "0",
    "time": "1:59",
    "save": "Viridian City, after beating Blue",
}

TYPE_COL = {"Normal": "#9aa0a8", "Fire": "#e0642e", "Water": "#4f8fdf", "Electric": "#d9a21b", "Grass": "#5aa04a", "Ice": "#5fb8c4",
            "Fighting": "#c0442c", "Poison": "#a05fb0", "Ground": "#b8944a", "Flying": "#8f7fe0", "Psychic": "#e0508a",
            "Bug": "#91a530", "Rock": "#a8903a", "Ghost": "#6a5a9a", "Dragon": "#6a3fd0", "Dark": "#5a4a40", "Steel": "#8a9aaa",
            "Fairy": "#e88ab0"}
RATES = {"land_mons": [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1], "water_mons": [60, 30, 5, 4, 1],
         "rock_smash_mons": [60, 30, 5, 4, 1]}
RODS = [("Old Rod", [0, 1]), ("Good Rod", [2, 3, 4]), ("Super Rod", [5, 6, 7, 8, 9])]
FISH_RATES = [70, 30, 60, 20, 20, 40, 40, 15, 4, 1]

_assets = None


def assets():
    global _assets
    if _assets is None:
        import assets as a
        _assets = a
    return _assets


def esc(t):
    return html.escape(t, quote=True)


BLANK = []


def data_uri(rel):
    p = os.path.join(WT, rel)
    if os.path.exists(p) and rel not in BLANK:
        try:
            from PIL import Image, ImageStat
            if sum(ImageStat.Stat(Image.open(p).convert("RGB")).stddev) < 6 and "intro_stars" not in rel:
                BLANK.append(rel); print("WARNING: blank screenshot", rel)
        except Exception:
            pass
    return "data:image/png;base64," + base64.b64encode(open(p, "rb").read()).decode() if os.path.exists(p) else ""


def slug(name):
    return re.sub(r"^\d+_", "", name).replace("_", "-")


# ---------- markdown subset ----------

def inline(t):
    t = html.escape(t, quote=False)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)

    def im(m):
        src = data_uri(m.group(2))
        return (f'<figure class="shot"><img src="{src}" alt="{esc(m.group(1))}" loading="lazy">'
                f'<figcaption>{m.group(1)}</figcaption></figure>') if src else ""
    t = re.sub(r"!\[([^\]]*)\]\(([^)]+)\)", im, t)

    def ln(m):
        href = m.group(2)
        if ".md" in href:
            base = os.path.basename(href.split("#")[0])[:-3]
            href = "#bugs" if base == "playthrough_bugs" else "#" + slug(base)
        return f'<a href="{href}">{m.group(1)}</a>'
    t = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", ln, t)
    t = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", t)
    t = re.sub(r"(?<![\w*])\*([^*\n]+)\*(?![\w*])", r"<em>\1</em>", t)
    return t


LIST_RE = re.compile(r"^(\s*)(-|\d+\.)\s+(.*)$")
FIG_RE = re.compile(r'<figure class="shot">.*?</figure>', re.S)


def parse_list(lines, i, steps=False):
    base = len(LIST_RE.match(lines[i]).group(1))
    ordered = LIST_RE.match(lines[i]).group(2) != "-"
    tag = "ol" if ordered else "ul"
    out = []
    while i < len(lines):
        m = LIST_RE.match(lines[i])
        if not m or len(m.group(1)) != base:
            break
        text = [m.group(3)]
        i += 1
        sub = ""
        while i < len(lines):
            n = LIST_RE.match(lines[i])
            if n and len(n.group(1)) > base:
                s, i = parse_list(lines, i)
                sub += s
            elif lines[i].strip() and not n and len(lines[i]) - len(lines[i].lstrip()) > base:
                text.append(lines[i].strip())
                i += 1
            else:
                break
        body = inline(" ".join(text))
        if steps:
            figs = FIG_RE.findall(body)
            body = FIG_RE.sub("", body).strip()
            shots = f'<div class="shots">{"".join(figs)}</div>' if figs else ""
            out.append(f"<li><p>{body}</p>{sub}{shots}</li>")
        else:
            out.append(f"<li>{body}{sub}</li>")
    return f"<{tag}>" + "".join(out) + f"</{tag}>", i


def table(rows, cls=""):
    cells = [[c.strip() for c in r.strip().strip("|").split("|")] for r in rows]
    h = "".join(f"<th>{inline(c)}</th>" for c in cells[0])
    b = "".join("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in r) + "</tr>" for r in cells[2:])
    return f'<div class="tablewrap"><table class="{cls}"><thead><tr>{h}</tr></thead><tbody>{b}</tbody></table></div>'


def render(lines, steps=False):
    out, i = [], 0
    while i < len(lines):
        ln = lines[i]
        if not ln.strip():
            i += 1
        elif ln.startswith("### "):
            out.append(f"<h3>{inline(ln[4:])}</h3>"); i += 1
        elif ln.startswith("|"):
            j = i
            while j < len(lines) and lines[j].startswith("|"):
                j += 1
            out.append(table(lines[i:j])); i = j
        elif LIST_RE.match(ln):
            s, i = parse_list(lines, i, steps); out.append(s)
        else:
            j, para = i, []
            while j < len(lines) and lines[j].strip() and not lines[j].startswith(("#", "|")) and not LIST_RE.match(lines[j]):
                para.append(lines[j].strip()); j += 1
            out.append("<p>" + inline(" ".join(para)) + "</p>"); i = j
    return "\n".join(out)


def sections(text):
    """{title: lines}, with '' for the text between the title and the first '## '."""
    secs, cur = {"": []}, ""
    for ln in text.split("\n"):
        if ln.startswith("# "):
            continue
        if ln.startswith("## "):
            cur = ln[3:].strip(); secs[cur] = []
        else:
            secs[cur].append(ln)
    return secs


# ---------- game data ----------

def pretty(species):
    return species.replace("SPECIES_", "").replace("_", " ").title().replace("Mr ", "Mr. ")


def icon_img(species):
    try:
        a = assets()
        return f'<img class="mon" src="{a.b64(a.icon(species.replace("SPECIES_", "").lower()))}" alt="" width="32" height="32">'
    except Exception:
        return ""


def monname(species, extra=""):
    return f'<span class="monname">{pretty(species)}{icon_img(species)}</span>{extra}'


_enc = None


def encounters():
    global _enc
    if _enc is None:
        _enc = json.load(open(os.path.join(ROOT, "src", "data", "wild_encounters.json")))
    return _enc


def tally(mons, rates):
    agg = {}
    for slot, m in enumerate(mons):
        r = rates[slot] if slot < len(rates) else 0
        a = agg.setdefault(m["species"], [m["min_level"], m["max_level"], 0])
        a[0], a[1], a[2] = min(a[0], m["min_level"]), max(a[1], m["max_level"]), a[2] + r
    return sorted(agg.items(), key=lambda kv: -kv[1][2])


def lv(a):
    return str(a[0]) if a[0] == a[1] else f"{a[0]}–{a[1]}"


def rate_table(rows):
    body = "".join(f'<tr><td>{monname(s)}</td><td class="n">{lv(a)}</td><td class="n">{a[2]}%</td></tr>' for s, a in rows)
    return ('<div class="tablewrap"><table><thead><tr><th>Pokémon</th><th>Level</th><th>Rate</th></tr></thead>'
            f"<tbody>{body}</tbody></table></div>")


def wild_panels(specs):
    out = []
    for map_id, kind, label in specs:
        base = night = None
        for g in encounters()["wild_encounter_groups"]:
            for m in g["encounters"]:
                if m.get("map") == map_id:
                    if m.get("time") == "WILD_TIME_NIGHT":
                        night = m
                    elif "time" not in m:
                        base = m
        day = (base or {}).get(kind)
        ngt = (night or {}).get(kind) or day
        if not day:
            continue
        if kind == "fishing_mons":
            rows = []
            for rod, slots in RODS:
                tot = sum(FISH_RATES[s] for s in slots)
                for s in slots:
                    m = day["mons"][s]
                    rows.append(f'<tr><td>{monname(m["species"])}</td><td>{rod}</td><td class="n">{lv([m["min_level"], m["max_level"]])}</td>'
                                f'<td class="n">{round(100 * FISH_RATES[s] / tot)}%</td></tr>')
            body = ('<div class="tablewrap"><table><thead><tr><th>Pokémon</th><th>Rod</th><th>Level</th><th>Rate</th></tr></thead>'
                    f'<tbody>{"".join(rows)}</tbody></table></div>')
            out.append(f'<div class="panel"><h3>Wild Pokémon <span class="hint">{label}</span></h3>{body}</div>')
            continue
        rates = RATES[kind]
        d, n = tally(day["mons"], rates), tally(ngt["mons"], rates)
        if d == n:
            body = f'<div class="tod"><h4>Day and night</h4>{rate_table(d)}</div>'
        else:
            body = (f'<div class="todgrid"><div class="tod"><h4>Day · 6:00–17:00</h4>{rate_table(d)}</div>'
                    f'<div class="tod"><h4>Night · 17:00–6:00</h4>{rate_table(n)}</div></div>')
        out.append(f'<div class="panel"><h3>Wild Pokémon <span class="hint">{label}</span></h3>{body}</div>')
    return "".join(out)


def map_panel(spec):
    a = assets()
    im = a.map_img(spec["name"], [tuple(m) for m in spec["markers"]], crop=tuple(spec["crop"]) if spec.get("crop") else None)
    key = ('<p class="key"><span class="mk item">1</span> item ball <span class="mk gift">1</span> gift / event '
           '<span class="mk hidden">1</span> hidden item <span class="mk trainer">T1</span> trainer <span class="mk cut">C</span> tree that needs Cut</p>')
    return (f'<div class="panel wide mapbox"><h3>Map</h3><div class="mapscroll"><img class="map" src="{a.b64(im, quant=True)}" '
            f'alt="Map of {esc(spec["name"])} with numbered markers"></div>{key}</div>')


def legend(items):
    lis = "".join(
        f'<li><span class="mk {i["kind"]}">{i["n"]}</span><span>{i["text"]} '
        f'<span class="chip {i.get("chip", "no")}">{i.get("chipText", "")}</span></span></li>' for i in items)
    return f'<ol class="legend">{lis}</ol>'


def trainer_cards(trainers):
    cards = ""
    for k, t in enumerate(trainers, 1):
        rows = "".join(f'<tr><td>{monname("SPECIES_" + s.upper())}</td><td class="n">{l}</td><td>{m}</td></tr>' for s, l, m in t["team"])
        cards += (f'<div class="trainer-card"><h4><span class="mk trainer">T{k}</span> {esc(t["name"])}</h4><p class="hint">{t.get("hint", "")}</p>'
                  f'<div class="tablewrap"><table><thead><tr><th>Pokémon</th><th>Lv</th><th>Moves / notes</th></tr></thead><tbody>{rows}</tbody></table></div></div>')
    return f'<div class="panel wide"><h3>Trainers</h3><div class="trainers">{cards}</div></div>'


# ---------- pages ----------

def load_pages():
    pages = []
    for md in sorted(glob.glob(os.path.join(WT, "[0-9][0-9]_*.md"))):
        name = os.path.basename(md)[:-3]
        data_p = os.path.join(WT, "data", name + ".json")
        data = json.load(open(data_p, encoding="utf-8")) if os.path.exists(data_p) else {}
        text = open(md, encoding="utf-8").read()
        title = re.match(r"# (?:\d+ )?(.*)", text).group(1)
        pages.append({"id": "intro" if name == "00_start" else slug(name), "title": data.get("title", title),
                      "tab": data.get("tab", data.get("title", title)), "secs": sections(text), "data": data, "kind": "area"})
    return pages


def area_html(p, n):
    s, d = p["secs"], p["data"]
    sign = (f'<header class="sign"><span class="num">{n}</span><div><h2>{esc(p["title"])}</h2>'
            f'<p class="sub">{esc(d.get("sub", ""))}</p></div></header>')
    lede = f'<div class="lede">{render(s[""])}</div>' if any(l.strip() for l in s[""]) else ""
    ref = ""
    if d.get("map"):
        ref += map_panel(d["map"])
    panel = ""
    if d.get("items"):
        panel += legend(d["items"])
    if any(l.strip() for l in s.get("Map notes", [])):
        panel += render(s["Map notes"]).replace("<ul>", '<ul class="notes-list">', 1)
    if panel:
        ref += f'<div class="panel"><h3>Items &amp; events</h3>{panel}</div>'
    if d.get("wild"):
        ref += wild_panels(d["wild"])
    if d.get("trainers"):
        ref += trainer_cards(d["trainers"])
    ref = f'<div class="ref">{ref}</div>' if ref else ""
    log = ""
    if s.get("Story"):
        log = ('<aside class="run" aria-label="From our playthrough"><h3><span class="runtag">Playthrough log</span> '
               f'What happened in our run as Ethan</h3>{render(s["Story"], steps=True).replace("<ol>", "<ol class=\"moves\">", 1)}</aside>')
    end = ""
    box = ""
    for label in ("Party after", "Bugs"):
        if any(l.strip() for l in s.get(label, [])):
            box += f"<h3>{label}</h3>{render(s[label])}"
    if box:
        end = f'<div class="notes"><h2>Where we left off</h2>{box}</div>'
    return f'<section class="step" id="s-{p["id"]}">{sign}{lede}{ref}{log}{end}</section>'


def stat_row(name, val, arrow):
    cls = " up" if arrow > 0 else " down" if arrow < 0 else ""
    mark = " ▲" if arrow > 0 else " ▼" if arrow < 0 else ""
    return (f'<div class="stat{cls}" title="{name}: {val}"><span class="sl">{name}{mark}</span><span class="track">'
            f'<span class="fill" style="width:{min(100, val / 60 * 100):.1f}%"></span></span><span class="sv">{val}</span></div>')


def type_chip(t, small=False):
    return f'<span class="type{" sm" if small else ""}" style="--tc:{TYPE_COL.get(t, "#777")}">{t}</span>'


def team_html():
    d = json.load(open(os.path.join(WT, "data", "team.json"), encoding="utf-8"))
    a = assets()
    cards = ""
    for m in d["mons"]:
        try:
            spr = f'<img class="mon-sprite" src="{a.b64(a.front(m["species"]))}" alt="{m["name"]} sprite" width="96" height="96">'
        except Exception:
            spr = ""
        stats = "".join(stat_row(n, v, ar) for n, v, ar in m["stats"])
        moves = "".join(f'<tr><td>{mv}</td><td>{type_chip(t, True)}</td><td class="n">{pw}</td><td class="n">{ac}</td><td class="n">{pp}</td></tr>'
                        for mv, t, pw, ac, pp in m["moves"])
        cards += (f'<article class="mon-card"><div class="mon-head">{spr}<div class="mon-id"><h3>{m["name"]}</h3>'
                  f'<p class="lv">Lv {m["lv"]} · {m["exp"]:,} exp.</p><div class="types">{"".join(type_chip(t) for t in m["types"])}</div></div></div>'
                  f'<dl class="mon-meta"><dt>Nature</dt><dd>{m["nature"]}</dd><dt>Ability</dt><dd>{m["ability"]}</dd><dt>Met</dt><dd>{m["met"]}</dd></dl>'
                  f'<p class="role">{m["role"]}</p><div class="stats" role="list" aria-label="{m["name"]} stats">{stats}</div>'
                  f'<div class="tablewrap"><table class="moves-t"><thead><tr><th>Move</th><th>Type</th><th>Pow</th><th>Acc</th><th>PP</th></tr></thead>'
                  f'<tbody>{moves}</tbody></table></div></article>')
    return ('<section class="team" id="s-team" aria-labelledby="team-h"><div class="team-intro"><h2 id="team-h">The team</h2>'
            '<p>The party as last saved, in party order. Stats are the in-game values; bars share one scale (0–60).</p></div>'
            f'<div class="team-grid">{cards}</div></section>')


def bugs_html():
    text = open(os.path.join(ROOT, "docs", "playthrough_bugs.md"), encoding="utf-8").read().replace("(walkthrough/", "(")
    head, _, det = text.partition("## Details")
    rows = [l for l in head.split("\n") if l.startswith("|")]
    tbl = table(rows) if len(rows) > 2 else "<p>No bugs found yet.</p>"
    details = ""
    for blk in re.split(r"\n(?=### )", det.strip()):
        if blk.startswith("### "):
            t, _, rest = blk.partition("\n")
            details += f'<div class="panel wide"><h3>{inline(t[4:])}</h3>{render(rest.split(chr(10)))}</div>'
    sign = ('<header class="sign"><span class="num">!</span><div><h2>Bugs found</h2>'
            '<p class="sub">Soft locks · scripts · interactions · graphics</p></div></header>')
    return (f'<section class="step" id="s-bugs">{sign}<div class="ref"><div class="panel wide bugs"><h3>Bug log</h3>{tbl}</div>'
            f'{details}</div></section>')


def main(out):
    pages = load_pages()
    allp = [(p["id"], p["tab"], area_html(p, i + 1)) for i, p in enumerate(pages)]
    allp.append(("team", "Team", team_html()))
    allp.append(("bugs", "Bugs", bugs_html()))
    tabs, body = "", ""
    for i, (pid, tab, h) in enumerate(allp):
        num = "!" if pid == "bugs" else str(i + 1)
        tabs += f'<a class="tab" href="#{pid}" data-page="{pid}"><span class="tn">{num}</span>{esc(tab)}</a>'
        prev = f'<a class="pg prev" href="#{allp[i - 1][0]}">← {esc(allp[i - 1][1])}</a>' if i else "<span></span>"
        nxt = f'<a class="pg next" href="#{allp[i + 1][0]}">{esc(allp[i + 1][1])} →</a>' if i + 1 < len(allp) else "<span></span>"
        body += (f'<div class="page" id="p-{pid}" data-page="{pid}" data-title="{esc(tab)}" hidden>{h}'
                 f'<nav class="pager" aria-label="Page navigation">{prev}{nxt}</nav></div>')
    s = STATUS
    a = assets()
    try:
        pika = f'<span class="monname">Pikachu<img class="mon" src="{a.b64(a.icon("pikachu"))}" alt="" width="32" height="32"></span>'
    except Exception:
        pika = "Pikachu"
    facts = (f'<div class="fact"><h3>Player · rival</h3><p class="big">ETHAN · BLUE</p></div>'
             f'<div class="fact"><h3>First Pokémon</h3><p class="big">{pika}</p><p class="mono">gift from Prof. Oak, L5</p></div>'
             f'<div class="fact"><h3>Team so far</h3><p class="big">{esc(s["party"])}</p></div>'
             f'<div class="fact"><h3>Badges</h3><p class="big">{esc(s["badges"])} of 8</p><p class="mono">{esc(s["time"])} play time · saved in {esc(s["save"])}</p></div>')
    lede = ("A step-by-step walkthrough of this Emerald-based ROM, played as Ethan from the new game screen to the end of the story, "
            "through Kanto, Hoenn, the Sevii Islands and Mega Island, and checked for bugs on the way. Each area opens with its map, "
            "items, wild Pokémon (by day and by night) and trainers; the boxed playthrough log shows what happened in our run, with its screenshots.")
    t = open(os.path.join(HERE, "walkthrough_template.html"), encoding="utf-8").read()
    t = (t.replace("%%H1%%", f"Ethan’s road through <em>Kanto and Hoenn</em>").replace("%%LEDE%%", lede)
          .replace("%%FACTS%%", facts).replace("%%TABS%%", tabs).replace("%%PAGES%%", body))
    open(out, "w", encoding="utf-8").write(t)
    print(out, len(allp), "pages", os.path.getsize(out) // 1024, "KB")


if __name__ == "__main__":
    main(sys.argv[1])
