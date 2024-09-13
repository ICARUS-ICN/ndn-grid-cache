Simulation Environment for Trying Cache Placement Strategies on a Grid-Like NDN Network
===

[![DOI](https://zenodo.org/badge/503401611.svg)](https://zenodo.org/doi/10.5281/zenodo.10912635)


Overview
---
This is an [ndnSIM][ndnSIM] simulation scenario for testing the behavior of an in-axis cache placement strategy in a grid-like NDN network.

The code is composed of several extension modules to the [ndnSIM] simulator and
an example basic scenario in file `ndn-static-gric.cc`. The behavior of the
in-axis placement strategy is described in the article [Cache Placement in an
NDN Based LEO Satellite Network
Constellation](https://doi.org/10.1109/TAES.2022.3227530).

Usage
---

    ndn-static-grid OPTIONS

### Options:

    - rate: Data rate of every link in the grid network.
    - delay: Link delay for every network link.
    - duration: Simulation length.
    - r: Number of rows in the grid.
    - c: Number of columns in the grid.
    - clients: Number of (randomly) placed clients.
    - cache: Size of the cache.
    - prefix: Output prefix for result files.
    - hcaches: Relative location of the caches in the horizontal axis relative to the producer.
    - vcaches: Relative location of the caches in the vertical axis relative to the producer.

---
### Legal:
Copyright ⓒ 2021–2022 Universidade de Vigo<br>
Author: Miguel Rodríguez Pérez <miguel@det.uvigo.gal>.<br>
This software is licensed under the GNU General Public License, version 3 (GPL-3.0) or later. For information see LICENSE.


Project PID2020-113240RB-I00 financed by MCIU/AEI/10.13039/501100011033.
![MCIU-AEI Logo](https://icarus.det.uvigo.es/assets/img/logo-mcin-aei.jpeg)

[ndnSim]: https://ndnsim.net
