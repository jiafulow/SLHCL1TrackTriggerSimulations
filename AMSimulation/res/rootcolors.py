from ROOT import TColor

kWhite      = 0     #FFFFFF
kBlack      = 1     #000000
kGray       = 920   #CCCCCC
kRed        = 632   #FF0000
kPink       = 900   #FF0033
kMagenta    = 616   #FF00FF
kViolet     = 880   #CC00FF
kBlue       = 600   #0000FF
kAzure      = 860   #0033FF
kCyan       = 432   #00FFFF
kTeal       = 840   #00FFCC
kGreen      = 416   #00FF00
kSpring     = 820   #33FF00
kYellow     = 400   #FFFF00
kOrange     = 800   #FFCC00

#TColor.InitializeColors()
kRed2       = TColor.GetColor("#C02020")
kGreen2     = TColor.GetColor("#20C020")
kBlue2      = TColor.GetColor("#2020C0")
kCyan2      = TColor.GetColor("#20C0C0")
kMagenta2   = TColor.GetColor("#C020C0")
kYellow2    = TColor.GetColor("#C0C020")
kOrange2    = TColor.GetColor("#FFA533")
kGray2      = TColor.GetColor("#E8E8E8")

kYellow3    = TColor.GetColor("#BFAF00")
kOrange3    = TColor.GetColor("#FF9900")

kPurple2    = TColor.GetColor("#800080")
kOlive2     = TColor.GetColor("#808000")
kTeal2      = TColor.GetColor("#008080")
kMaroon2    = TColor.GetColor("#800000")
kNavy2      = TColor.GetColor("#000080")
kLime2      = TColor.GetColor("#008000")

kGold2      = TColor.GetColor("#DAA520")
kSalmon2    = TColor.GetColor("#FA8072")
kPistachio2 = TColor.GetColor("#93C572")
kBrown2     = TColor.GetColor("#964B00")
kCocoa2     = TColor.GetColor("#D2691E")
kTurquoise2 = TColor.GetColor("#40E0D0")

kQuark      = TColor.GetColor("#EA6699")
kLepton     = TColor.GetColor("#608060")
kBoson      = TColor.GetColor("#20ABBF")
kFermion    = TColor.GetColor("#D67F20")

# from http://ethanschoonover.com/solarized
# https://github.com/altercation/ethanschoonover.com/blob/master/resources/css/style.css
sBase03     = TColor.GetColor("#002b36")
sBase02     = TColor.GetColor("#073642")
sBase01     = TColor.GetColor("#586e75")
sBase00     = TColor.GetColor("#657b83")
sBase0      = TColor.GetColor("#839496")
sBase1      = TColor.GetColor("#93a1a1")
sBase2      = TColor.GetColor("#eee8d5")
sBase3      = TColor.GetColor("#fdf6e3")
sYellow     = TColor.GetColor("#b58900")
sOrange     = TColor.GetColor("#cb4b16")
sRed        = TColor.GetColor("#dc322f")
sMagenta    = TColor.GetColor("#d33682")
sViolet     = TColor.GetColor("#6c71c4")
sBlue       = TColor.GetColor("#268bd2")
sCyan       = TColor.GetColor("#2aa198")
sGreen      = TColor.GetColor("#859900")

ssBase3     = TColor.GetColor("#E3EAFD")
ssBase4     = TColor.GetColor("#FCF8E3")
ssBase5     = TColor.GetColor("#FDE3F8")
ssBase6     = TColor.GetColor("#EAFDE3")

#blkrgb      = (kBlack, kBlue, kRed, kGreen)
#blkrgb      = (kBlack, kNavy2, kMaroon2, kLime2)
blkrgb       = map(lambda x: TColor.GetColor(x), ("#333333", "#0571b0", "#ca0020", "#008837", "#7b3294", "#e66101"))

# Very nice color palette stolen from tkgeometry
# https://code.google.com/p/tkgeometry/source/browse/trunk/src/Palette.cc
palette      = map(lambda x: TColor.GetColor(x), ("#004586","#FF420E","#FFD320","#579D1C","#7E0021","#83CAFF","#314004","#AECF00","#4B1F6F","#FF950E","#C5000B","#0084D1"))
lightpalette = map(lambda x: TColor.GetColor(x), ("#79B7F2","#F29379","#FFEDA6","#B1F279","#F27999","#C2DEF2","#D4F279","#DFF279","#C791F2","#F2BD79","#F27980","#AAD7F2"))

#paletteSet1      = map(lambda x: TColor.GetColor(x), ("#e41a1c", "#377eb8", "#4daf4a", "#984ea3", "#ff7f00", "#ffff33", "#a65628", "#f781bf", "#999999"))
#lightpaletteSet1 = map(lambda x: TColor.GetColor(x), ("#f28d8e", "#9bbfdc", "#a6d7a5", "#cca7d1", "#ffbf80", "#ffff99", "#d3ab94", "#fbc0df", "#cccccc"))
paletteSet1      = map(lambda x: TColor.GetColor(x), ("#e41a1c", "#377eb8", "#4daf4a", "#984ea3", "#ff7f00", "#a65628", "#f781bf", "#999999"))
lightpaletteSet1 = map(lambda x: TColor.GetColor(x), ("#f28d8e", "#9bbfdc", "#a6d7a5", "#cca7d1", "#ffbf80", "#d3ab94", "#fbc0df", "#cccccc"))

