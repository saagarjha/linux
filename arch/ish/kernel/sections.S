# Darwin doesn't have linker scripts, so sections appear in the order they show
# up an object files. This object takes up no space and has the sections in the
# desired order.
#define ___(S,s,c) .section __##S,__##s; _##c:
#define __(S,s,c) ___(S,s,c)
#define _(S,s) __(S,s,__COUNTER__)

_(DATA,initcallearly)
_(DATA,initcall0)
_(DATA,initcall1)
_(DATA,initcall2)
_(DATA,initcall3)
_(DATA,initcall4)
_(DATA,initcall5)
_(DATA,initcallrootfs)
_(DATA,initcall6)
_(DATA,initcall7)

# For some reason leaving this out makes it put the last section first
.data
