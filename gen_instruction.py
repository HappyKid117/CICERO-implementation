# match a
# matchany
# split 4
# jump 10
inst = []
n = int(input())
for i in range(n):
    inst.append(str(input()).split(' '))

for i in range(n):
    s = "    instructionMemory["
    s+=str(i+1)
    s+="] = 0b"

    if(inst[i][0] == "match"):
        s+="001"
        b = "{0:b}".format(ord(inst[i][1])-ord("a"))
        s+="0"*(13-len(b))
        s+=b
    
    if(inst[i][0] == "matchany"):
        s+="000"
        s+="0000000000000"
    
    if(inst[i][0] == "split"):
        s+="011"
        b = "{0:b}".format(int(inst[i][1]))
        s+="0"*(13-len(b))
        s+=b
    
    if(inst[i][0] == "jump"):
        s+="100"
        b = "{0:b}".format(int(inst[i][1]))
        s+="0"*(13-len(b))
        s+=b

    if(inst[i][0] == "accept_partial"):
        s+="110"
        s+="0000000000000"

    s+=";"
    s+= " // "
    s+=(" ".join(inst[i]))
    print(s)
    
