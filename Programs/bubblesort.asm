        ldc 5
        stl 0

        ldc 4
        stl 1

        ldc 3
        stl 2

        ldc 2
        stl 3

        ldc 1
        stl 4

        ldc 0
        stl 5

outer:  ldl 5
        ldc 4
        sub
        brz end

        ldc 0
        stl 6

inner:  ldl 6
        ldc 4
        sub
        brz next

        ldl 6
        ldnl 0

        ldl 6
        ldc 1
        add
        ldnl 0

        sub
        brlz skip

        ldl 6
        ldnl 0
        stl 7

        ldl 6
        ldc 1
        add
        ldnl 0
        stnl 6

        ldl 7
        ldl 6
        ldc 1
        add
        stnl 0

skip:   ldl 6
        ldc 1
        add
        stl 6
        br inner

next:   ldl 5
        ldc 1
        add
        stl 5
        br outer

end:    HALT