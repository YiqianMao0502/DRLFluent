import argparse
import os
import sys
import csv
import numpy as np
import time

from tensorforce.agents import Agent
from tensorforce.execution import ParallelRunner
from Environment import env, nb_actuations, simulation_duration, dt
from FluentProcess import FluentProcessKey

ap = argparse.ArgumentParser()
ap.add_argument("-s", "--Number_of_environments", required=True, help="number of environments", type=int)
ap.add_argument("-c", "--Number_of_controllers", required=True, help="number of controllers", type=int)
ap.add_argument("-a", "--Number_of_actions", required=True, help="number of actions", type=int)

args = vars(ap.parse_args())

number_envs = args["Number_of_environments"]
number_controllers=args["Number_of_controllers"]
number_actions=args["Number_of_actions"]

use_best_model = True

dump = 20
example_environment = env(port=10000, plot=False, step=100, dump=dump, number_controllers=number_controllers, number_actions=number_actions)


environments = []
for crrt_simu in range(number_envs):
    environments.append(resume_env(
        port=crrt_simu, plot=False, step=100, dump=dump
    ))
    time.sleep(1)
os.chdir('/scratch/m83358ym/Workingfolder')
cwd = os.getcwd()
print('cwd:'+str(cwd))

if use_best_model:
    evaluation_environment = environments.pop()
else:
    evaluation_environment = None

network = [dict(type='dense', size=512), dict(type='dense', size=512)]

learning_rate = 1e-3
decaying_lr = learning_rate


agent = Agent.create(
    # Agent + Environment
    agent='ppo',
    environment=example_environment,

    max_episode_timesteps=nb_actuations,
    # TODO: nb_actuations could be specified by Environment.max_episode_timesteps() if it makes sense...
    # Network
    network=network,
    # Optimization
    batch_size=5, learning_rate=decaying_lr, subsampling_fraction=0.2, optimization_steps=25,
    # Reward estimation
    likelihood_ratio_clipping=0.2,
    # TODO: gae_lambda=0.97 doesn't currently exist
    # Critic
    critic_network=network,
    critic_optimizer=dict(
        type='multi_step', num_steps=5,
        optimizer=dict(type='adam', learning_rate=1e-3)
    ),
    # Regularization
    entropy_regularization=0.01,
    # TensorFlow etc
    parallel_interactions=number_envs,
    saver=dict(directory=os.path.join(os.getcwd(), 'saver_data'), frequency=72000),  # the high value of the seconds parameter here is so that no erase of best_model
    recorder=dict(directory=os.path.join(os.getcwd(), 'recorder_data'), frequency=20)
)

agent.initialize()
cwd = os.getcwd()
print('cwd:'+str(cwd))

os.chdir('/scratch/m83358ym/Workingfolder')
cwd = os.getcwd()
print('cwd:'+str(cwd))

runner = ParallelRunner(
    agent=agent, environments=environments, evaluation_environment=evaluation_environment,
    save_best_agent=use_best_model
)

os.chdir('/scratch/m83358ym/Workingfolder')
cwd = os.getcwd()
print('cwd:'+str(cwd))

evaluation_folder = "env_" + str(number_envs - 1)
sys.path.append(cwd + evaluation_folder)

def evaluation_callback_1(r):
    if(not os.path.exists(evaluation_folder + "/saved_models/output.csv")):
        print("no output.csv file, check path\n")
        sys.exit()
    else:
        with open(evaluation_folder + "/saved_models/output.csv", 'r') as csvfile:
            data = csv.reader(csvfile, delimiter = ';')
            for row in data:
                lastrow = row
            avg_drag = float(lastrow[1])

    return avg_drag

half_epoch = int(simulation_duration/dt/dump/2) * (-1)

def evaluation_callback_2(r):
    os.chdir('/scratch/m83358ym/Workingfolder')
    cwd = os.getcwd()
    print('cwd:'+str(cwd))
    if(not os.path.exists(evaluation_folder + "/saved_models/debug.csv")):
        print("no debug.csv file, check path\n")
        sys.exit()
    else:
        debug_data = np.genfromtxt(evaluation_folder + "/saved_models/debug.csv", delimiter=";")
        debug_data = debug_data[1:,1:]
        avg_data = np.average(debug_data[half_epoch:], axis=0)
        avg_drag = avg_data[3]


    if np.isnan(avg_data).any() or np.isinf(avg_data).any():
        print("------- hit NaN in callback function -------")
        return(-100)

    return avg_drag

runner.run(
    num_episodes=600, max_episode_timesteps=nb_actuations, sync_episodes=False)
    #evaluation_callback=evaluation_callback_2
#)
runner.close()
