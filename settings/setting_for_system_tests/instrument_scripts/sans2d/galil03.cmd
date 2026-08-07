GalilCreateController("Galil", "$(GALILADDR)", 20)

GalilCreateAxis("Galil","A",1,"",1)
GalilCreateAxis("Galil","B",1,"",1)
GalilCreateAxis("Galil","C",1,"",1)
GalilCreateAxis("Galil","D",1,"",1)
GalilCreateAxis("Galil","E",1,"",1)
GalilCreateAxis("Galil","F",1,"",1)
GalilCreateAxis("Galil","G",1,"",1)
GalilCreateAxis("Galil","H",1,"",1)

epicsEnvSet "HOME_HEADER" "$(GALIL)/gmc/galil_Default_Header.gmc"
epicsEnvSet "HOME_MODE_1" "$(GALIL)/gmc/galil_Home_Home.gmc"
epicsEnvSet "HOME_MODE_2" "$(GALIL)/gmc/galil_Home_Home.gmc"
epicsEnvSet "HOME_MODE_3" "$(GALIL)/gmc/galil_Home_Home.gmc"
epicsEnvSet "HOME_MODE_4" "$(GALIL)/gmc/galil_Home_Home.gmc"
epicsEnvSet "HOME_MODE_5" "$(GALIL)/gmc/galil_Home_ForwLimit.gmc"
epicsEnvSet "HOME_MODE_6" "$(GALIL)/gmc/galil_Home_RevLimit.gmc"
epicsEnvSet "HOME_MODE_7" "$(GALIL)/gmc/galil_Home_No_Home.gmc"
epicsEnvSet "HOME_MODE_8" "$(GALIL)/gmc/galil_Home_No_Home.gmc"
epicsEnvSet "HOME_FOOTER" "$(GALIL)/gmc/galil_Default_Footer.gmc"

GalilStartController("Galil","$(HOME_HEADER);$(HOME_MODE_1)!$(HOME_MODE_2)!$(HOME_MODE_3)!$(HOME_MODE_4)!$(HOME_MODE_5)!$(HOME_MODE_6)!$(HOME_MODE_7)!$(HOME_MODE_8);$(HOME_FOOTER)",0,0,3)

