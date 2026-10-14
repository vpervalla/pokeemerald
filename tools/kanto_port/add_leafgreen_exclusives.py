"""Add LeafGreen's version exclusives to the Kanto wild encounter tables, where their FireRed counterparts live.

    python3 tools/kanto_port/add_leafgreen_exclusives.py --frlg ../pokefirered          # show the changes
    python3 tools/kanto_port/add_leafgreen_exclusives.py --frlg ../pokefirered --write  # apply them

Run it on FireRed's tables, as written by `port_kanto.py encounters`. For each table, every species that
LeafGreen has and FireRed doesn't goes in the slots where FireRed has its counterpart:
- if the counterpart has other slots in that table, the LeafGreen species takes all of those slots;
- if they share several slots, they split them, balancing the rates (which of the two gets the most
  common slot alternates from one area to the next);
- if they share a single slot, a true version exclusive takes it in every other area, unless it's
  already in that area. Other species (like NIDORAN F, found elsewhere in FireRed) are left alone.
Fishing slots are only exchanged within the same rod. No other species loses a slot.
"""
import json,sys,collections,os
FRLG=sys.argv[sys.argv.index('--frlg')+1] if '--frlg' in sys.argv else '../pokefirered'
F=json.load(open(os.path.join(FRLG,'src/data/wild_encounters.json')))['wild_encounter_groups'][0]
E={e['base_label']:e for e in F['encounters']}
P='src/data/wild_encounters.json'
R=json.load(open(P))
rg=[g for g in R['wild_encounter_groups'] if g['label']=='gWildMonHeaders'][0]
rates={f['type']:f['encounter_rates'] for f in rg['fields']}
FIELDS=['land_mons','water_mons','rock_smash_mons','fishing_mons']
GROUPS={'fishing_mons':[[0,1],[2,3,4],[5,6,7,8,9]]}
def sig(e): return {f:[m['species'] for m in e[f]['mons']] for f in FIELDS if f in e}
def allsp(lbl_suffix):
    s=set()
    for e in F['encounters']:
        if e['base_label'].endswith(lbl_suffix):
            for f in FIELDS:
                if f in e: s|={m['species'] for m in e[f]['mons']}
    return s
EXCL=allsp('_LeafGreen')-allsp('_FireRed')
alt=collections.Counter(); log=[]
for lab,fe in E.items():
    if not lab.endswith('_FireRed'): continue
    le=E[lab[:-7]+'LeafGreen']
    cand=[e for e in rg['encounters'] if e['map'] in (fe['map'],fe['map'].replace('MAP_','MAP_KANTO_')) and sig(e)==sig(fe)]
    if len(cand)!=1: sys.exit(f'no FireRed table found for {lab}: were the exclusives already added?')
    re_=cand[0]; name=lab[1:-8]
    singles=[]
    for f in FIELDS:
        if f not in fe: continue
        mons=re_[f]['mons']; a=[m['species'] for m in fe[f]['mons']]; lm=le[f]['mons']; b=[m['species'] for m in lm]
        for grp in GROUPS.get(f,[list(range(len(a)))]):
            for y in sorted({b[i] for i in grp}-set(a)):
                L=[i for i in grp if b[i]==y]; x=a[L[0]]
                if any(a[j]==x for j in grp if j not in L):
                    # the counterpart has other slots here: the exclusive takes all of its LeafGreen slots
                    give=L
                elif len(L)>=2:
                    # split the slots, balancing the rates; which of the two gets the most common slot
                    # alternates from one area to the next
                    alt[('split',x,y)]+=1; first_y=alt[('split',x,y)]%2==0
                    tx=ty=0; give=[]
                    for i in sorted(L,key=lambda i:(-rates[f][i],i)):
                        to_y = ty < tx or (ty == tx and first_y)
                        if to_y: ty+=rates[f][i]; give.append(i)
                        else: tx+=rates[f][i]
                else:
                    singles.append((f,L[0],x,y)); continue
                for i in give:
                    mons[i]=dict(lm[i]); log.append((name,f,i,rates[f][i],a[i][8:],'->',y[8:]))
    # single slots whose counterpart has nowhere else to go: only for true exclusives that aren't in the area yet,
    # alternating between the two from one area to the next
    for f,i,x,y in singles:
        present={m['species'] for f2 in FIELDS if f2 in re_ for m in re_[f2]['mons']}
        if y in present or y not in EXCL: log.append(('skip',name,f,i,x[8:],y[8:],'present' if y in present else 'not exclusive')); continue
        alt[(x,y)]+=1
        if alt[(x,y)]%2==1:
            re_[f]['mons'][i]=dict(le[f]['mons'][i]); log.append((name,f,i,rates[f][i],x[8:],'->',y[8:],'(alternating)'))
        else: log.append(('keep',name,f,i,x[8:],'instead of',y[8:],'(alternating)'))
for l in log: print(*l)
if '--write' in sys.argv: open(P,'w').write(json.dumps(R,indent=2)+'\n')
