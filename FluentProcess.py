import numpy as np
import os

class FluentProcessKey():
    '''Point value of pressure at locations'''
    def __init__(self, number_envs):
        self.aasFluentKey=[]
        self.fluentUnit =[]
        self.schemeController=[]
        for crrt_simu in range(number_envs):
            with open('env_'+str()+'/aaS_FluentId.txt', 'r') as f:
                self.aasFluentKey.append(f.read())
            self.fluentUnit.append(orb.string_to_object(aasFluentKey))
            self.schemeController.append(fluentUnit.getSchemeControllerInstance())
            if crrt_simu == len(self.aasFluentKey):
                print('CurrentFluentKey:'+str(crrt_simu))
            else:
                print('CurrentFluentKey is wrong')
    def sample(self, ts): return ts



# ------------------------------------------------------------------------------
