C+-----------------------------------------------------------------------+
C|                         INCLUDE FOR MADWEIGHT                         |
C|                                                                       |
C|     Author: Pierre Artoisenet (UCL-CP3)                               |
C|             Olivier Mattelaer (UCL-CP3)                               |
C+-----------------------------------------------------------------------+
C|     This file is generated automaticly by MADWEIGHT-ANALYZER          |
C+-----------------------------------------------------------------------+     
        integer nb_vis_part
        parameter (nb_vis_part=5)
        integer nb_sol_config
        parameter (nb_sol_config=1)
        integer dim_phase_space
        parameter (dim_phase_space=20)
        integer nb_channel
        parameter (nb_channel=1)
        integer config_ordering(nb_sol_config)

	integer label        
	integer num_vis(nb_sol_config)
        integer vis_nb(nb_vis_part,nb_sol_config)	
        integer nb_block(nb_sol_config)
        integer num_propa(nb_sol_config)
        integer propa_max(max_branches,max_branches,nb_sol_config)
        integer propa_min(max_branches,max_branches,nb_sol_config)
        integer propa_cont(max_branches,nb_sol_config)
        integer var2random(dim_phase_space,nb_sol_config)


C+-----------------------------------------------------------------------+
C|                                                                       |
C|        ** Enlarged Contraint Sector global information **             |
C|                                                                       |
C|    Class: B                                                           |
C|    particle in ECS : 4(missing)    3(visible)                         |
C|    blob linked are generated by :                                     |
C|                                                                       |
C|                                                                       |
C+-----------------------------------------------------------------------+



        data num_vis(1) /5/
        data (vis_nb(label,1),label=1,5) /3,5,6,7,8/
        data nb_block(1) / 1/
       data (var2random(label, 1), label=1,20) / 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0/



        data num_propa(1) /1/
        data (propa_cont(label,1),label=1,1) /-1/
        data (propa_max(1,label,1),label=1,1) / 0/
        data (propa_min(1,label,1),label=1,3) / 4,3,0/


C+-----------------------------------------------------------------------+
C|                  ORDERING OF THE CONFIGURATION                        |
C+-----------------------------------------------------------------------+
C|     order from the most important to the least important              |
C|         0 means that the configuration is bypassed                    |
C+-----------------------------------------------------------------------+
        data (config_ordering(label),label=1,1) /1/
C+-----------------------------------------------------------------------+
