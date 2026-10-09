from pix import *
# Leon: long purple hair under a snapback cap, a red cape covered in patches, a black and white
# champion's outfit, one fist raised (his Charizard pose)
PAL=[(0,0,0),(24,16,28),(176,120,208),(128,80,168),(88,48,120),(240,196,160),(200,148,112),
     (232,56,56),(184,32,40),(120,20,32),(248,248,248),(200,200,208),(48,48,56),(248,200,64),(80,160,224),(32,32,36)]
T,O,U0,U1,U2,K0,K1,R0,R1,R2,W0,W1,B0,GD,BL,B1=range(16)
def draw():
    p=Pic(PAL)
    # cape behind, flowing to the left
    p.poly([(24,20),(40,20),(46,40),(48,60),(30,58),(14,61),(10,52),(18,36)],
           lambda x,y: R0 if x<20 else (R1 if x<40 else R2))
    for (px,py,c) in ((15,48,GD),(19,54,BL),(43,50,W0),(13,56,W0),(41,57,GD)):   # sponsor patches
        p.rect(px,py,3,2,c)
    # long purple hair down the back
    p.poly([(24,8),(40,8),(43,26),(41,38),(36,30),(28,30),(23,38),(21,26)],lambda x,y: U1 if (x+y)%5 else U2)
    # legs: black leggings and white shorts, sneakers
    p.poly([(27,42),(37,42),(37,58),(33,58),(32,50),(31,58),(27,58)],lambda x,y: B0 if x<32 else B1)
    p.rect(26,40,12,6,W1); p.rect(26,40,6,6,W0)
    p.rect(25,58,7,4,W1); p.rect(33,58,7,4,W1); p.rect(25,61,7,1,R1); p.rect(33,61,7,1,R1)
    # torso: black top with a white collar stripe
    p.poly([(25,20),(39,20),(40,32),(38,41),(26,41),(24,32)],lambda x,y: B0 if x<34 else B1)
    p.line(28,21,32,26,W0); p.line(36,21,32,26,W0); p.rect(31,30,2,8,W1)
    # left arm down, right arm raised in a fist
    p.poly([(25,21),(22,24),(21,34),(22,40),(25,40),(26,30)],lambda x,y: B0)
    p.ell(23,41.5,2.3,2.5,K0)
    p.poly([(39,21),(43,18),(46,11),(49,8),(51,11),(47,19),(42,26)],lambda x,y: B1 if y>16 else K1)
    p.ell(49.5,7.5,3,3,K1); p.set(49,6,K0); p.set(50,6,K0)
    # neck and face
    p.rect(30,17,4,4,K1)
    p.ell(32,12,5.5,6.4,lambda x,y: K0 if x<34 else K1)
    p.rect(29,12,2,2,GD); p.rect(33,12,2,2,GD); p.set(29,12,O); p.set(33,12,O)
    p.line(29,10,30,10,U2); p.line(33,10,34,10,U2); p.line(30,16,33,16,K1)
    # snapback cap with a crown, and the fringe
    p.poly([(24,9),(26,4),(32,2),(38,4),(40,9)],lambda x,y: B0 if x<34 else B1)
    p.rect(23,8,19,2,B1); p.rect(18,9,8,2,B0)        # brim pointing to the side
    p.set(31,4,GD); p.set(32,3,GD); p.set(33,4,GD); p.rect(31,5,3,1,GD)
    p.poly([(25,10),(29,10),(26,14)],U0); p.poly([(36,10),(40,10),(41,17),(38,14)],U1)
    p.outline(O)
    return p
if __name__=='__main__': draw().img().resize((256,256),0).save('/tmp/claude-0/leon.png')
