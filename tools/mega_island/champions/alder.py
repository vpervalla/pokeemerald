from pix import *
# Alder: wild red hair, tanned, an off-white poncho with orange patterns, a necklace of Poke Balls
PAL=[(0,0,0),(28,20,20),(248,120,72),(216,72,48),(152,40,32),(240,192,152),(200,144,104),
     (248,244,228),(216,208,184),(168,156,128),(232,144,48),(120,128,72),(84,92,48),(255,255,255),(200,40,40),(64,56,48)]
T,O,R0,R1,R2,K0,K1,P0,P1,P2,OR,G0,G1,W,BR,DK=range(16)
def draw():
    p=Pic(PAL)
    # wild hair mass behind the head
    p.poly([(20,10),(18,4),(25,6),(27,1),(32,4),(36,0),(39,5),(45,3),(44,9),(48,12),(44,15),(46,22),(41,20),(38,24),(26,24),(23,20),(18,22),(20,16),(16,13)],
           lambda x,y: R0 if (x<27 and y<11) else (R2 if (x-(y//2))%5==0 or x>43 else R1))
    # trousers (olive), rolled up, and sandals
    p.poly([(26,44),(38,44),(39,58),(34,58),(32,50),(30,58),(25,58)],lambda x,y: G0 if x<32 else G1)
    p.rect(24,58,7,2,K1); p.rect(33,58,7,2,K1); p.rect(24,60,7,2,DK); p.rect(33,60,7,2,DK)
    # poncho: wide, draped over the shoulders down to the hips
    p.poly([(26,20),(38,20),(46,26),(49,40),(44,47),(32,49),(20,47),(15,40),(18,26)],
           lambda x,y: P0 if x<26 else (P1 if x<40 else P2))
    for x in range(17,48):             # orange zigzag band near the hem
        y=43+(abs((x%6)-3)>1)
        p.set(x,y,OR); p.set(x,y+1,R1)
    # arms from under the poncho, big hands
    p.poly([(17,38),(21,40),(19,47),(15,46)],K1); p.ell(16.5,47.5,2.5,2.5,K0)
    p.poly([(47,38),(43,40),(45,47),(49,46)],K1); p.ell(47.5,47.5,2.5,2.5,K1)
    # necklace of Poke Balls
    for i,(bx,by) in enumerate(((26,26),(28,29),(31,31),(34,31),(37,29),(39,26))):
        p.ell(bx+.5,by+.5,1.4,1.4,BR); p.set(bx,by+1,W); p.set(bx+1,by+1,W)
    # neck and face, stubble
    p.rect(29,17,6,4,K1)
    p.ell(32,12,6,7,lambda x,y: K0 if x<34 else K1)
    p.rect(27,15,10,4,lambda x,y: 0) if False else None
    for (x,y) in ((28,16),(30,17),(33,17),(35,16),(31,18),(34,18),(29,18)): p.set(x,y,K1)
    p.rect(29,11,2,2,DK); p.rect(34,11,2,2,DK); p.set(29,11,O); p.set(34,11,O)
    p.line(28,9,30,9,R2); p.line(33,9,36,9,R2)
    p.line(31,16,33,16,R2)
    # spiky fringe over the forehead
    p.poly([(24,10),(26,4),(30,6),(33,2),(36,6),(40,5),(41,10),(38,8),(36,10),(33,7),(30,9),(27,8)],
           lambda x,y: R0 if y<6 else R1)
    p.outline(O)
    return p
if __name__=='__main__': draw().img().resize((256,256),0).save('/tmp/claude-0/alder.png')
