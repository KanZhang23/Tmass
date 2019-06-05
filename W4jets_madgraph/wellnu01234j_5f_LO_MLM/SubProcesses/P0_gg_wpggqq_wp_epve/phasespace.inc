      include 'maxparticles.inc' 
      integer max_branches
      parameter (max_branches=max_particles-1)
      integer max_configs
      parameter (max_configs=3136)

c     channel position
      integer config_pos,perm_pos
      common /to_config/config_pos,perm_pos
        
        