        INTEGER    NPERM
       PARAMETER (NPERM=4)
       include 'nb_tf.inc'
        integer nb_channel2
         integer min_perm(1)
        parameter (nb_channel2=1)
        double precision perm_value(NPERM, nb_tf)
        double precision perm_error(NPERM,nb_tf)
        double precision perm_value_it(NPERM, nb_tf)
        double precision perm_error_it(NPERM, nb_tf)
        double precision tf_value_it(nb_tf)
        double precision tf_error_it(nb_tf)
        integer curr_perm, nb_point_by_perm(NPERM), perm_order(NPERM,nb_channel2)
        common/mw_perm_value/ perm_order,perm_value,
     & perm_error, nb_point_by_perm, curr_perm, min_perm
        common/mc_value_error/perm_value_it, perm_error_it, tf_value_it,tf_error_it

