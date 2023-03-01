from tensorforce.environments import Environment
import numpy as np
from threading import Thread
import os
import time
import csv

import CORBA
import AAS_CORBA


def found_invalid_values(to_check, abs_threshold=300):
    if type(to_check) is np.ndarray:
        bool_ret = np.isnan(to_check).any() or np.isinf(to_check).any() or (np.abs(to_check) > abs_threshold).any()
    else:
        bool_ret = found_invalid_values(np.array(to_check))

    return(bool_ret)


# @printidc()
class Fluent_server(Environment):

    def __init__(self, port, Interaction_parameters, State_parameters, Control_parameters, Output_params, Initial_params, reward_function='plain_drag'):
        """

        """
        self.port=port
        self.observation = None
        self.thread = None

        orb = CORBA.ORB_init()
        with open('env_' + str(self.port)+'/aaS_FluentId.txt', 'r') as f:
            aasFluentKey = f.read()
        self.fluentUnit = orb.string_to_object(aasFluentKey)
        self.schemeController = self.fluentUnit.getSchemeControllerInstance()
        print('Initenv_' + str(self.port))

        self.Interaction_parameters = Interaction_parameters
        self.State_parameters = State_parameters
        self.Control_parameters = Control_parameters
        self.Output_params = Output_params
        self.Initial_params=Initial_params
        self.jet_number = int(self.Control_parameters["action_shape"])
        self.probe_type = self.State_parameters["probe_type"]
        self.verbose = int(self.Output_params["verbose"])
        self.num_iter_before_action = self.Initial_params["Num_iter_before_action"]
        self.reward_function = reward_function
        self.number_steps_execution = int(self.Interaction_parameters["numerical_steps_per_ac"])
        self.dt = float(self.Interaction_parameters["Delta_t"])
        self.NumProbe = int(self.State_parameters["state_shape"])
        self.op_interval=int(self.Output_params["Output_interval"])

        self.epi_reward=[]
        self.epi_time=0
        self.time_start = 0
        self.armed_time_measurement = True

        self.start_function = 0
        self.end_function = 0
        self.crrt_time_function = 0
        self.epi_function_time = 0

        self.start_evolve = 0
        self.end_evolve = 0
        self.crrt_time_evolve = 0
        self.epi_evolve_time = 0


        #Related to writing Episode .csv
        name="RL_outputs.csv"
        last_row = None
        if(os.path.exists("env_" + str(self.port)+"/monitors/"+name)):
            with open("env_" + str(self.port)+"/monitors/"+name, 'r') as f:
                for row in reversed(list(csv.reader(f, delimiter=";", lineterminator="\n"))):
                    last_row = row
                    break
        if(not last_row is None):
            self.episode_number = int(last_row[0])
            self.last_episode_number = int(last_row[0])
        else:
            self.last_episode_number = 0
            self.episode_number = 0
        self.episode_drags = np.array([])
        self.episode_areas = np.array([])
        self.episode_lifts = np.array([])

        self.reset_flow(complete_reset=True)

        print("--- done init ---")

    def smooth_control(self, Pre_Jet_Values, Curr_Jet_Values, NoTS):
        flow_time=self.fluentUnit.getOutputParameterValueByName('flow-time-op')
        flow_time_step=int(flow_time/self.dt)
        self.fluentUnit.getOutputParameterValueByName('flow-time-op')
        self.fluentUnit.setInputParameterValueByName('starttimestep', int(flow_time_step))
        for curr_Jet in range(self.jet_number):
            self.fluentUnit.setInputParameterValueByName('astart'+str(curr_Jet), float(Pre_Jet_Values[curr_Jet]))
            self.fluentUnit.setInputParameterValueByName('aend'+str(curr_Jet), float(Curr_Jet_Values[curr_Jet]))
        self.schemeController.doMenuCommandToString('/solve/dual-time-iterate ' + str(NoTS) + ' 40')

    def flow_evolve(self, NoTS):
        for curr_Jet in range(self.jet_number):
            self.fluentUnit.setInputParameterValueByName('astart'+str(curr_Jet), 0)
            self.fluentUnit.setInputParameterValueByName('aend'+str(curr_Jet), 0)
        self.schemeController.doMenuCommandToString('/solve/dual-time-iterate ' + str(NoTS) + ' 40')

    def Savecasedata(self, Simnuber):
        self.schemeController.doMenuCommandToString('file/write-case-data ' + str(Simnuber) + '/T1T2.cas')

    def Sample_Velocity(self, NumProbe):
        self.velocity_probes = np.zeros(2*NumProbe, dtype=float)
        self.sqvelx = np.zeros(NumProbe, dtype=float)
        self.sqvely = np.zeros(NumProbe, dtype=float)
        # update x-velocity data
        fname = 'sqvelx.out'
        with open("env_" + str(self.port)+"/"+fname, 'r') as f:
            lines = f.readlines()
            last_line = lines[-1]
            ressqxa = list(filter(None, last_line.split(" ")))
            ressqxa=np.array(ressqxa,dtype = float)

        # update y-velocity data
        fname = 'sqvely.out'
        with open("env_" + str(self.port)+"/"+fname, 'r') as f:
            lines = f.readlines()
            last_line = lines[-1]
            ressqya = list(filter(None, last_line.split(" ")))
            ressqya=np.array(ressqya,dtype = float)

        #Todo: It's time consuming, use Numba
        for crrt_probe in range(NumProbe):
            self.velocity_probes[2 * crrt_probe] = ressqxa[crrt_probe+1]
            self.velocity_probes[2 * crrt_probe+1] = ressqya[crrt_probe+1]

        return self.velocity_probes

    def Sample_Drag(self, steps):
        tdrag= []
        fname = 'drag-rfile.out'
        with open("env_" + str(self.port)+"/"+fname, 'rb') as f:
            f.seek(-1024,2)
            lines = f.readlines()
            for crrt in range(steps):
                crrt_line = lines[-crrt-1].decode('utf-8')
                resd = list(filter(None, crrt_line.split(" ")))
                tdrag.append(float(resd[1]))
        return np.array(tdrag)

    def Sample_Lift(self, steps):
        tlift= []
        fname = 'lift-rfile.out'
        with open("env_" + str(self.port)+"/"+fname, 'rb') as f:
            f.seek(-1024,2)
            lines = f.readlines()
            for crrt in range(steps):
                crrt_line = lines[-crrt-1].decode('utf-8')
                resl = list(filter(None, crrt_line.split(" ")))
                tlift.append(float(resl[1]))
        return np.array(tlift)

    def reset_flow(self, complete_reset=True):
        if complete_reset == False:
            self.solver_step = 0
        else:

            self.solver_step = 0

            # Load initial flow field
            if self.num_iter_before_action is None:
                if self.verbose > 0:
                    print("Load initial flow")

                self.fluentUnit.loadCase("LaminarFlowControl_smooth_control_151probes.cas")
                self.fluentUnit.loadData("LaminarFlowControl_smooth_control_151probes.dat")

            # Iterates before the first action
            if self.num_iter_before_action is not None:
                self.flow_evolve(int(self.num_iter_before_action))
                if self.verbose > 0:
                    print("Compute initial flow")
                self.probes_values = self.Sample_Velocity(self.NumProbe)
                self.drag = self.Sample_Drag(self.number_steps_execution)
                self.lift = self.Sample_Lift(self.number_steps_execution)

                self.RL_outputs()
                self.solver_step += self.num_iter_before_action

                # save field data
                self.schemeController.doMenuCommandToString('file/write-case-data LaminarFlowControl_smooth_control_151probes.cas')

            #Start from a random initial state
            if self.Control_parameters["random_start"]:
                rd_advancement = np.random.randint(650)
                self.flow_evolve(rd_advancement)
                print("Simulated {} iterations before starting the control".format(rd_advancement))

                self.probes_values = self.Sample_Velocity(self.NumProbe)
                self.drag = self.Sample_Drag(self.number_steps_execution)
                self.lift = self.Sample_Lift(self.number_steps_execution)

                self.RL_outputs()

                self.solver_step += rd_advancement

            self.probes_values = self.Sample_Velocity(self.NumProbe)
            self.drag = self.Sample_Drag(self.number_steps_execution)
            self.lift = self.Sample_Lift(self.number_steps_execution)

            self.RL_outputs()

            # Zero jet flux in the beginning
            self.As = np.zeros(self.jet_number)
            self.pre_As = np.zeros(self.jet_number)

            self.ready_to_use = True


    def RL_outputs(self):
        #Saving the drag in the csv at the end of each episode
        self.episode_drags = np.append(self.episode_drags, self.drag[-self.number_steps_execution:])
        self.episode_lifts = np.append(self.episode_lifts, self.lift[-self.number_steps_execution:])

        if(self.last_episode_number != self.episode_number and self.Output_params["single_run"] == False):
            self.last_episode_number = self.episode_number
            avg_drag = np.average(self.episode_drags[len(self.episode_drags)//2:])
            avg_lift = np.average(self.episode_lifts[len(self.episode_lifts)//2:])
            name = "RL_outputs.csv"
            if(not os.path.exists("env_" + str(self.port)+"/monitors")):
                os.makedirs("env_" + str(self.port)+"/monitors", exist_ok=True)
            if(not os.path.exists("env_" + str(self.port)+"/monitors/"+name)):
                with open("env_" + str(self.port)+"/monitors/"+name, "w") as csv_file:
                    spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow(["Episode", "AvgDrag", "AvgLift"])
                    spam_writer.writerow([self.last_episode_number, avg_drag, avg_lift])
            else:
                with open("env_" + str(self.port)+"/monitors/"+name, "a") as csv_file:
                    spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow([self.last_episode_number, avg_drag, avg_lift])
            self.episode_drags = np.array([])
            self.episode_lifts = np.array([])
            ave_epi_reward = np.average(self.epi_reward)
            sum_epi_reward = np.sum(self.epi_reward)
            name = "Epi_reward.csv"
            if (not os.path.exists("Epi_total_reward")):
                os.makedirs("Epi_total_reward", exist_ok=True)
            if (not os.path.exists("Epi_total_reward/" + name)):
                with open("Epi_total_reward/" + name, "w") as csv_file:
                    spam_writer = csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow(["Episode", "Time", "Epi_total_reward", "Epi_ave_reward"])
                    spam_writer.writerow([
                        self.episode_number,
                        time.time(),
                        sum_epi_reward,
                        ave_epi_reward])
            else:
                with open("Epi_total_reward/" + name, "a") as csv_file:
                    spam_writer = csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow([
                        self.episode_number,
                        time.time(),
                        sum_epi_reward,
                        ave_epi_reward])
            self.epi_reward = []


        if self.solver_step % self.op_interval == 0:
            #Display on command line
            print("Ep N: %4d, step: %4d, drag: %.4f, lift: %.4f"%(self.episode_number,
            self.solver_step,
            self.drag[-1],
            self.lift[-1]))

        if (self.Output_params["single_run"] == True):
            ave_epi_reward = np.average(self.epi_reward)
            sum_epi_reward = np.sum(self.epi_reward)
            name = "Deterministic_outputs.csv"
            if (not os.path.exists("Deterministic_control")):
                os.makedirs("Deterministic_control", exist_ok=True)
            if (not os.path.exists("Deterministic_control/" + name)):
                with open("Deterministic_control/" + name, "w") as csv_file:
                    spam_writer = csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow(["Solver_step", "Reward", "As"])
                    spam_writer.writerow([
                        self.solver_step,
                        self.reward,
                        self.As])
            else:
                with open("Deterministic_control/" + name, "a") as csv_file:
                    spam_writer = csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow([
                        self.solver_step,
                        self.reward,
                        self.As])
            self.epi_reward = []
            if self.solver_step % self.op_interval == 0:
                self.schemeController.doMenuCommandToString("/file export cgns " + str(self.solver_step) + " n velocity-magnitude ()")

    def update_time_function_start(self):
        if self.armed_time_measurement:
            self.start_function = time.time()

    def update_time_function_end(self):
        if self.armed_time_measurement:
            self.end_function = time.time()
            self.crrt_time_function = self.end_function - self.start_function
            self.start_function = None
            self.epi_function_time += self.crrt_time_function

    def update_time_evolve_start(self):
        if self.armed_time_measurement:
            self.start_evolve = time.time()

    def update_time_evolve_end(self):
        if self.armed_time_measurement:
            self.end_evolve = time.time()
            self.crrt_time_evolve = self.end_evolve - self.start_evolve
            self.start_evolve = None
            self.epi_evolve_time += self.crrt_time_evolve

    def write_time_information(self):
        if self.armed_time_measurement:
            name = "saved_function_times.csv"
            os.makedirs("env_" + str(self.port)+"/saved_function_times", exist_ok=True)
            if(not os.path.exists("env_" + str(self.port)+"/saved_function_times/"+name)):
                with open("env_" + str(self.port)+"/saved_function_times/"+name, "w") as csv_file:
                    spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow(["Episode", 'End_numerical_time_step' "epi_time", "epi_function_time", "epi_evolve_time"])
                    spam_writer.writerow([self.episode_number, self.start_flow_time, self.epi_time, self.epi_function_time, self.epi_evolve_time])
            else:
                with open("env_" + str(self.port)+"/saved_function_times/"+name, "a") as csv_file:
                    spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                    spam_writer.writerow([self.episode_number, self.start_flow_time, self.epi_time, self.epi_function_time, self.epi_evolve_time])
            self.epi_function_time=0
            self.epi_evolve_time=0

    def __str__(self):
        # printi("Env2DCylinder ---")
        print('')

    def close(self):
        self.ready_to_use = False

    def reset(self):
        #update time
        self.start_flow_time=self.fluentUnit.getOutputParameterValueByName('flow-time-op')
        self.epi_time = time.time()-self.time_start
        self.time_start = time.time()
        self.write_time_information()
        self.update_time_function_start()

        #reset flow
        self.reset_flow(complete_reset=True)

        #Get state
        next_state = np.transpose(np.array(self.probes_values))
        if self.verbose > 1:
            print(next_state)

        if(not os.path.exists("env_" + str(self.port)+"/monitors")):
            os.mkdir("env_" + str(self.port)+"/monitors")

        self.episode_number += 1

        if found_invalid_values(next_state):
            print("------- hit NaN in state in reset -------")
            next_not_nan_state = self.reset()

            self.update_time_function_end()

            return(next_not_nan_state)

        self.update_time_function_end()

        return(next_state)

    def execute(self, actions=None):

        self.update_time_function_start()

        try:
            action = actions

            if self.verbose > 1:
                print("--- call execute ---")

            if action is None:

                print("carefull, no action given; by default, no jet!")

                nbr_jets = len(self.jet_number)
                action = np.zeros((nbr_jets, ))

            if self.verbose > 2:
                print("env_" + str(self.port)+':action:'+str(action))

            self.update_time_evolve_start()

            self.pre_As=self.As
            self.As = action
            # Impose a zero net As
            if "zero_net_As" in self.Control_parameters:
                if self.Control_parameters["zero_net_As"]:
                    self.As = self.As - np.mean(self.As)

            # Iterate for a number of numerical steps corresponding to a action step
            self.smooth_control(self.pre_As,self.As,self.number_steps_execution)
            self.solver_step += self.number_steps_execution

            # sample probes and drag
            self.probes_values = self.Sample_Velocity(self.NumProbe)
            self.drag = self.Sample_Drag(self.number_steps_execution)
            self.lift = self.Sample_Lift(self.number_steps_execution)


            # kill already here if starts to NaN
            if found_invalid_values(np.array(self.probes_values)) or found_invalid_values(self.drag) or found_invalid_values(self.lift):
                print("------- hit NaN in flow field variables in execute -------")
                terminal = 2
                reward = 0
                # self.reward = reward
                next_state = np.zeros(self.states()['shape'])

                self.update_time_evolve_end()

                return(next_state, terminal, reward)

            self.update_time_evolve_end()

            next_state = np.transpose(np.array(self.probes_values))

            terminal = False

            #Compute reward
            reward = self.compute_reward()
            self.reward=reward
            self.epi_reward = np.append(self.epi_reward, reward)
            # Output RL results
            self.RL_outputs()

            if self.verbose > 2:
                print(next_state)
                print(terminal)
                print(reward)

            if self.verbose > 1:
                print("--- done execute ---")

            if found_invalid_values(next_state) or found_invalid_values(reward):
                print("------- hit NaN in state or reward in execute -------")
                terminal = 2
                reward = 0
                next_state = np.zeros(self.states()['shape'])

            self.update_time_function_end()

            return(next_state, terminal, reward)

        except Exception as e:
            print(e)
            print("------- hit NaN in execute Exception -------")

            terminal = 2
            reward = 0
            next_state = np.zeros(self.states()['shape'])


            self.update_time_function_end()

            return(next_state, terminal, reward)

    def compute_reward(self):
        # NOTE: reward should be computed over the whole number of iterations in each execute loop
        if self.reward_function == 'plain_drag':
            values_drag_in_last_execute = self.drag[-self.number_steps_execution:]
            return(np.mean(values_drag_in_last_execute) + 0.159)  # TODO: the 0.159 value is a proxy value corresponding to the mean drag when no control; may depend on the geometry
        elif self.reward_function == 'drag':
            return self.drag[-1] + 0.159
        elif self.reward_function == 'drag_plain_lift':
            avg_length = min(500, self.number_steps_execution)
            avg_drag = np.mean(self.drag[(3-avg_length):]) # to avoid the numerical fluctuation
            avg_lift = np.mean(self.lift[(3-avg_length):])
            return - avg_drag + 0.159 - 0.2 * abs(avg_lift)
        elif self.reward_function == 'max_plain_drag':
            values_drag_in_last_execute = self.drag[-self.number_steps_execution:]
            return - (np.mean(values_drag_in_last_execute) + 0.159)
        elif self.reward_function == 'drag_avg_abs_lift':
            avg_length = min(500, self.number_steps_execution)
            avg_abs_lift = np.mean(np.absolute(self.drag[(3-avg_length):]))
            avg_drag = np.mean(self.drag[(3-avg_length):])
            return avg_drag + 0.159 - 0.2 * avg_abs_lift

        else:
            raise RuntimeError("reward function {} not yet implemented".format(self.reward_function))

    def states(self):
        if self.probe_type == 'pressure':
            return dict(type='float',
                        shape=(151, )
                        )

        elif self.probe_type == 'velocity':
            return dict(type='float',
                        shape=(2 * 151, )
                        )

    def actions(self):
        # NOTE: we could also have several levels of dict in dict, for example:
        # return { str(i): dict(continuous=True, min_value=0, max_value=1) for i in range(self.n + 1) }

        return dict(type='float',
                    shape=(self.jet_number, ),
                    min_value=self.Control_parameters["min_value_jet_MFR"],
                    max_value=self.Control_parameters["max_value_jet_MFR"])

    def max_episode_timesteps(self):
        return None
