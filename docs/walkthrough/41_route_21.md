# 41 Route 21 (the sea from Pallet to Cinnabar)

**Arrival.** From Pallet Town's pond (7..9,17..19): SURF south. Route 21 is two maps, North (24x50, a fenced patch of tall grass at the top, x 11..16, y 0..10) and South, joined at the bottom; the South half's sand strip (12..13,45..49) lands on Cinnabar Island's north shore.

## Map notes

- **Wild:** the tall grass at the top of the North map has only **TANGELA**; on the water TENTACOOL (and TENTACRUEL); fishing tables (Horsea, Krabby, Seadra, Gyarados, Psyduck, Magikarp; the South half adds Kingler and Slowpoke) not tried yet.
- **Trainers:** North: fishermen **Ronald** (7,27) on the west island and **Wade** (16,26) on the east island (talk to them: they don't look around), swimmer **Spencer** (12,44, wanders), and **Lil & Ian** (14,35)/(15,35), a double battle (SEADRA and STARMIE L33). South: fishermen **Claude** (11,8) and **Nolan** (14,15) on the island edges, swimmers **Jack** (15,24), **Roland** (10,29), **Jerome** (15,38).
- A hidden **PEARL** at (17,42) on the North map's south-east island (not searched).

## Story

1. **Healing:** FLY (HM02 from the bag, the borrowed PIDGEOT) from Route 20 to Pallet Town; MOM healed the party.
2. **TANGELA L23** caught in the North grass (a GREAT BALL; Box 2), the first-visit rule.
3. **Training SNORLAX** (your request): SNORLAX led every fight. The type rule (`TYPES=1`) switched it out on the first turn against Ronald (it had no super effective move), so Venusaur won that one; from Wade on, `TYPES=move` (best move by type, no type switches) kept SNORLAX in, with the level rule (`SWITCH=auto`) as the safety net.
4. **TM31 BRICK BREAK** taught to SNORLAX in place of YAWN (your choice). At L46 SNORLAX learned **ROLLOUT** in place of SNORE (your choice). BODY SLAM's 15 PP ran out twice: two ETHERS and an ELIXIR.
5. **Lil & Ian** (double): SNORLAX's BRICK BREAK on SEADRA, GENGAR's SHADOW PUNCH on STARMIE; ¥792.
6. Saved on the North map (`saves/ethan_route21.sav`), then the ROM was rebuilt (FLY from the TOWN MAP, see the bugs page, bug 6); the TOWN MAP FLY tests started from that save.
7. Back by FLY to Pallet and down the sea again: Jerome (crossed on the way), Roland, Jack, Nolan and Claude beaten. All nine trainers of Route 21 are beaten.
8. Cinnabar Island's north shore.

## Party after

SNORLAX L47 (BODY SLAM, BRICK BREAK, REST, ROLLOUT; L45 → L47, about 16 000 exp), GENGAR L47, VENUSAUR L48, CHARIZARD L51, DRAGONAIR L47, BLASTOISE L46. Money ¥168 401.

## Bugs

None in the game. Tool notes: a move-learn prompt in the middle of a `fight` stalls it (the learn guard refuses A), and a long `sgoto` can overshoot onto the next map.
