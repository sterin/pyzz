import time
import sys
import optparse
import itertools

import pyzz

class measurement(object):
    
    def __init__(self):
        
        self.running = False
        self.start_time = 0.0
        self.end_time = 0.0
        self.total = 0.0
        
    def start(self):
        
        self.total += self.end_time-self.start_time
        self.running = True
        self.start_time = time.time()
        
    def stop(self):

        self.end_time = time.time()
        self.running = False

    def current(self):

        if self.running:
            return time.time() - self.start_time
        
        return self.end_time - self.start_time

    def elapsed(self):
        
        return self.total + self.current()
    
    def __enter__(self):
        self.start()
        
    def __exit__(self, exc_type, exc_value, traceback):
        self.stop()

class prop_result(object):
    
    def __init__(self, po):
        
        self.po = po
        self.min = -1
        self.k = -1
        self.cex = None
        
    def done(self, start, end):
        if self.k > -1:
            return True
        elif self.min > end:
            return True
        else:
            return False
        
    def sat(self, k, start, end, cex):
        
        self.k = k
        self.min = start
    
    def unsat(self, start, end):
        
        self.min = end
        
    def __repr__(self):
        return "\{po=%s, min=%d, k=%d\}"%(self.po, self.min, self.k)

class prop_manager(object):
    
    def __init__(self, U, callback, assume_proved = True):
        
        self.U = U
        self.unsolved = set( po for po in self.U.N.get_POs() )
        self.results = dict( (po, prop_result(po) ) for po in self.unsolved )
        self.callback = callback
        self.min = 0
        self.assume_proved = assume_proved

    def done(self, start, end):
    
        return not self.unsolved or self.min>=end
                
    def bmc_assumptions(self, S, start, end):

        var = S.new_var()

        clause = list( pyzz.chain( self.U[self.unsolved, k] for k in xrange(start, end) ) )
        
        S.clause( clause, control=var )
        
        return [var]
                
    
    def analyze_sat(self, S, start, end):
        
        cex = pyzz.cex(self.U, S)
        
        solved = set()
        
        for po in self.unsolved:
            
            for k in xrange(start, end):
                
                fpo = self.U.map_one(po, k)
                
                if S.value(fpo) == pyzz.l_True:
                    self.results[po].sat( k, start, end, cex)
                    solved.add(po)
                    self.callback( self.results[po] )
                    break
            
        self.unsolved -= solved

    def analyze_unsat(self, S, start, end):

        self.min = end
        
        for po in self.unsolved:
            self.results[po].unsat( start, end)
        
        if self.assume_proved:
            for k in xrange(start, end):
                for fpo in self.U.map(self.unsolved, k):
                    S.clause( [~fpo] )

def steps(end, step):
    
    if not end:
        end = sys.maxint
        
    for frame in xrange( 0, end, step):
        yield frame, min(frame+step, end)

def bmc(N, max=None, step=1, assume_proved=True, verbose=True, callback=lambda *args, **kwargs: None):

    U = pyzz.unroll(N)
    S = pyzz.solver(U.F)

    total_time = measurement()
    unroll_time = measurement()
    solve_time = measurement()
    analyze_time = measurement()
    
    props = prop_manager(U, callback=callback, assume_proved=assume_proved)

    for start, end in steps(max, step):
        with total_time:
        
            try:

                with unroll_time:
                    U.unroll(step)
 
                while not props.done(start, end):

                    with solve_time:
                        rc = S.solve( props.bmc_assumptions(S, start, end) )
                        
                    with analyze_time:
                        if rc == pyzz.l_True:
                            props.analyze_sat(S, start, end)
                        elif rc == pyzz.l_False:
                            props.analyze_unsat(S, start, end)
                        else:
                            return
                
                if not props.unsolved:
                    return
                
            finally:
                if verbose:
                    print "%5d-%-5d:"%(start, end-1),
                    print "    unroll=%8.2fs,"%unroll_time.current(),
                    print "    solve=%8.2fs,"%solve_time.current(),
                    print "    analyze=%8.2fs,"%analyze_time.current(),
                    print "    iteration=%8.2fs,"%total_time.current(),
                    print "    total=%8.2fs"%total_time.elapsed()
    return

def main():
    parser = optparse.OptionParser()
    
    parser.add_option("--frames", dest="frames", type="int", default=None)
    parser.add_option("--step", dest="step", type="int", default=1)
    parser.add_option("--verbose", dest="verbose", action="store_true", default=False)
    
    options, args = parser.parse_args()
    
    N = pyzz.netlist.read_aiger(args[0])
    
    def result_callback(result):
        print result
    
    results = bmc(N, max=options.frames, verbose=options.verbose, step=options.step, callback=result_callback)
    
if __name__ == "__main__":
    main()
