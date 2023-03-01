<div align="center">
<!-- Title: -->
  <h1>DRLFluent</h1>

DRLFluent is a distributed co-simulation framework coupling a DRL API, <a href="https://github.com/tensorforce/tensorforce">TensorForce</a> and a general CFD solver, <a href="https://www.ansys.com/products/fluids/ansys-fluent">Ansys-Fluent</a> . This framework is mainly for large-scale RL tasks where the environments are fluid flow simulated by CFD means.
</div>

## Dependencies
### Reinforcement learning
|      Package     |     Version   |
|:-----------------|--------------:|
| Python           |      3.7      |
| TensorFlow       |      1.13     |
| TensorForce      |      0.5.0    |

### CFD solver
|      Software    |     Version   |
|:-----------------|--------------:|
| Ansys-Fluent     |     ≥19.2     |

### Communication
|      Software    |     Version   |
|:-----------------|--------------:|
| omniORB          |     4.2.4     |
| omniORBpy        |     4.2.4     |

The versions of the components listed above are stable versions used for the development of the test case (Applying DRL to active flow control), which can also meet general development needs. If a higher version of TensorForce is needed, the prerequisite tensorflow package and Python also need to be updated to match the version. If the Python version changes, please find the corresponding version of <a href="https://sourceforge.net/projects/omniorb/files/">omniORB</a> on sourceforge.net.

## Download

```bash
git clone https://github.com/YiqianMao0502/DRLFluent.git
```

## Installation

## OmniORB

OmniORB installaztion on local machines (Windows system) can be refered to <a href="https://www.youtube.com/watch?v=v4eZPioTOYs">Python-Fluent AAS coupling</a> 

The omniORB v4.2.4 is included in the DRLFluent package. For other versions please download on <a href="https://sourceforge.net/projects/omniorb/files/">sourceforge.net</a>.

Copy the folder OmniORB to a long-term storage directory *Dir1*. Set environment variables by any of the following three ways.

1. Setting in .bashrc file. (recommended, set permanently on HPC)
2. Setting in the job script. (on HPC)
3. Setting mannually. (also applicable of installation on local machines)

The corresponding commands are listed below. Note that *Dir1* should be changed to the real directory of OmniORB.

```bash
export PYTHONPATH=$PYTHONPATH:*Dir1*/OmniORB/omni_inst/lib/python3.7/site-packages
export LD_LIBRARY_PATH=*Dir1*/OmniORB/omni_inst/lib
export PATH=$PATH:*Dir1*/OmniORB/omni_inst/bin
OMNINAMES_DATADIR=*Dir1*/OmniORB/wib/wob
export OMNINAMES_DATADIR
OMNIORB_CONFIG=*Dir1*/OmniORB/wib/wob/omniORB.cfg
export OMNIORB_CONFIG
```

Start omniNames

```bash
omniNames -start &
```

## Compile Corba interfaces (if using another version of OmniORB)

Collect CoFluentUnit.idl from the folder of Ansys-Fluent. 

On HPC (Linux system)

```bash
omniidl -I *Dir1*/OmniORB/omni_inst/share/idl/omniORB -bpython /mnt/iusers01/mace01/m83358ym/InterfaceFluentPython/Workingfolder/CoFluentUnit.idl
```

## Examples

Copy Working
