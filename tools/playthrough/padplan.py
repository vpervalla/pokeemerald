"""padplan.py X0 Y0 TX TY : BFS over Saffron Gym teleport pads (same-map warps); prints the list of pads (x,y) to step on, then 'TARGET'."""
import json,subprocess,sys,collections
R='/home/user/pokeemerald'
d=json.load(open(f'{R}/data/maps/SaffronCity_Gym/map.json'))
W=d['warp_events']
pads={i:(w['x'],w['y']) for i,w in enumerate(W) if w['dest_map']=='MAP_SAFFRON_CITY_GYM'}
dest={i:pads[int(W[i]['dest_warp_id'])] for i in pads}
cache={}
def reach(a,b,adj=False):
    k=(a,b,adj)
    if k not in cache:
        cmd=['python3',f'{R}/tools/playthrough/path.py','SaffronCity_Gym',str(a[0]),str(a[1]),str(b[0]),str(b[1])]+(['--adj'] if adj else [])
        out=subprocess.run(cmd,capture_output=True,text=True,cwd=f'{R}/tools/playthrough').stdout
        cache[k]='end' in out
    return cache[k]
x0,y0,tx,ty=map(int,sys.argv[1:5])
start=(x0,y0); q=collections.deque([start]); prev={start:None}
while q:
    p=q.popleft()
    if reach(p,(tx,ty),True):
        path=[]; c=p
        while prev[c]: path.append(prev[c][1]); c=prev[c][0]
        print(' '.join(f'{a},{b}' for a,b in reversed(path)) or '-'); sys.exit(0)
    for i,pp in pads.items():
        if pp==p: continue
        if reach(p,pp):
            n=dest[i]
            if n not in prev: prev[n]=(p,pp); q.append(n)
print('NONE')
