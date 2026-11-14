#!/usr/bin/env python3
"""Build the walkthrough as one self-contained HTML page from docs/walkthrough/*.md, docs/playthrough_bugs.md
and docs/walkthrough/img (inlined as data: URIs).

usage: build_walkthrough.py OUT.html

The pages are the numbered files NN_name.md, in order, plus an overview and the bug log. To change the look edit
TEMPLATE below; to change the content edit the markdown. The status block of the overview is STATUS."""
import base64, glob, html, os, re, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
WT = os.path.join(ROOT, "docs", "walkthrough")

# The state of the run at the last save: update it when a leg ends.
STATUS = {
    "where": "Viridian City",
    "party": [("PIKACHU", "L6", "Thundershock, Growl, Tail Whip")],
    "money": "¥3000",
    "badges": "0 / 8",
    "dex": "1",
    "time": "0:11",
    "save": "ethan_viridian_tutorial.sav",
    "bag": "5 Poké Balls, 2 Potions, Teachy TV",
}


def img_uri(rel):
    p = os.path.join(WT, rel)
    if not os.path.exists(p):
        return ""
    return "data:image/png;base64," + base64.b64encode(open(p, "rb").read()).decode()


def slug(name):
    return re.sub(r"^\d+_", "", name).replace("_", "-")


def inline(t):
    t = html.escape(t, quote=False)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)

    def im(m):
        src = img_uri(m.group(2))
        return (f'<figure><img src="{src}" alt="{m.group(1)}" loading="lazy"><figcaption>{m.group(1)}</figcaption></figure>'
                if src else "")
    t = re.sub(r"!\[([^\]]*)\]\(([^)]+)\)", im, t)

    def ln(m):
        href = m.group(2)
        if href.endswith(".md") or ".md#" in href:
            base = os.path.basename(href.split("#")[0])[:-3]
            href = "#bugs" if base == "playthrough_bugs" else "#" + slug(base)
        return f'<a href="{href}">{m.group(1)}</a>'
    t = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", ln, t)
    t = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", t)
    t = re.sub(r"(?<![\w*])\*([^*\n]+)\*(?![\w*])", r"<em>\1</em>", t)
    return t


LIST_RE = re.compile(r"^(\s*)(-|\d+\.)\s+(.*)$")


def parse_list(lines, i):
    base = len(LIST_RE.match(lines[i]).group(1))
    ordered = LIST_RE.match(lines[i]).group(2) != "-"
    out = ["<ol>" if ordered else "<ul>"]
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
        out.append("<li>" + inline(" ".join(text)) + sub + "</li>")
    out.append("</ol>" if ordered else "</ul>")
    return "".join(out), i


def table(rows):
    cells = [[c.strip() for c in r.strip().strip("|").split("|")] for r in rows]
    head, body = cells[0], [r for r in cells[2:]]
    h = "".join(f"<th>{inline(c)}</th>" for c in head)
    b = "".join("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in r) + "</tr>" for r in body)
    return f'<div class="tbl"><table><thead><tr>{h}</tr></thead><tbody>{b}</tbody></table></div>'


def md_sections(text):
    """[(title, html)] split at '## ' headings; the part before the first one has title None. '# ' is dropped."""
    lines = text.split("\n")
    secs, cur, buf = [], None, []

    def flush():
        if buf or cur:
            secs.append((cur, render(buf)))
    for ln in lines:
        if ln.startswith("# "):
            continue
        if ln.startswith("## "):
            flush(); buf = []; cur = ln[3:].strip(); continue
        buf.append(ln)
    flush()
    return secs


def render(lines):
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
            s, i = parse_list(lines, i); out.append(s)
        else:
            j, para = i, []
            while j < len(lines) and lines[j].strip() and not lines[j].startswith(("#", "|")) and not LIST_RE.match(lines[j]):
                para.append(lines[j].strip()); j += 1
            out.append("<p>" + inline(" ".join(para)) + "</p>"); i = j
    return "\n".join(out)


def area_page(path):
    name = os.path.basename(path)[:-3]
    text = open(path, encoding="utf-8").read()
    title = re.match(r"# (?:\d+ )?(.*)", text).group(1)
    secs = md_sections(text)
    lead = secs[0][1] if secs and secs[0][0] is None else ""
    body = "".join(f'<section><h2>{html.escape(t)}</h2>{h}</section>' for t, h in secs if t)
    num = name.split("_")[0]
    return {"id": slug(name), "num": num, "title": title, "lead": lead, "body": body}


def bugs_page():
    p = os.path.join(ROOT, "docs", "playthrough_bugs.md")
    text = open(p, encoding="utf-8").read()
    text = text.replace("(walkthrough/", "(")
    parts = text.split("## Details")
    head = md_sections(parts[0])[0][1]
    det = render(parts[1].split("\n")) if len(parts) > 1 else ""
    return {"id": "bugs", "num": "!", "title": "Bugs found", "lead": head, "body": f"<section><h2>Details</h2>{det}</section>"}


def overview():
    s = STATUS
    party = "".join(f'<li><span class="mon">{n}</span> <span class="lv">{l}</span><span class="mv">{m}</span></li>' for n, l, m in s["party"])
    rows = [("Now at", s["where"]), ("Badges", s["badges"]), ("Money", s["money"]), ("Pokédex", s["dex"]),
            ("Bag", s["bag"]), ("Play time", s["time"]), ("Save file", f'<code>{s["save"]}</code>')]
    dl = "".join(f"<div><dt>{a}</dt><dd>{b}</dd></div>" for a, b in rows)
    legs = [("Kanto", "eight badges, the Elite Four, the Hall of Fame", "now"), ("The ship to Hoenn", "the post-game opens", ""),
            ("Hoenn", "eight badges, then Hoenn's Elite Four", ""), ("Back to the Sevii Islands", "rematches and the Mega Stones", ""),
            ("Mega Island", "win the final tournament", "")]
    lg = "".join(f'<li class="{c}"><strong>{a}</strong><span>{b}</span></li>' for a, b, c in legs)
    concepts = [
        ("Pallet Town", "Open the START menu and re-enter the map before the sign lady clears the way north."),
        ("Route 1", "The Mart clerk gives one sample Potion. Ledges can be jumped, southward only."),
        ("Viridian City", "The parcel from the Mart goes to OAK, who gives the POKéDEX and 5 POKé BALLS."),
        ("Viridian City", "Weaken a wild POKéMON before throwing a ball: a healthy one can escape. The TEACHY TV explains the basics."),
    ]
    cc = "".join(f"<li><strong>{a}</strong>{b}</li>" for a, b in concepts)
    body = (f'<section><h2>The run so far</h2><div class="status"><dl>{dl}</dl><div><h3>Party</h3><ul class="party">{party}</ul></div></div></section>'
            f'<section><h2>The five legs</h2><ol class="legs">{lg}</ol></section>'
            f'<section><h2>What the people taught</h2><ul class="learned">{cc}</ul></section>')
    lead = ("<p>A new game on this build, played headless from the title screen to the end of the story and checked for bugs on the way. "
            "ETHAN, Pikachu start, text speed fast, battle scenes off. Each area has what to do, what is where, who fights and what they carry.</p>")
    return {"id": "overview", "num": "", "title": "The run", "lead": lead, "body": body}


TEMPLATE = open(os.path.join(os.path.dirname(__file__), "walkthrough_template.html"), encoding="utf-8").read()


def main(out):
    pages = [overview()]
    for p in sorted(glob.glob(os.path.join(WT, "[0-9][0-9]_*.md"))):
        pages.append(area_page(p))
    pages.append(bugs_page())
    nav = "".join(f'<a href="#{p["id"]}" data-id="{p["id"]}"><span class="n">{p["num"]}</span><span class="t">{html.escape(p["title"])}</span></a>' for p in pages)
    secs = "".join(
        f'<article id="p-{p["id"]}" hidden><header><p class="eyebrow">{"Area " + p["num"] if p["num"] not in ("", "!") else ("Bug log" if p["num"] == "!" else "Overview")}</p>'
        f'<h1>{html.escape(p["title"])}</h1><div class="lead">{p["lead"]}</div></header>{p["body"]}</article>' for p in pages)
    open(out, "w", encoding="utf-8").write(TEMPLATE.replace("%%NAV%%", nav).replace("%%PAGES%%", secs))
    print(out, len(pages), "pages")


if __name__ == "__main__":
    main(sys.argv[1])
