from pix import *
# Diantha: short dark hair with curled ends, a long white gown with a pink sash and gloves
PAL=[(0,0,0),(28,20,28),(96,72,72),(64,44,48),(40,28,32),(255,228,212),(232,180,160),
     (255,255,255),(232,232,240),(192,192,212),(248,160,184),(216,104,144),(144,152,184),(180,96,96),(80,64,120),(0,0,0)]
T,O,H0,H1,H2,K0,K1,W0,W1,W2,PK0,PK1,G,L,E,_=range(16)
def draw():
    p=Pic(PAL)
    # gown: fitted bodice, then a long skirt flaring to the floor
    p.poly([(26,22),(38,22),(39,34),(43,48),(48,61),(16,61),(21,48),(25,34)],
           lambda x,y: W0 if x<28 else (W1 if x<38 else W2))
    for x0 in (24,30,36):                     # long folds in the skirt
        p.line(x0+2,40,x0-1+(x0-30)//2,60,W2)
    # pink sash at the waist with a long ribbon
    p.rect(25,33,15,3,PK0); p.hline=None
    for x in range(25,40): p.set(x,35,PK1)
    p.poly([(37,35),(40,35),(41,48),(38,46)],PK1)
    # bare shoulders and arms in long white gloves
    p.ell(26,23,3,2.3,K0); p.ell(38,23,3,2.3,K1)
    p.poly([(24,24),(27,25),(25,34),(23,42),(20,41),(22,32)],lambda x,y: W0 if y>29 else K0)
    p.poly([(40,24),(37,25),(39,34),(41,42),(44,41),(42,32)],lambda x,y: W1 if y>29 else K1)
    p.ell(21.5,42.5,2,2.3,W0); p.ell(42.5,42.5,2,2.3,W1)
    # neckline and the neck, a pink choker
    p.poly([(27,21),(37,21),(35,24),(29,24)],K0)
    p.rect(30,16,4,5,K1); p.rect(30,18,4,1,PK1)
    # face
    p.ell(32,11.5,5.4,6.4,lambda x,y: K0 if x<34 else K1)
    p.rect(29,11,2,2,E); p.rect(33,11,2,2,E); p.set(29,11,O); p.set(33,11,O)
    p.line(29,9,30,9,H2); p.line(33,9,34,9,H2); p.set(31,15,L); p.set(32,15,L)
    # short dark hair: a side-swept bob whose ends curl outwards
    p.poly([(25,9),(27,4),(32,2),(37,3),(39,7),(40,12),(39,16),(41,19),(38,20),(37,14),(37,9),(33,6),(28,8),(27,13),(27,17),(25,20),(22,19),(24,15)],
           lambda x,y: H0 if (x<30 and y<8) else (H1 if (x+y)%5 else H2))
    p.ell(23.5,19.5,2,1.6,H1); p.ell(40.5,19.5,2,1.6,H1)
    p.outline(O)
    return p
if __name__=='__main__': draw().img().resize((256,256),0).save('/tmp/claude-0/diantha.png')
