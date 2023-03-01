<div align="center">
<!-- Title: -->
  <h1>DRLFluent</h1>

DRLFluent is a distributed co-simulation framework coupling a DRL API, <a href="https://github.com/tensorforce/tensorforce">TensorForce</a> and a general CFD solver, <a href="https://www.ansys.com/products/fluids/ansys-fluent">Ansys-Fluent</a> . This framework is mainly for large-scale RL tasks where the environments are fluid flow simulated by CFD means.
</div>

## Dependencies
### Reinforcement learning                                      ### CFD solver
|      Package     |     Version   |                   |      Software    |     Version   |
|:-----------------|--------------:|                   |:-----------------|--------------:|
| Python           |      3.7      |                   | Ansys-Fluent     |     ≥19.2     |
| TensorFlow       |      1.13     |
| TensorForce      |      0.5.0    |






### Communication
|      Software    |     Version   |
|:-----------------|--------------:|
| omniORB          |     4.2.4     |
| omniORBpy        |     4.2.4     |

The versions of the components listed above are stable versions used for the development of the test case (Applying DRL to active flow control), which can also meet general development needs. If a higher version of TensorForce is needed, the prerequisite tensorflow package and Python also need to be updated to match the version. If the Python version changes, please find the corresponding version of <a href="https://sourceforge.net/projects/omniorb/files/">omniORB</a> on sourceforge.net.

## Installation

```bash
git clone https://github.com/YiqianMao0502/DRLFluent.git
```


### HPC

```bash
git clone https://github.com/YiqianMao0502/DRLFluent.git
```
