proc set_is_subset {a b} {
    # Check if set $a is a subset of set $b
    set union [lsort -unique [concat $a $b]]
    if {[llength $union] == [llength [lsort -unique $b]]} {
        return 1
    } else {
        return 0
    }
}

proc set_excess {a b} {
    # Find which elements of set $a do not belong to set $b
    set bsorted [lsort -unique $b]
    set union [lsort -unique [concat $a $b]]
    if {[llength $bsorted] == 0} {
        return $union
    }
    set l3 [lsort [concat $union $bsorted]]
    set len [llength $l3]
    set result {}
    set skip 0
    foreach e1 [lrange $l3 0 [expr {$len - 2}]] e2 [lrange $l3 1 end] {
        if {$skip} {
            set skip 0
        } elseif {$e1 == $e2} {
            set skip 1
        } else {
            lappend result $e1
        }
    }
    if {!$skip} {
        lappend result $e2
    }
    set result
}

proc set_union {args} {
    lsort -unique [eval concat $args]
}

proc set_difference {args} {
    # Symmetric set difference -- find elements
    # which do not belong to every set
    set intersection [eval set_intersection $args]
    set union [lsort -unique [eval concat $args]]
    if {[llength $intersection] == 0} {
        return $union
    }
    set l3 [lsort [concat $union $intersection]]
    set len [llength $l3]
    set result {}
    set skip 0
    foreach e1 [lrange $l3 0 [expr {$len - 2}]] e2 [lrange $l3 1 end] {
        if {$skip} {
            set skip 0
        } elseif {$e1 == $e2} {
            set skip 1
        } else {
            lappend result $e1
        }
    }
    if {!$skip} {
        lappend result $e2
    }
    set result
}

proc set_intersection {args} {
    set n_args [llength $args]
    if {$n_args < 2} {
        error "At least two arguments required"
    } elseif {$n_args > 2} {
        set therest [lrange $args 2 end]
        lappend therest [set_intersection [lindex $args 0] [lindex $args 1]]
        set result [eval set_intersection $therest]
    } else {
        set l1 [lsort -unique [lindex $args 0]]
        set l2 [lsort -unique [lindex $args 1]]
        set l3 [lsort [concat $l1 $l2]]
        set len [llength $l3]
        set result {}
        foreach e1 [lrange $l3 0 [expr {$len - 2}]] e2 [lrange $l3 1 end] {
            if {$e1 == $e2} {
                lappend result $e1
            }
        }
    }
    set result
}
