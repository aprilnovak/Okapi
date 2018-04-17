Okapi
=====

Okapi is a "wrapping" of the open-source Monte Carlo code OpenMC that allows OpenMC to be run within the Multiphysics Object-Oriented Simulation Environment (MOOSE) framework. This wrapping essentially involves replacing key MOOSE methods that would ordinarily call libMesh finite element routines with calls to OpenMC subroutines. This wrapping allows OpenMC to be run as a MOOSE application, allowing relatively easy coupling to any MOOSE-based application (such as [BISON](https://bison.inl.gov/SitePages/Home.aspx) fuels performance, [RELAP-7](https://relap7.inl.gov/SitePages/Overview.aspx) system-level thermal-hydraulics, and many more) and non-MOOSE codes that have also been wrapped as MOOSE applications (such as [Nek5000](https://nek5000.mcs.anl.gov/) computational fluid dynamics).

Okapi has MOOSE and OpenMC as git submodules to ensure that the correct versions of OpenMC and MOOSE are used.

The `docs/papers` directory contains publications and presentations related to Okapi that are a good starting point for understanding the wrapping and coupling, while the `docs/notes` directory contains developer notes compiled throughout the development that may provide useful secondary information.

The `tests` directory contains several test problems illustrating OpenMC-Buffalo, OpenMC-Nek5000, and OpenMC-Buffalo-Nek5000 coupling examples, where [Buffalo](https://github.com/aprilnovak/Buffalo) is a lightweight surrogate for the BISON fuels performance code. The `examples/physor-paper` directory contains the input files used to obtain simulation results for the 2018 PHYSOR conference, with paper information available in `docs/papers/PHYSOR-2018`.

We welcome all contributors and users!
