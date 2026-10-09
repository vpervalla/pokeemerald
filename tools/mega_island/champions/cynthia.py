from pix import *
PAL=[(0,0,0),(24,20,32),(252,244,188),(236,208,112),(184,144,64),(255,228,204),(232,176,144),
     (96,92,112),(60,58,76),(34,32,46),(176,172,188),(88,104,136),(255,255,255),(208,112,112),(130,126,146),(0,0,0)]
T,O,H0,H1,H2,K0,K1,C0,C1,C2,F,E,W,L,F1,_=range(16)
def draw():
    p=Pic(PAL)
    # long hair behind the body, to the waist
    p.poly([(23,8),(26,3),(35,2),(41,6),(43,16),(44,30),(47,46),(41,48),(37,34),(27,34),(23,48),(16,46),(20,30),(21,16)],
           lambda x,y: H2 if (x<22 or x>42) else (H1 if (x+y)%7 else H2))
    # legs (black trousers) and heels
    p.poly([(28,32),(36,32),(37,60),(33,60),(32,50),(31,60),(27,60)],lambda x,y: C1 if x<32 else C2)
    p.line(32,50,32,44,O)
    p.rect(26,60,5,2,C2); p.rect(33,60,5,2,C2)
    # long black coat, open below the waist
    p.poly([(23,21),(41,21),(43,34),(42,44),(47,58),(38,58),(34,44),(33,32),(31,32),(30,44),(26,58),(17,58),(22,44),(21,34)],
           lambda x,y: C0 if x<25 else (C1 if x<38 else C2))
    for y in range(46,58): p.set(35+(y-46)//3,y,C0)     # coat fold highlight
    # inner top and the V of the neckline
    p.poly([(29,21),(35,21),(32,27)],lambda x,y: K0 if x<33 else K1)
    p.poly([(28,27),(36,27),(35,33),(29,33)],C2)   # her black top under the coat
    # arms in the sleeves
    p.poly([(23,22),(26,23),(24,36),(22,42),(19,41),(20,32)],lambda x,y: C0 if x<22 else C1)
    p.poly([(41,21),(38,23),(42,31),(39,36),(41,38),(46,32),(44,25)],lambda x,y: C1 if x<43 else C2)   # hand on the hip
    p.ell(20,43,2.2,2.5,K0); p.ell(39.5,37,2.2,2,K1)
    # fur collar
    p.poly([(23,19),(29,20),(32,24),(35,20),(41,19),(43,23),(37,24),(32,30),(27,24),(21,23)],lambda x,y: F if (x+y)%4 else F1)
    # neck and face
    p.rect(30,17,4,4,K1)
    p.ell(31.5,12.5,5.6,6.5,lambda x,y: K0 if x<34 else K1)
    # eyes, mouth
    p.rect(28,12,2,2,E); p.set(28,12,O); p.set(29,13,W); p.hline=None
    for x in (27,28,29,30): p.set(x,10,H2)
    p.set(31,16,L); p.set(32,16,L)
    # fringe: swept over her left eye (our right), with the black hair clips
    p.poly([(25,4),(31,3),(38,4),(39,9),(38,17),(35,20),(36,12),(33,9),(30,8),(27,10),(25,13),(25,8)],
           lambda x,y: H0 if y<6 else (H1 if (x*3+y)%5 else H2))
    p.poly([(36,10),(38,9),(39,16),(37,21)],H1)
    p.ell(25,8,2,2.6,C2); p.ell(39,8,2,2.6,C2); p.set(24,7,C0); p.set(38,7,C0)
    p.outline(O)
    return p
if __name__=='__main__':
    draw().img().resize((256,256),0).save('/tmp/claude-0/cynthia.png')
