class union_find(object):

    def __init__(self, N):
        self.parent = range(N)
        self.rank = [0]*N
        
    def find(self, x):
        z = x
        
        while x != self.parent[x]:
            x = self.parent[x]
            
        while z != self.parent[z]:
            self.parent[z], z = x, self.parent[z]
            
        return x
        
    def union(self, x, y):
        x = self.find(x)
        y = self.find(y)
        
        if x == y:
            return
        
        if self.rank[x] < self.rank[y]:
            self.parent[x] = y

        elif self.rank[x] > self.rank[y]:
            self.parent[y] = x

        else:
            self.parent[y] = x
            self.rank[x] += 1

