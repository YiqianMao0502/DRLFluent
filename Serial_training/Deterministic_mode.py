import os
import argparse
import numpy as np
import csv

from tensorforce.agents import Agent
from tensorforce.execution import ParallelRunner

from Fluent_server import Fluent_server

cwd = os.getcwd()
ap = argparse.ArgumentParser()
#Num of environements
ap.add_argument("-n", "--Number_of_servers", default=32, required=True, help="Number of Fluent sessions to connect", type=int)
#Time of a complete episode
ap.add_argument("-d", "--Episode_duration", default=2.0, required=True, help="Time of a single episode", type=float)
#Num of interactions per episode
ap.add_argument("-i", "--Max_interaction_timesteps", default=100, required=True, help="Number of maximum interaction timesteps per episode", type=int)
#Size of numerical time step
ap.add_argument("-t", "--Delta_t", default=0.001, required=True, help="numerical step size", type=float)
#Action space
ap.add_argument("-a", "--Action_shape", default=2, required=True, help="Number_of_jets", type=int)
ap.add_argument("-al", "--Action_lower_bound", default=-1e-2, required=True, help="Lower bound of action", type=float)
ap.add_argument("-au", "--Action_upper_bound", default=1e-2, required=True, help="Upper bound of action", type=float)
#State space
ap.add_argument("-s", "--State_shape", default=151, required=True, help="Number_of_probes", type=int)
#Display parameters
ap.add_argument("-d", "--Output_interval", default=20, required=True, help="Intervals of outputting interactions", type=int)

args = vars(ap.parse_args())

Numerical_steps_per_ac=int((args["Episode_duration"]/args["Delta_t"])/args["Max_interaction_timesteps"])

Interaction_parameters = {"Multi_envs":args["Number_of_servers"],
                          "Episode_duration": args["Episode_duration"],
                          "max_inter_steps": args["Max_interaction_timesteps"],
                          "Delta_t": args["Delta_t"],
                          "numerical_steps_per_ac": Numerical_steps_per_ac}

State_parameters = {"probe_type":"velocity",
                    "state_shape":["State_shape"]}

Control_parameters = {"action_shape":args["Action_shape"],
                      "min_value_jet_MFR": args["Action_lower_bound"],
                      "max_value_jet_MFR": args["Action_upper_bound"],
                      "zero_net_As": True,
                      "random_start": False}

Output_params = {"verbose": 0,
                 "Output_interval": args["Output_interval"],
                 "single_run":True}

Initial_params={"Num_iter_before_action": None}

reward_function = 'drag_plain_lift'

#n_iter = None


example_env = Fluent_server(port=0,
                            Interaction_parameters=Interaction_parameters,
                            State_parameters=State_parameters,
                            Control_parameters=Control_parameters,
                            Output_params=Output_params,
                            reward_function=reward_function)

deterministic = True

network = [dict(type='dense', size=512), dict(type='dense', size=512)]
os.chdir(cwd)
saver_restore = dict(directory="saver_data/", load="best-model")

agent = Agent.create(
    # Agent + Environment
    agent='ppo', environment=example_env, max_episode_timesteps=args["Max_interaction_timesteps"],
    # TODO: nb_actuations could be specified by Environment.max_episode_timesteps() if it makes sense...
    # Network
    network=network,
    # Optimization
    batch_size=40, learning_rate=1e-3, subsampling_fraction=0.2, optimization_steps=25,
    # Reward estimation
    likelihood_ratio_clipping=0.2, estimate_terminal=True,  # ???
    # TODO: gae_lambda=0.97 doesn't currently exist
    # Critic
    critic_network=network,
    critic_optimizer=dict(
        type='multi_step', num_steps=5,
        optimizer=dict(type='adam', learning_rate=1e-3)
    ),
    # Regularization
    #entropy_regularization=0.01,
    # TensorFlow etc
    parallel_interactions=1,
    saver=saver_restore,
)

# restore_directory = './saver_data/'
# restore_file = 'model-40000'
# agent.restore(restore_directory, restore_file)
# agent.restore()
agent.initialize()

os.chdir(cwd)

if(os.path.exists("saved_models/test_strategy.csv")):
    os.remove("saved_models/test_strategy.csv")

if(os.path.exists("saved_models/test_strategy_avg.csv")):
    os.remove("saved_models/test_strategy_avg.csv")

def one_run():
    print("start simulation")
    state = example_env.reset()
    example_env.render = True

    for k in range(6*nb_actuations):
        #environment.print_state()
        action = agent.act(state, deterministic=deterministic, independent=True)
        state, terminal, reward = example_env.execute(action)
    # just for test, too few timesteps
    # runner.run(episodes=10000, max_episode_timesteps=20, episode_finished=episode_finished)

if not deterministic:
    for _ in range(10):
        one_run()

else:
    one_run()

example_env.Savecasedata()
