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

PAGES = []
STATUS = {
    "where": "Two Island, Joyful Game Corner",
    "party": "Dragonair L47, Gengar L48, Venusaur L49, Charizard L51, Snorlax L55, Blastoise L47",
    "badges": "7",
    "time": "≈214h",
    "save": "Two Island (ethan_two_island)",
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
    mime = "image/jpeg" if rel.lower().endswith((".jpg", ".jpeg")) else "image/png"
    return f"data:{mime};base64," + base64.b64encode(open(p, "rb").read()).decode() if os.path.exists(p) else ""


def slug(name):
    return re.sub(r"^\d+[a-z]?_", "", name).replace("_", "-")


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
            tgt = next((q for q in PAGES if q["id"] == href[1:]), None)
            if tgt and re.fullmatch(r"page [\d.]+", m.group(1)):   # "[page 24](24_route_12.md)": page numbers move, names don't
                return f'<a href="{href}">{tgt["tab"]}</a>'
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


FLOOR_TOK = r"(?:B?\d+F|roof room|roofroom|roof|elevator)"


def ALL_KEYS(specs):
    return {(m[0], str(m[3])) for sp in specs for m in sp["markers"]}


def nkey(s):
    """A comparable key: "Roof room" -> ROOFROOM, "Route 15" -> ROUTE15."""
    return re.sub(r"[^A-Z0-9]", "", s.upper())


def floor_key(name):
    """The part of a map's name that tells it from the page's other maps: MtMoon_B2F -> B2F, SafariZone_East -> EAST, Route15 -> ROUTE15."""
    return nkey(name.split("_")[-1])


def floors_in(text):
    """The floors a note or event names: "B2F", "1F", "the roof", "roof room"."""
    found = {m.upper() for m in re.findall(r"\b(B?\d+F)\b", text)}
    low = text.lower()
    if "roof room" in low or "roofroom" in low:
        found.add("ROOFROOM")
        low = low.replace("roof room", "").replace("roofroom", "")
    if re.search(r"\broof\b", low):
        found.add("ROOF")
    if "elevator" in low:
        found.add("ELEVATOR")
    return found


def lead_key(seg, keys):
    """The floor or area a note opens with ("**1F:** ...", "1F (46x38) ...", "the **roof** ...", "**CENTER** (...)"), if it is one of keys."""
    s = re.sub(r"^(?:-\s+)?(?:the\s+(?=\*\*))?", "", seg.strip(), flags=re.I)
    m = re.match(r"\*\*([^*]+)\*\*", s)
    if m:
        lab = m.group(1).strip(" :,")
        k = nkey(lab)
        if k in keys:
            return k
        f = re.match(FLOOR_TOK + r"\b", lab, re.I)
        return nkey(f.group(0)) if f and nkey(f.group(0)) in keys else None
    f = re.match(FLOOR_TOK + r"\b", s, re.I)
    return nkey(f.group(0)) if f and nkey(f.group(0)) in keys else None


def note_bullets(lines):
    """Split a Map notes section into top-level bullets (each with its indented continuation) and the other lines."""
    bullets, other = [], []
    for ln in lines:
        if ln.startswith("- "):
            bullets.append([ln])
        elif bullets and ln.startswith((" ", "\t")) and ln.strip():
            bullets[-1].append(ln)
        elif ln.strip():
            other.append(ln)
    return ["\n".join(b) for b in bullets], other


def place_notes(lines, keys):
    """Hand each note to the floor (or area) it is about. A note that opens with a floor goes there; a note that walks through the floors
    ("**1F:** ...; 2F ...; the **roof** ...", "**1F** ... **2F MARKET:** ...") is cut at each floor; a note that only names one floor goes
    there; anything else is about the whole place. Returns ({key: [lines]}, [general lines])."""
    per, general = {}, []
    bullets, other = note_bullets(lines)
    general += other
    cut = re.compile(r"(?:;\s+|(?<=[.!?\"”)])\s+)(?=(?:the\s+)?(?:\*\*)?" + FLOOR_TOK + r"\b)", re.I)
    for b in bullets:
        body = b[2:]
        parts = [x for x in cut.split(body) if x.strip()]
        if len(parts) > 1:
            segs, head = [], []
            for x in parts:
                k = lead_key(x, keys)
                if k:
                    segs.append([k, x])
                elif segs:
                    segs[-1][1] += "; " + x
                else:
                    head.append(x)
            fm = re.search(r"\b" + FLOOR_TOK + r"\b", body, re.I)
            pre = body[:fm.start()].strip().rstrip("*").rstrip() if fm else ""
            if len(segs) > 1 and not pre.endswith(":"):   # a note that walks the floors, not a list under a heading ("Pads used: 3F ...")
                if " ".join(head).strip(" .:;,"):
                    general.append("- " + "; ".join(head))
                for k, x in segs:
                    x = x.strip().rstrip(";")
                    per.setdefault(k, []).append("- " + x[0].upper() + x[1:])
                continue
        k = lead_key(body, keys)
        # "**EAST** (34.64), **NORTH** (34.65), **WEST**: ..." names several areas at once: the whole place
        if k and not any(nkey(m) in keys - {k} for m in re.findall(r"\*\*([^*]+)\*\*", body[:60])):
            per.setdefault(k, []).append(b)
            continue
        named = floors_in(b[:50]) & keys if floors_in(b) & keys == floors_in(b[:50]) & keys else set()   # one floor, named up front
        if len(named) == 1:
            per.setdefault(named.pop(), []).append(b)
        else:
            general.append(b)
    return per, general


def map_title(name):
    """PokemonTower_3F -> Pokemon Tower 3F (a floor's heading on multi-map pages)."""
    parts = [re.sub(r"(?<=[a-z])(?=[A-Z0-9])|(?<=[A-Z])(?=[A-Z][a-z])", " ", x) for x in name.split("_")]
    return " ".join(parts).replace("Pokemon", "Pokémon")


def map_panel(spec, title="Map"):
    a = assets()
    im = a.map_img(spec["name"], [tuple(m) for m in spec["markers"]], crop=tuple(spec["crop"]) if spec.get("crop") else None)
    key = ('<p class="key"><span class="mk item">1</span> item ball <span class="mk gift">1</span> gift / event '
           '<span class="mk hidden">1</span> hidden item <span class="mk trainer">T1</span> trainer <span class="mk cut">C</span> tree that needs Cut</p>')
    return (f'<div class="panel wide mapbox"><h3>{esc(title)}</h3><div class="mapscroll"><img class="map" src="{a.b64(im, quant=True)}" '
            f'alt="Map of {esc(spec["name"])} with numbered markers"></div>{key}</div>')


def legend(items):
    lis = "".join(
        f'<li><span class="mk {i["kind"]}">{i["n"]}</span><span>{i["text"]} '
        f'<span class="chip {i.get("chip", "no")}">{i.get("chipText", "")}</span></span></li>' for i in items)
    return f'<ol class="legend">{lis}</ol>'


def trainer_cards(trainers, title="Trainers", numbered=None):
    """Trainer cards; numbered = [(k, trainer)] keeps the page-wide T-numbers used on the maps (floor subsections)."""
    cards = ""
    for k, t in (numbered if numbered is not None else enumerate(trainers, 1)):
        rows = "".join(f'<tr><td>{monname("SPECIES_" + s.upper())}</td><td class="n">{l}</td><td>{m}</td></tr>' for s, l, m in t["team"])
        cards += (f'<div class="trainer-card"><h4><span class="mk trainer">T{k}</span> {esc(t["name"])}</h4><p class="hint">{t.get("hint", "")}</p>'
                  f'<div class="tablewrap"><table><thead><tr><th>Pokémon</th><th>Lv</th><th>Moves / notes</th></tr></thead><tbody>{rows}</tbody></table></div></div>')
    return f'<div class="panel wide"><h3>{title}</h3><div class="trainers">{cards}</div></div>'


# ---------- pages ----------

def load_pages():
    pages = []
    for md in sorted(m for m in glob.glob(os.path.join(WT, "[0-9][0-9]*_*.md")) if re.match(r"\d\d[a-z]?_", os.path.basename(m))):
        name = os.path.basename(md)[:-3]
        data_p = os.path.join(WT, "data", name + ".json")
        data = json.load(open(data_p, encoding="utf-8")) if os.path.exists(data_p) else {}
        text = open(md, encoding="utf-8").read()
        title = re.match(r"# (?:[\d.]+ )?(.*)", text).group(1)
        parent = slug(name.split("--")[0]) if "--" in name else None   # NN_city--place.md: a place inside NN_city
        pages.append({"id": "intro" if name == "00_start" else slug(name), "title": data.get("title", title),
                      "tab": data.get("tab", data.get("title", title)), "secs": sections(text), "data": data, "kind": "area",
                      "parent": parent, "order": [tuple(int(v) for v in x.split(".")) for x in re.findall(r"# ([\d.]+) ", text)[:1]] or [(99,)]})
    # a place page keeps the number in its title (33.1, 33.2, ...): order the children by it
    return sorted(pages, key=lambda p: (0, 0) if not p["parent"] else (1, p["order"][0]))


def caught_panel(d):
    """The Pokémon of this area that are in the party or the PC (met location stored in each Pokémon), and the wild species still to catch."""
    if "caught" not in d:
        return ""
    got, miss = d["caught"], d.get("missing", [])
    if got:
        rows = "".join(f'<tr><td>{monname("SPECIES_" + c["species"].upper())}</td><td class="n">{c["met"]}</td>'
                       f'<td>{"in the party" if c["where"] == "party" else "in the PC (" + c["where"] + ")"}{" · now Lv " + str(c["level"]) if c.get("level") else ""}</td>'
                       f'<td>{esc(c["how"])}</td></tr>' for c in got)
        body = ('<div class="tablewrap"><table><thead><tr><th>Pokémon</th><th>Met at Lv</th><th>Now</th><th>How</th></tr></thead>'
                f'<tbody>{rows}</tbody></table></div>')
    else:
        body = '<p class="hint">Nothing caught here yet.</p>'
    walk = [m for m in miss if "land" in m["via"]]
    rod = [m for m in miss if "land" not in m["via"]]
    def chips(lst, note=""):
        return " ".join(f'<span class="chip no">{monname("SPECIES_" + m["species"].upper())}{note}</span>' for m in lst)
    if walk:
        body += f'<h4>Still to catch (walking)</h4><p>{chips(walk)}</p>'
    if rod:
        body += f'<h4>Still to catch (surf or a rod)</h4><p>{chips(rod)}</p>'
    if not walk and not rod and got:
        body += '<p class="hint">Every species of this area\'s wild tables is in the dex.</p>'
    return f'<div class="panel wide"><h3>Pokémon caught here</h3>{body}</div>'


KMAP = {}
KMAP_CSS_DONE = []


def kmap_html(spec, pages, current=None, compact=False):
    """The illustrated Kanto map: every label is a link to the page of the place when the run has one (dimmed otherwise); the labels of the
    current page are marked "you are here". The picture is embedded once, as a CSS background of .kmap-img, and shared by every page."""
    w, h = spec["size"]
    ids = {pg["id"]: pg["tab"] for pg in pages}
    hs = ""
    for sp in spec["spots"]:
        x, y, bw, bh = sp["box"]
        pos = f'left:{x / w * 100:.2f}%;top:{y / h * 100:.2f}%;width:{bw / w * 100:.2f}%;height:{bh / h * 100:.2f}%'
        if sp.get("page") in ids:
            here = " here" if sp["page"] == current else ""
            hs += f'<a class="hs ok{here}" style="{pos}" href="#{sp["page"]}" title="{esc(sp["label"])}: {esc(ids[sp["page"]])}"><span class="sr">{esc(sp["label"])}</span></a>'
        else:
            hs += f'<span class="hs no" style="{pos}" title="{esc(sp["label"])}: not visited yet"><span class="sr">{esc(sp["label"])}</span></span>'
    css = ""
    if not KMAP_CSS_DONE:
        KMAP_CSS_DONE.append(1)
        css = f'<style>.kmap-img{{background-image:url({data_uri(spec["image"])})}}</style>'
    hint = ("Click a label to open that place's page. Outlined labels have a page, the gold ones are this page, dimmed ones are still ahead. (Map art: psypoke.org.)"
            if not compact else "The Kanto map: gold = this page, outlined = other pages, dimmed = still ahead.")
    return (f'{css}<div class="panel wide kmap{" compact" if compact else ""}"><h3>The Kanto map</h3><p class="hint">{hint}</p><div class="kmap-wrap">'
            f'<div class="kmap-img" role="img" aria-label="Map of Kanto with the towns and routes" style="aspect-ratio:{w}/{h}"></div>{hs}</div></div>')


def places_html(p):
    """The city page's list of its places (houses, shops, gym, dungeons), each a link to its own page."""
    kids = [q for q in PAGES if q.get("parent") == p["id"]]
    if not kids:
        return ""
    li = "".join(f'<li><a href="#{q["id"]}"><b>{esc(q["tab"])}</b></a>{" · " + esc(q["data"].get("sub", "")) if q["data"].get("sub") else ""}</li>'
                 for q in kids)
    return (f'<div class="panel wide places"><h3>Places in {esc(p["tab"])}</h3><p class="hint">Every building, the gym and the dungeons '
            f'have their own page (map, people, items, trainers and our playthrough log there).</p><ul class="notes-list">{li}</ul></div>')


def area_html(p, n):
    s, d = p["secs"], p["data"]
    sign = (f'<header class="sign"><span class="num{" sub" if "." in str(n) else ""}">{n}</span><div><h2>{esc(p["title"])}</h2>'
            f'<p class="sub">{esc(d.get("sub", ""))}</p></div></header>')
    lede = f'<div class="lede">{render(s[""])}</div>' if any(l.strip() for l in s[""]) else ""
    if p.get("parent"):
        par = next((q for q in PAGES if q["id"] == p["parent"]), None)
        if par:
            lede = f'<p class="crumb"><a href="#{par["id"]}">← {esc(par["tab"])}</a> · a place in {esc(par["tab"])}</p>' + lede
    if KMAP and not p.get("parent"):
        lede = kmap_html(KMAP, PAGES, current=p["id"], compact=p["id"] != "intro") + lede
    lede += places_html(p)
    ref = ""
    specs = d["map"] if isinstance(d.get("map"), list) else [d["map"]] if d.get("map") else []
    items = d.get("items") or []
    notes = s.get("Map notes", [])
    left_trainers = []
    if len(specs) > 1:
        # dungeons and buildings with several floors: each floor's map is followed by its own items, events and notes
        fkeys = {floor_key(sp["name"]) for sp in specs}
        if len(fkeys) < len(specs):
            fkeys = set()          # two maps share a name ending (rooms of a ship): no guessing
        per_note, notes = place_notes(notes, fkeys)
        if notes:                  # notes about the whole place come first, above the floor maps
            ref += f'<div class="panel wide"><h3>Notes for the whole place</h3>{render(notes).replace("<ul>", '<ul class="notes-list">', 1)}</div>'
            notes = []
        placed = set()
        trs = list(enumerate(d.get("trainers") or [], 1))
        wild = list(d.get("wild") or [])
        for spec in specs:
            fk = floor_key(spec["name"]) if fkeys else None
            keys = {(m[0], str(m[3])) for m in spec["markers"]}
            here = [i for i in items if (i["kind"], str(i["n"])) in keys
                    or (fk and id(i) not in placed and (i["kind"], str(i["n"])) not in ALL_KEYS(specs) and floors_in(i["text"]) & fkeys == {fk})]
            placed |= {id(i) for i in here}
            mine = [dict(i, text=i["text"].replace(f' ({spec["name"]})', "")) for i in here]
            ttl = esc(map_title(spec["name"]))
            ref += f'<h3 class="floor-head">{ttl}</h3>'
            ref += map_panel(spec, map_title(spec["name"]))
            body = legend(mine) if mine else ""
            if per_note.get(fk):
                body += render(per_note[fk]).replace("<ul>", '<ul class="notes-list">', 1)
            body = body or '<p class="note">No items or events on this floor.</p>'
            ref += f'<div class="panel wide floor-items"><h3>Items, events &amp; notes · {ttl}</h3>{body}</div>'
            # the floor's wild Pokémon and trainers (a card's hint starts with its map's name on multi-map pages)
            mw = [w for w in wild if re.sub(r"[^A-Z0-9]", "", w[0][4:]) == re.sub(r"[^A-Z0-9]", "", spec["name"].upper())]
            wild = [w for w in wild if w not in mw]
            if mw:
                ref += wild_panels([[w[0], w[1], w[2].replace(f" ({spec['name']})", "")] for w in mw])
            mt = [(k, tr) for k, tr in trs if re.match(re.escape(spec["name"]) + r"[ .]", tr.get("hint", ""))]
            trs = [kt for kt in trs if kt not in mt]
            if mt:
                ref += trainer_cards(None, f"Trainers · {ttl}", [(k, dict(tr, hint=tr["hint"][len(spec["name"]) + 1:].lstrip())) for k, tr in mt])
        items = [i for i in items if id(i) not in placed]
        d = dict(d, wild=wild, trainers=None)
        left_trainers = trs
    else:
        for spec in specs:
            ref += map_panel(spec)
    panel = ""
    if items:
        panel += legend(items)
    if any(l.strip() for l in notes):
        panel += render(notes).replace("<ul>", '<ul class="notes-list">', 1)
    if panel:
        ref += f'<div class="panel"><h3>{"Other events" if len(specs) > 1 else "Items &amp; events"}</h3>{panel}</div>'
    if d.get("wild"):
        ref += wild_panels(d["wild"])
    ref += caught_panel(d)
    if d.get("trainers"):
        ref += trainer_cards(d["trainers"])
    elif len(specs) > 1 and left_trainers:
        ref += trainer_cards(None, "Other trainers", left_trainers)
    if any(l.strip() for l in s.get("Everyone and every building", [])):
        ref += f'<div class="panel wide"><h3>People and buildings</h3>{render(s["Everyone and every building"])}</div>'
    ref = f'<div class="ref">{ref}</div>' if ref else ""
    log = ""
    # every other section of the page text (return visits, side trips, one-off lessons) is part of "what happened":
    # shown after the Story under its own heading. Items / Pokémon lists are drawn from game data in the panels above instead.
    shown = {"", "Map notes", "Story", "Everyone and every building", "Party after", "Bugs", "Items", "Pokémon", "Pokémon / trainers"}
    extra = [(t, ls) for t, ls in s.items() if t not in shown and not t.startswith("Map notes") and any(l.strip() for l in ls)]
    if s.get("Story") or extra:
        body = render(s["Story"], steps=True).replace("<ol>", "<ol class=\"moves\">", 1) if s.get("Story") else ""
        for t, ls in extra:
            body += f"<h4 class=\"later\">{inline(t)}</h4>" + render(ls, steps=True).replace("<ol>", "<ol class=\"moves\">").replace("<ul>", "<ul class=\"moves\">")
        log = ('<aside class="run" aria-label="From our playthrough"><h3><span class="runtag">Playthrough log</span> '
               f'What happened in our run as Ethan</h3>{body}</aside>')
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
    pc = ""
    try:
        cj = json.load(open(os.path.join(WT, "data", "caught.json"), encoding="utf-8"))
        box = [m for m in cj["mons"] if m["where"] != "party"]
        boxes = {}
        for m in box:
            boxes.setdefault(m["where"], []).append(m)
        def pcname(b):
            return int(b.split()[-1]) if b.split()[-1].isdigit() else 99
        secs = ""
        for b in sorted(boxes, key=pcname):
            chips = " ".join(f'<span class="chip no">{monname("SPECIES_" + m["species"].upper())}'
                             f' met at Lv {m["metlevel"]}, {m["mapsec"].replace("MAPSEC_", "").replace("_", " ").title()}</span>' for m in boxes[b])
            secs += f'<h4>{b.title()} ({len(boxes[b])})</h4><p>{chips}</p>'
        pc = (f'<div class="panel wide"><h3>In the PC ({len(box)} Pokémon)</h3>{secs}'
              f'<p class="hint">{len(cj["dex_owned"])} species owned in the Pokédex.</p></div>')
    except Exception:
        pass
    return ('<section class="team" id="s-team" aria-labelledby="team-h"><div class="team-intro"><h2 id="team-h">The team</h2>'
            '<p>The party as last saved, in party order. Stats are the in-game values; bars share one scale (0–60).</p></div>'
            f'<div class="team-grid">{cards}</div>{pc}</section>')


def party_text(s):
    """The party line comes from team.json (gen_team.py), falling back to STATUS."""
    try:
        d = json.load(open(os.path.join(WT, "data", "team.json"), encoding="utf-8"))
        return ", ".join(f'{m["name"]} L{m["lv"]}' for m in d["mons"])
    except Exception:
        return s["party"]


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
    global PAGES
    PAGES = pages
    KMAP.update(next((pg["data"]["kmap"] for pg in pages if pg["data"].get("kmap")), {}))
    top = [p for p in pages if not p.get("parent")]
    num = {p["id"]: str(i + 1) for i, p in enumerate(top)}
    for p in pages:
        if p.get("parent"):
            sib = [q for q in pages if q.get("parent") == p["parent"]]
            num[p["id"]] = f'{num.get(p["parent"], "?")}.{sib.index(p) + 1}'
    allp = [(p["id"], p["tab"], area_html(p, num[p["id"]])) for p in top]
    allp.append(("team", "Team", team_html()))
    allp.append(("bugs", "Bugs", bugs_html()))
    kids = ""
    for p in pages:
        if not p.get("parent"):
            continue
        sib = [q for q in pages if q.get("parent") == p["parent"]]
        k = sib.index(p); par = next(q for q in pages if q["id"] == p["parent"])
        prev = f'<a class="pg prev" href="#{sib[k - 1]["id"]}">← {esc(sib[k - 1]["tab"])}</a>' if k else f'<a class="pg prev" href="#{par["id"]}">← {esc(par["tab"])}</a>'
        nxt = f'<a class="pg next" href="#{sib[k + 1]["id"]}">{esc(sib[k + 1]["tab"])} →</a>' if k + 1 < len(sib) else f'<a class="pg next" href="#{par["id"]}">{esc(par["tab"])} ↩</a>'
        kids += (f'<div class="page" id="p-{p["id"]}" data-page="{p["id"]}" data-parent="{p["parent"]}" data-title="{esc(p["tab"])}" hidden>'
                 f'{area_html(p, num[p["id"]])}<nav class="pager" aria-label="Page navigation">{prev}{nxt}</nav></div>')
    tabs, body = "", ""
    for i, (pid, tab, h) in enumerate(allp):
        num = "!" if pid == "bugs" else str(i + 1)
        tabs += f'<a class="tab" href="#{pid}" data-page="{pid}"><span class="tn">{num}</span>{esc(tab)}</a>'
        prev = f'<a class="pg prev" href="#{allp[i - 1][0]}">← {esc(allp[i - 1][1])}</a>' if i else "<span></span>"
        nxt = f'<a class="pg next" href="#{allp[i + 1][0]}">{esc(allp[i + 1][1])} →</a>' if i + 1 < len(allp) else "<span></span>"
        body += (f'<div class="page" id="p-{pid}" data-page="{pid}" data-title="{esc(tab)}" hidden>{h}'
                 f'<nav class="pager" aria-label="Page navigation">{prev}{nxt}</nav></div>')
    body += kids
    s = STATUS
    a = assets()
    try:
        pika = f'<span class="monname">Pikachu<img class="mon" src="{a.b64(a.icon("pikachu"))}" alt="" width="32" height="32"></span>'
    except Exception:
        pika = "Pikachu"
    facts = (f'<div class="fact"><h3>Player · rival</h3><p class="big">ETHAN · BLUE</p></div>'
             f'<div class="fact"><h3>First Pokémon</h3><p class="big">{pika}</p><p class="mono">gift from Prof. Oak, L5</p></div>'
             f'<div class="fact"><h3>Team so far</h3><p class="big">{esc(party_text(s))}</p></div>'
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
