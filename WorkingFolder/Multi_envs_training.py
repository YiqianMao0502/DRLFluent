import argparse
import os
import sys
import csv
import numpy as np
import time

from tensorforce.agents import Agent
from tensorforce.execution import ParallelRunner
from Fluent_server import Fluent_server


cwd = os.getcwd()
ap = argparse.ArgumentParser()
#Num of environements
ap.add_argument("-n", "--Number_of_servers", default=32, required=True, help="Number of Fluent sessions to connect", type=int)
#Time of a complete episode
ap.add_argument("-ed", "--Episode_duration", default=2.0, help="Time of a single episode", type=float)
#Num of interactions per episode
ap.add_argument("-i", "--Max_interaction_timesteps", default=100, help="Number of maximum interaction timesteps per episode", type=int)
#Size of numerical time step
ap.add_argument("-t", "--Delta_t", default=0.001, help="numerical step size", type=float)
#Action space
ap.add_argument("-a", "--Action_shape", default=2, help="Number_of_jets", type=int)
ap.add_argument("-al", "--Action_lower_bound", default=-1e-2, help="Lower bound of action", type=float)
ap.add_argument("-au", "--Action_upper_bound", default=1e-2, help="Upper bound of action", type=float)
#State space
ap.add_argument("-s", "--State_shape", default=151, help="Number_of_probes", type=int)
#Display parameters
ap.add_argument("-d", "--Output_interval", default=20, help="Intervals of outputting interactions", type=int)

args = vars(ap.parse_args())

Numerical_steps_per_ac=int((args["Episode_duration"]/args["Delta_t"])/args["Max_interaction_timesteps"])

Interaction_parameters = {"Multi_envs":args["Number_of_servers"],
                          "Episode_duration": args["Episode_duration"],
                          "max_inter_steps": args["Max_interaction_timesteps"],
                          "Delta_t": args["Delta_t"],
                          "numerical_steps_per_ac": Numerical_steps_per_ac}

State_parameters = {"probe_type":"velocity",
                    "state_shape":151}

Control_parameters = {"action_shape":args["Action_shape"],
                      "min_value_jet_MFR": args["Action_lower_bound"],
                      "max_value_jet_MFR": args["Action_upper_bound"],
                      "zero_net_As": True,
                      "random_start": False}

Output_params = {"verbose": 0,
                  "Output_interval": args["Output_interval"],
                  "single_run":False}

Initial_params={"Num_iter_before_action": None}

reward_function = 'drag_plain_lift'

#n_iter = None


example_env = Fluent_server(port=0,
                            Interaction_parameters=Interaction_parameters,
                            State_parameters=State_parameters,
                            Control_parameters=Control_parameters,
                            Output_params=Output_params,
                            Initial_params=Initial_params,
                            reward_function=reward_function)


environments = []
for num_process in range(args["Number_of_servers"]):
    environments.append(Fluent_server(
        port=num_process,
        Interaction_parameters=Interaction_parameters,
        State_parameters=State_parameters,
        Control_parameters=Control_parameters,
        Output_params=Output_params,
        Initial_params=Initial_params,
        reward_function=reward_function
    ))
    time.sleep(1)
os.chdir(cwd)

Best_agent=True

if Best_agent:
    evaluation_environment = environments.pop()
else:
    evaluation_environment = None

learning_rate = 1e-3
decaying_lr = learning_rate


agent = Agent.create(
    # Agent + Environment
    agent='ppo',
    environment=example_env,
    max_episode_timesteps=args["Max_interaction_timesteps"],
    # TODO: max_episode_timesteps can also be defined by max_episode_timesteps() in the Environment Class
    # Network
    network=[dict(type='dense', size=512), dict(type='dense', size=512)],
    # Optimization
    batch_size=20, learning_rate=learning_rate, subsampling_fraction=0.2, optimization_steps=25,
    # Reward estimation
    likelihood_ratio_clipping=0.2,
    # Critic network
    critic_network=[dict(type='dense', size=512), dict(type='dense', size=512)],
    critic_optimizer=dict(
        type='multi_step', num_steps=5,
        optimizer=dict(type='adam', learning_rate=1e-3)
    ),
    # Regularization
    entropy_regularization=0.01,
    # TensorFlow etc
    parallel_interactions=args["Number_of_servers"],
    saver=dict(directory=os.path.join(os.getcwd(), 'saver_data')),
    recorder=dict(directory=os.path.join(os.getcwd(), 'recorder_data'), frequency=20),
    summarizer=dict(
        directory='summary',
        # list of labels, or 'all'
        labels=['entropy', 'kl-divergence', 'loss', 'reward', "episode-reward", 'update-norm']
    ),
)

agent.initialize()


runner = ParallelRunner(
    agent=agent, environments=environments, evaluation_environment=evaluation_environment,
    save_best_agent=True
)


#evaluation_folder = "env_" + str(number_envs - 1)
#sys.path.append(cwd + evaluation_folder)


runner.run(
    num_episodes=400, max_episode_timesteps=args["Max_interaction_timesteps"], sync_episodes=False)

runner.close()
