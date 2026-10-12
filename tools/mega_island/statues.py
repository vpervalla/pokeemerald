# The Mewtwo statues that stand at the castle's door and gate, after the user's reference sprite
# (a stone figure on a pedestal, 16x32; the second variant has red eyes). Written by hand from the
# reference, which is a blurred screenshot. Imported by build_castle.py.
COLOURS={'O':(56,56,72),'a':(64,104,80),'b':(88,120,104),'c':(112,144,128),'d':(128,168,152),
         'e':(160,200,184),'P':(104,104,120),'r':(232,48,40),'R':(152,24,32)}
STATUE=[
    "................",
    "................",
    "..OO.......OO...",
    "..OdbOOOOOObdO..",
    "..OceeeeeeeecO..",
    "...OdeeeeeedO...",
    "..OcdeeeeeedcO..",
    "..ObOdeeeedObO..",
    "..ObOOccccOObO..",
    "...ObOccccObO...",
    "...OOabccbaOO...",
    "..OcbOaaaaObcO..",
    ".OccObOOOObOccO.",
    "OedOOccccccOOdeO",
    "OdccOOOcbOOOccdO",
    ".OcOOOaaaaOOOcO.",
    "OOOcbOabbaObcOOO",
    "OObcbaOcbObcbaOO",
    "OOaaaOcccbOaaaOO",
    "OcOOOOaaaaOOOOcO",
    "OcbdbOOOOOObdbcO",
    "OecccccccccccceO",
    "OeddddddddddddeO",
    "OcaaOOOOOOOOaacO",
    "OcaaOPPPPPPOaacO",
    "OcaaOPPPPPPOaacO",
    "OcaaOOOOOOOOaacO",
    "OcaaaaaaaaaaaaaO",
    "OcaaaaaaaaaaaaaO",
    "OcaaaaaaaaaaaaaO",
    "OcaaaaaaaaaaaaaO",
    ".OOOOOOOOOOOOOO.",
]
# the red-eyed variant: the eyes glow
RED_EYES={(7,4):'r',(7,11):'r',(8,4):'R',(8,5):'r',(8,10):'r',(8,11):'R'}

def statue(red=False):
    # 32 rows of 16 RGBA pixels (alpha 0 or 255)
    out=[]
    for y,row in enumerate(STATUE):
        line=[]
        for x,ch in enumerate(row):
            if red: ch=RED_EYES.get((y,x),ch)
            line.append(COLOURS[ch]+(255,) if ch!='.' else (0,0,0,0))
        out.append(line)
    return out
