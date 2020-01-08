from collections import namedtuple
import matplotlib.pyplot as plt

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []

VEH_SPEED = 0x0D
CATALYST_TEMP_B1_S1 = 0x0C

def getDiagsFromFile(f):
    diags = []
    f = open(f)
    for line in f:
        if line[25:-18] == "7E8":
            diags.append(line.rstrip('\n'))
    f.close()
    return diags

def stripMetaFromDiag(diagList):
    diagMsg = namedtuple('diagMsg', 'timestamp data')
    strippedDiagList = []
    for line in diagList:
        msg = diagMsg(timestamp=line[1:-27], data=bytearray.fromhex(line.split('#')[-1]))
        strippedDiagList.append(msg)
    return strippedDiagList

def scaleData(diagList, servID):
    for line in diagList:
        if line.data[2] == servID:
            if servID == 0x0D:
                xs.append(line.timestamp)
                ys.append(line.data[3] / 1.6) # 1.6km in 1m
            if servID == 0x0C:
                xs.append(line.timestamp)
                ys.append((256 * line.data[3] + line.data[4]) / 4) # ((256*A)+B)/4

def graphDiag(xs, ys):
    plt.plot(xs, ys)
    plt.show()

dl = getDiagsFromFile("2015-volt-firstdiaglist.log")
dl = stripMetaFromDiag(dl)
scaleData(dl, 0x0D)
graphDiag(xs, ys)
