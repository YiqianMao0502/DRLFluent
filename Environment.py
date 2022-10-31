import sys
import os
import shutil
cwd = os.getcwd()
sys.path.append(cwd + "/../")

from CircularCylinder import CircularCylinder
import numpy as np
import math

import os
cwd = os.getcwd()

nb_actuations = 100
simulation_duration = 2.0
dt=0.001

def env(port=10000,
               plot=False,
               step=50,
               dump=10,
               random_start=False,
               action_output=False):
    # ---------------------------------------------------------------------------------
    # the configuration version number 1

    state_params = {'nbr_probes': 151,
                     'probe_type': 'velocity'
                     }

    optimization_params = {"num_steps_in_pressure_history": 1,
                        "min_value_jet_MFR": -1e-2,
                        "max_value_jet_MFR": 1e-2,
                        "smooth_control": (nb_actuations/dt)*(0.1*0.0005/80),
                        "zero_net_Qs": True,
                        "random_start": random_start}

    inspection_params = {"plot": plot,
                        "step": step,
                        "dump": dump,
                        "action_output":action_output
                        }

    reward_function = 'drag_plain_lift'

    verbose = 2

    number_steps_execution = int((simulation_duration/dt)/nb_actuations)
    size_history = int(simulation_duration/dt/2)


    n_iter = None



    simu_name = 'Simu'

    if (geometry_params["jet_positions"][0] - 90) != 0:
        next_param = 'A' + str(geometry_params["jet_positions"][0] - 90)
        simu_name = '_'.join([simu_name, next_param])
    if geometry_params["cylinder_size"] != 0.01:
        next_param = 'M' + str(geometry_params["cylinder_size"])[2:]
        simu_name = '_'.join([simu_name, next_param])
    if optimization_params["max_value_jet_MFR"] != 0.01:
        next_param = 'maxF' + str(optimization_params["max_value_jet_MFR"])[2:]
        simu_name = '_'.join([simu_name, next_param])
    if nb_actuations != 80:
        next_param = 'NbAct' + str(nb_actuations)
        simu_name = '_'.join([simu_name, next_param])
    next_param = 'drag'
    if reward_function == 'recirculation_area':
        next_param = 'area'
    if reward_function == 'max_recirculation_area':
        next_param = 'max_area'
    elif reward_function == 'drag':
        next_param = 'last_drag'
    elif reward_function == 'max_plain_drag':
        next_param = 'max_plain_drag'
    elif reward_function == 'drag_plain_lift':
        next_param = 'lift'
    elif reward_function == 'drag_avg_abs_lift':
        next_param = 'avgAbsLift'
    simu_name = '_'.join([simu_name, next_param])

    env_ini = CircularCylinder(port=port,
                                    number_controllers=number_controllers,
                                    number_actions=number_actions,
                                    state_params=state_params,
                                    optimization_params=optimization_params,
                                    inspection_params=inspection_params,
                                    n_iter_make_ready=n_iter,  # On recalcule si besoin
                                    verbose=verbose,
                                    size_history=size_history,
                                    reward_function=reward_function,
                                    number_steps_execution=number_steps_execution,
                                    simu_name = simu_name)

    return(env_ini)
