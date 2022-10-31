import numpy as np
import os
import pickle
import csv
from collections import Counter

#TensorForce
from tensorforce.environments import Environment

#omniORB
import CORBA
import AAS_CORBA

def found_invalid_values(to_check, abs_threshold=300):
    if type(to_check) is np.ndarray:
        bool_ret = np.isnan(to_check).any() or np.isinf(to_check).any() or (np.abs(to_check) > abs_threshold).any()
    else:
        bool_ret = found_invalid_values(np.array(to_check))

    return(bool_ret)



class RingBuffer():
    "A 1D ring buffer using numpy arrays"
    def __init__(self, length):
        self.data = np.zeros(length, dtype='f')
        self.index = 0

    def extend(self, x):
        "adds array x to ring buffer"
        x_index = (self.index + np.arange(x.size)) % self.data.size
        self.data[x_index] = x
        self.index = x_index[-1] + 1

    def get(self):
        "Returns the first-in-first-out data in the ring buffer"
        idx = (self.index + np.arange(self.data.size)) % self.data.size
        return self.data[idx]


# @printidc()
class Env2DCylinder(Environment):
    """Environment for 2D flow simulation around a cylinder."""

    def __init__(self, port, number_controllers, number_actions, state_params,
                 optimization_params, inspection_params, n_iter_make_ready=None, verbose=0, size_history=20,
                 reward_function='plain_drag', size_time_state=50, number_steps_execution=1, simu_name="Simu"):
        """

        """

        # TODO: should actually save the dicts in to double check when loading that using compatible simulations together

        #printi("--- call init ---")
        
        self.port=port
        if self.port == 10000:
            os.chdir('/scratch/m83358ym/Workingfolder/simulation_base/')
            self.cwd = os.getcwd()
            print('Init:cwd:'+str(self.cwd))
            print('example')
        else:
            os.chdir('/scratch/m83358ym/Workingfolder/env_' + str(self.port))
            self.cwd = os.getcwd()
            print('Init:cwd:'+str(self.cwd))

        orb = CORBA.ORB_init()
        with open('aaS_FluentId.txt', 'r') as f:
            aasFluentKey = f.read()
        self.fluentUnit = orb.string_to_object(aasFluentKey)
        self.schemeController = self.fluentUnit.getSchemeControllerInstance()
        
        self.observation = None

        self.flag_need_reset = True
        self.number_controllers=number_controllers
        self.number_actions=number_actions
        self.state_params = state_params
        self.optimization_params = optimization_params
        self.inspection_params = inspection_params
        self.verbose = verbose
        self.n_iter_make_ready = n_iter_make_ready
        self.size_history = size_history
        self.reward_function = reward_function
        self.size_time_state = size_time_state
        self.number_steps_execution = number_steps_execution
        self.dt = 0.001
        self.NumProbe = 151

        self.simu_name = simu_name

        self.list_save_states = []
        self.list_save_actions = []
        self.list_save_reward = []

        self.list_saved_solver_states = []


        #Relatif a l'ecriture des .csv
        name="output.csv"
        last_row = None
        if(os.path.exists("saved_models/"+name)):
            with open("saved_models/"+name, 'r') as f:
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

        self.initialized_visualization = False

        self.train_Re = [100, 200, 300, 400]
        self.train_mu = [1E-3, 5E-4, 3.3E-4, 2.5E-4] # do not change the order

        self.selected_Re = []

        # self.reward_Re = dict((Re, 0) for Re in self.train_Re)

        self.count = 0

        # self.batch_size = self.inspection_params["batch_size"]

        self.start_class(complete_reset=True)

        #printi("--- done init ---")

    def flow_evolve(self, Jet_Values, NumofTimeSteps):
        os.chdir(str(self.cwd))
        jet_numbers=self.number_controllers
        for curr_Jet in range(jet_numbers):
            self.schemeController.doMenuCommandToString('define/named-expression edit u' + str(curr_Jet)+ ' definition ' + str(Jet_Values[curr_Jet]) + ' q q q')
        NoTS = str(NumofTimeSteps)
        self.schemeController.doMenuCommandToString('/solve/dual-time-iterate ' + NoTS + ' 40')

    def Savecasedata(self, Simnuber):
        self.schemeController.doMenuCommandToString('file/write-case-data ' + str(Simnuber) + '/T1T2.cas')

    def Sample_Velocity(self, ts, NumProbe):
        os.chdir(str(self.cwd))
        #cwd = os.getcwd()
        #print('Sample:cwd:'+str(cwd))
        ts = ts + 1
        self.velocity_probes = np.zeros(2*NumProbe, dtype=float)
        self.sqvelx = np.zeros(NumProbe, dtype=float)
        self.sqvely = np.zeros(NumProbe, dtype=float)
        # update x-velocity data
        fname = 'sqvelx.out'
        with open(fname, 'r') as f:
            lines = f.readlines()
            last_line = lines[-1]
            ressqxa = list(filter(None, last_line.split(" ")))
            ressqxa=np.array(ressqxa,dtype = float)

        # update y-velocity data
        fname = 'sqvely.out'
        with open(fname, 'r') as f:
            lines = f.readlines()
            last_line = lines[-1]
            ressqya = list(filter(None, last_line.split(" ")))
            ressqya=np.array(ressqya,dtype = float)

        for crrt_probe in range(NumProbe):
            self.velocity_probes[2 * crrt_probe] = ressqxa[crrt_probe+1]
            self.velocity_probes[2 * crrt_probe+1] = ressqya[crrt_probe+1]

        return self.velocity_probes

    def Sample_Drag(self, ts):
        os.chdir(str(self.cwd))
        #cwd = os.getcwd()
        #print('Sample_Drag:cwd:'+str(cwd))
        fname = 'drag-rfile.out'
        with open(fname, 'r') as f:
            lines = f.readlines()
            last_line = lines[-1]
            resd = list(filter(None, last_line.split(" ")))
            tdrag = float(resd[1])
        return tdrag

    def Sample_Lift(self, ts):
        os.chdir(str(self.cwd))
        #cwd = os.getcwd()
        #print('Sample_Lift:cwd:'+str(cwd))
        fname = 'lift-rfile.out'
        with open(fname, 'r') as f:
            lines = f.readlines()
            last_line = lines[-1]
            resl = list(filter(None, last_line.split(" ")))
            tlift = float(resl[1])
        return tlift

    def start_class(self, complete_reset=True):
        os.chdir(str(self.cwd))
        if complete_reset == False:
            self.solver_step = 0

        else:
            print("******************************************************")
            print('New Episode')
            print("******************************************************")

            self.solver_step = 0
            self.accumulated_drag = 0
            self.accumulated_lift = 0

            self.initialized_output = False

            self.resetted_number_probes = False

            self.area_probe = None

            self.history_parameters = {}

            for crrt_jet in range(self.number_controllers):
                self.history_parameters["jet_{}".format(crrt_jet)] = RingBuffer(self.size_history)

            self.history_parameters["number_of_jets"] = self.number_controllers

            for crrt_probe in range(151):
                if self.state_params["probe_type"] == 'pressure':
                    self.history_parameters["probe_{}".format(crrt_probe)] = RingBuffer(self.size_history)
                elif self.state_params["probe_type"] == 'velocity':
                    self.history_parameters["probe_{}_u".format(crrt_probe)] = RingBuffer(self.size_history)
                    self.history_parameters["probe_{}_v".format(crrt_probe)] = RingBuffer(self.size_history)

            self.history_parameters["number_of_probes"] = 151

            self.history_parameters["drag"] = RingBuffer(self.size_history)
            self.history_parameters["lift"] = RingBuffer(self.size_history)
            self.history_parameters["recirc_area"] = RingBuffer(self.size_history)

            # ------------------------------------------------------------------------

            # ------------------------------------------------------------------------
            # if necessary, load initialization fields
            if self.n_iter_make_ready is None:
                if self.verbose > 0:
                    print("Load initial flow")

                self.fluentUnit.loadCase("TurbulentFlowControl.cas")
                self.fluentUnit.loadData("TurbulentFlowControl.dat")


                if self.verbose > 0:
                    print("Load buffer history")

                if not "number_of_probes" in self.history_parameters:
                    self.history_parameters["number_of_probes"] = 0

                if not "number_of_jets" in self.history_parameters:
                    self.history_parameters["number_of_jets"] = self.number_controllers
                    #printi("Warning!! The number of jets was not set in the loaded hdf5 file")

                if not "lift" in self.history_parameters:
                    self.history_parameters["lift"] = RingBuffer(self.size_history)
                    #printi("Warning!! No value for the lift founded")

                if not "recirc_area" in self.history_parameters:
                    self.history_parameters["recirc_area"] = RingBuffer(self.size_history)
                    #printi("Warning!! No value for the recirculation area founded")

                # if not the same number of probes, reset
                if not self.history_parameters["number_of_probes"] == 151:
                    for crrt_probe in range(151):
                        if self.state_params["probe_type"] == 'pressure':
                            self.history_parameters["probe_{}".format(crrt_probe)] = RingBuffer(self.size_history)
                        elif self.state_params["probe_type"] == 'velocity':
                            self.history_parameters["probe_{}_u".format(crrt_probe)] = RingBuffer(self.size_history)
                            self.history_parameters["probe_{}_v".format(crrt_probe)] = RingBuffer(self.size_history)

                    self.history_parameters["number_of_probes"] = 151

                    #printi("Warning!! Number of probes was changed! Probes buffer content reseted")

                    self.resetted_number_probes = True

            # ------------------------------------------------------------------------
            # Initialization of actuators
            self.Qs = np.zeros(self.number_controllers)
            self.action = np.zeros(self.number_controllers)


            # ------------------------------------------------------------------------
            # if necessary, make converge
            if self.n_iter_make_ready is not None:
                self.flow_evolve(self.Qs, 1)
                if self.verbose > 0:
                    print("Compute initial flow")

                for _ in range(self.n_iter_make_ready):
                    self.flow_evolve(self.Qs, 1)

                    self.probes_values = self.Sample_Velocity(self.dt, self.NumProbe)
                    self.drag = self.Sample_Drag(self.dt)
                    self.lift = self.Sample_Lift(self.dt)

                    self.write_history_parameters()
                    self.visual_inspection()
                    self.output_data()

                    self.solver_step += 1

            if self.n_iter_make_ready is not None:

                # save field data
                self.schemeController.doMenuCommandToString('file/write-case-data ' + str(Simnuber) + '_T1T2.cas')

                # save buffer dict
                with open('mesh/dict_history_parameters.pkl', 'wb') as f:
                    pickle.dump(self.history_parameters, f, pickle.HIGHEST_PROTOCOL)

            # ----------------------------------------------------------------------
            if self.n_iter_make_ready is None:
                #Start control from a randon phase of vortex shedding
                if self.optimization_params["random_start"]:
                    rd_advancement = np.random.randint(650)
                    for j in range(1):
                        self.flow_evolve(self.Qs, rd_advancement)
                    print("Simulated {} iterations before starting the control".format(1))

                self.flow_evolve(self.Qs, 1)

                self.probes_values = self.Sample_Velocity(self.dt, self.NumProbe)
                self.drag = self.Sample_Drag(self.dt)
                self.lift = self.Sample_Lift(self.dt)

                self.write_history_parameters()
                self.visual_inspection()
                self.output_data()

            # ----------------------------------------------------------------------
            # if necessary, fill the probes buffer
            #if self.resetted_number_probes:
            #    #printi("Need to fill again the buffer; modified number of probes")

            #    for _ in range(self.size_history):
            #        self.execute()

            # ----------------------------------------------------------------------
            # ready now

            self.ready_to_use = True

    def write_history_parameters(self):
        for crrt_jet in range(self.number_controllers):
            self.history_parameters["jet_{}".format(crrt_jet)].extend(self.Qs[crrt_jet])

        if self.state_params["probe_type"] == 'pressure':
            for crrt_probe in range(151):
                self.history_parameters["probe_{}".format(crrt_probe)].extend(self.probes_values[crrt_probe])
        elif self.state_params["probe_type"] == 'velocity':
            for crrt_probe in range(151):
                self.history_parameters["probe_{}_u".format(crrt_probe)].extend(self.probes_values[2 * crrt_probe])
                self.history_parameters["probe_{}_v".format(crrt_probe)].extend(self.probes_values[2 * crrt_probe + 1])

        self.history_parameters["drag"].extend(np.array(self.drag))
        self.history_parameters["lift"].extend(np.array(self.lift))

    def data_output(self):
        os.chdir(str(self.cwd))
        if self.solver_step % self.inspection_params["dump"] == 0 and self.inspection_params["dump"] < 10000:
            print("%s | Ep N: %4d, step: %4d, drag: %.4f, lift: %.4f"%(self.simu_name,
            self.episode_number,
            self.solver_step,
            self.history_parameters["drag"].get()[-1],
            self.history_parameters["lift"].get()[-1]))

        if "dump" in self.inspection_params:
            modulo_base = self.inspection_params["dump"]

            self.episode_drags = np.append(self.episode_drags, [self.history_parameters["drag"].get()[-1]])
            self.episode_lifts = np.append(self.episode_lifts, [self.history_parameters["lift"].get()[-1]])

            if(self.last_episode_number != self.episode_number and "action_output" in self.inspection_params and self.inspection_params["action_output"] == False):
                self.last_episode_number = self.episode_number
                avg_drag = np.average(self.episode_drags[len(self.episode_drags)//2:])
                avg_lift = np.average(self.episode_lifts[len(self.episode_lifts)//2:])
                name = "output.csv"
                if(not os.path.exists("saved_models")):
                    os.mkdir("saved_models")
                if(not os.path.exists("saved_models/"+name)):
                    with open("saved_models/"+name, "w") as csv_file:
                        spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                        spam_writer.writerow(["Episode", "AvgDrag", "AvgLift"])
                        spam_writer.writerow([self.last_episode_number, avg_drag, avg_lift])
                else:
                    with open("saved_models/"+name, "a") as csv_file:
                        spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                        spam_writer.writerow([self.last_episode_number, avg_drag, avg_lift])
                self.episode_drags = np.array([])
                self.episode_lifts = np.array([])

            if("action_output" in self.inspection_params and self.inspection_params["action_output"] == True):
                self.action_output()

                if (self.solver_step % modulo_base == 0):
                    if not self.initialized_output:
                        self.schemeController.doMenuCommandToString("/file export cgns " + str(self.solver_step) + " n velocity-magnitude ()")
                        self.initialized_output = True
                    self.schemeController.doMenuCommandToString("/file export cgns " + str(self.solver_step) + " n velocity-magnitude ()")


    def action_output(self):
        os.chdir(str(self.cwd))
        name = "action_output.csv"
        if(not os.path.exists("saved_models")):
            os.mkdir("saved_models")
        if(not os.path.exists("saved_models/"+name)):
            with open("saved_models/"+name, "w") as csv_file:
                spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                spam_writer.writerow(["Name", "Step"] + ["Jet" + str(v) for v in range(len(self.Qs))])
                spam_writer.writerow([self.simu_name, self.solver_step] + [str(v) for v in self.Qs.tolist()])
        else:
            with open("saved_models/"+name, "a") as csv_file:
                spam_writer=csv.writer(csv_file, delimiter=";", lineterminator="\n")
                spam_writer.writerow([self.simu_name, self.solver_step] + [str(v) for v in self.Qs.tolist()])
        return



    def __str__(self):
        print('')

    def close(self):
        self.ready_to_use = False

    def reset(self):
        os.chdir(str(self.cwd))
        if self.solver_step > 0:
            mean_accumulated_drag = self.accumulated_drag / self.solver_step

            if self.verbose > -1:
                print("mean accumulated drag on the whole episode: {}".format(mean_accumulated_drag))

        self.start_class(complete_reset=True)

        next_state = np.transpose(np.array(self.probes_values))
        if self.verbose > 0:
            print(next_state)


        if(not os.path.exists("saved_models")):
            os.mkdir("saved_models")

        self.episode_number += 1

        self.list_save_states = []
        self.list_save_actions = []
        self.list_save_reward = []

        if found_invalid_values(next_state):
            print("------- hit NaN in state in reset -------")
            self.flag_need_reset = True
            next_not_nan_state = self.reset()
            self.list_save_states.append(next_not_nan_state)
            return(next_not_nan_state)

        self.list_save_states.append(next_state)
        return(next_state)

    def execute(self, actions=None):
        os.chdir(str(self.cwd))
        try:
            action = actions

            if self.verbose > 1:
                print("--- call execute ---")

            if action is None:
                if self.verbose > -1:
                    print("carefull, no action given; by default, no jet!")

                nbr_actuators = self.number_controllers
                action = np.zeros((nbr_actuators, ))

            if self.verbose > 2:
                print(str(self.cwd)+'action:'+str(action))

            self.previous_action = self.action
            self.action = action

            # to execute several numerical integration steps
            for crrt_action_nbr in range(self.number_steps_execution):

                # try to force a continuous / smoothe(r) control
                if "smooth_control" in self.optimization_params:
                    self.Qs += self.optimization_params["smooth_control"] * (np.array(action) - self.Qs)
                else:
                    self.Qs = np.transpose(np.array(action))

                # impose a zero net Qs
                if "zero_net_Qs" in self.optimization_params:
                    if self.optimization_params["zero_net_Qs"]:
                        self.Qs = self.Qs - np.mean(self.Qs)

                # evolve one numerical timestep forward
                self.flow_evolve(self.Qs, 1)

                # displaying information that has to do with the solver itself
                self.visual_inspection()
                self.output_data()

                # we have done one solver step
                self.solver_step += 1

                # sample probes and drag
                self.probes_values = self.Sample_Velocity(self.dt, self.NumProbe)
                self.drag = self.Sample_Drag(self.dt)
                self.lift = self.Sample_Lift(self.dt)

                # write to the history buffers
                self.write_history_parameters()

                self.accumulated_drag += self.drag
                self.accumulated_lift += self.lift

                # kill already here if starts to NaN
                if found_invalid_values(np.array(self.probes_values)) or found_invalid_values(self.drag) or found_invalid_values(self.lift):
                    self.flag_need_reset = True
                    print("------- hit NaN in flow field variables in execute -------")
                    terminal = 2
                    reward = 0
                    # self.reward = reward
                    next_state = np.zeros(self.states()['shape'])

                    self.list_save_states.append(next_state)
                    self.list_save_actions.append(actions)
                    self.list_save_reward.append(reward)

                    return(next_state, terminal, reward)

            next_state = np.transpose(np.array(self.probes_values))

            if self.verbose > 2:
                print(next_state)

            terminal = False

            if self.verbose > 2:
                print(terminal)

            reward = self.compute_reward()

            if self.verbose > 2:
                print(reward)

            if self.verbose > 1:
                print("--- done execute ---")

            if found_invalid_values(next_state) or found_invalid_values(reward):
                self.flag_need_reset = True
                print("------- hit NaN in state or reward in execute -------")
                terminal = 2
                reward = 0
                next_state = np.zeros(self.states()['shape'])

            self.list_save_states.append(next_state)
            self.list_save_actions.append(actions)
            self.list_save_reward.append(reward)

            return(next_state, terminal, reward)


        except Exception as e:
            print(e)
            print("------- hit NaN in execute Exception -------")
            self.flag_need_reset = True

            terminal = 2
            reward = 0
            next_state = np.zeros(self.states()['shape'])

            self.list_save_states.append(next_state)
            self.list_save_actions.append(actions)
            self.list_save_reward.append(reward)

            return(next_state, terminal, reward)

    def compute_reward(self):
        if self.reward_function == 'drag_plain_lift':  # a bit dangerous, may be injecting some momentum
            avg_length = min(500, self.number_steps_execution-3)
            avg_drag = np.mean(self.history_parameters["drag"].get()[-avg_length:])
            avg_lift = np.mean(self.history_parameters["lift"].get()[-avg_length:])
            # return avg_drag + 0.2 - 0.2 * abs(avg_lift) + sum(value for value in self.reward_Re.values())
            return 1000 * (- avg_drag + 0.159 - 0.2 * abs(avg_lift))
            # return avg_drag + 0.159 - 0.2 * abs(avg_lift) # Hongwei Tang
        elif self.reward_function == 'max_plain_drag':  # a bit dangerous, may be injecting some momentum
            values_drag_in_last_execute = self.history_parameters["drag"].get()[-self.number_steps_execution:]
            return - (np.mean(values_drag_in_last_execute) + 0.159)
        else:
            raise RuntimeError("reward function {} not yet implemented".format(self.reward_function))

    def states(self):
        if self.state_params["probe_type"] == 'pressure':
            return dict(type='float',
                        shape=(151 * self.optimization_params["num_steps_in_pressure_history"], )
                        )

        elif self.state_params["probe_type"] == 'velocity':
            return dict(type='float',
                        shape=(2 * 151 * self.optimization_params["num_steps_in_pressure_history"], )
                        )

    def actions(self):
        # NOTE: we could also have several levels of dict in dict, for example:
        # return { str(i): dict(continuous=True, min_value=0, max_value=1) for i in range(self.n + 1) }

        return dict(type='float',
                    shape=(self.number_actions, ),
                    min_value=self.optimization_params["min_value_jet_MFR"],
                    max_value=self.optimization_params["max_value_jet_MFR"])

    def max_episode_timesteps(self):
        return None
