import pyzz
print dir(pyzz)
    
from pyzz import *


N = netlist()
K = 5

pis = [ N.add_PI(i) for i in xrange(K) ]

def conjunction(N, wires):

    if len(wires)==1:
        return wires[0]

    if len(wires)==2:
        return wires[0] & wires[1]
    
    if len(wires)==0:
        return N.get_True()

    return conjunction(N, wires[0:len(wires)/2]) & conjunction(N, wires[len(wires)/2:])

pis = [w for w in N.get_PIs() ]


print len(pis)

res = conjunction(N, pis)

po = N.add_PO(1)
po[0] = res

s = solver(N)

v1 = s.new_var()
v2 = s.new_var()

s.clause( [~res], control=v1 )
s.clause( [res], control=v2 )

print s
print s.solve([v2])

print s.value(v1)
print s.value(v2)

#~ for w in pis:
    #~ print s.value(w)


#~ for w in ( list(N.uporder( [po] )) + [~N.get_True()] ) :
    #~ print "%-10s"%w , w.is_True(w), w.is_PI(w), w.is_And(w), w.is_Flop(w), w.is_PO(w), w.sign()

#~ m = wwmap()

#~ for i in xrange(10):
    #~ m[pis[i]] = pos[i]

#~ for i in xrange(10):
    #~ print m[pis[i]]

#~ m.clear()

#~ print pis[0] in m


#~ print len(m)

#~ for i in xrange(10):
    #~ print m[pis[i]], pis[i] in m
#~ print "Help"