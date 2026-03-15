; Bubble Sort Example for 5 elements
; Array stored at memory locations 100-104

ldc 100
stl arraybase       ; store base address of array

ldc 5
stl arraylen        ; store array length

; Outer loop index
ldc 0
stl i

outerloop:
ldl i
ldl arraylen
sub
ldc 1
sub
brlz endouter       ; if (arraylen - i - 1) <= 0 then end outer loop

; Inner loop index
ldc 0
stl j

innerloop:
ldl j
ldl arraylen
ldl i
sub
ldc 1
sub
sub
brlz endinner       ; if j >= arraylen - i - 1 then end inner loop

; Compare array[j] and array[j+1]
ldl arraybase
ldl j
adc 0
ldnl
stl temp

ldl arraybase
ldl j
adc 1
ldnl
stl next

ldl temp
ldl next
sub
brlz noswap         ; if array[j] <= array[j+1], skip swap

; Swap array[j] and array[j+1]
ldl next
ldl arraybase
ldl j
adc 0
stnl

ldl temp
ldl arraybase
ldl j
adc 1
stnl

noswap:
ldl j
ldc 1
add
stl j               ; j++

br innerloop

endinner:
ldl i
ldc 1
add
stl i               ; i++

br outerloop

endouter:
HALT

; Data for array
data arraybase
data arraylen
data 45
data 12
data 33
data 7
data 18