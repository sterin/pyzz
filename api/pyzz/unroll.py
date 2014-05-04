from .pyzz import netlist, wwmap

class unrollx(object):
    
    def __init__(self, N, unroll=0, init = True):
        
        self.init = init
        
        self.N = N
        self.F = netlist()
        
        self.wmaps = [] 
        
        self.frames = 0

        self.unroll(unroll)
        
    def unroll_wire(self, w, k):
        
        if w.is_And():
            
            f = self.wmaps[k][w[0]] & self.wmaps[k][w[1]]
        
        elif w.is_PI():
            
            f = self.F.add_PI()

        elif w.is_Flop():
            
            if k == 0:
                if self.init:
                    f = ~self.F.get_True()
                else:
                    f = self.F.add_PI()
            else:
                f = self.wmaps[k-1][w[0]]

        elif w.is_PO():
            
            f = self.F.add_PO()
            f[0] = self.wmaps[k][w[0]]

        elif w.is_True():
            
            f = self.F.get_True()
        
        self.wmaps[k][w] = f 
        
    def unroll_frame(self):

        m = wwmap()
        m[ self.N.get_True() ] = self.F.get_True()
        
        self.wmaps.append( m )

        sinks = list(self.N.get_POs()) + list(self.N.get_Flops()) + list(self.N.get_PIs()) + list(self.N.get_Ands())
        
        for w in self.N.uporder( sinks ):
            self.unroll_wire(w, self.frames)
            
        self.frames += 1
        
    def unroll(self, k):
        
        for i in xrange(k):
            self.unroll_frame()
            
        return self

    def __getitem__(self, item):
        return self.map_one(item[0], item[1])
        
    def map(self, wires, k):
        
        wmap = self.wmaps[k]
        
        for w in wires:
            yield wmap[w]

    def map_one(self, w, k):
        
        return self.wmaps[k][w]

