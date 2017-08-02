Okapi
=====

Okapi is OpenMC wrapped as a MOOSE App that allows relatively easy coupling to any MOOSE-based application. Most of the development notes have been located in docs/manual.tex for coupling OpenMC, Buffalo (a lightweight surrogate for BISON), and MOON (Nek wrapped as a MOOSE App).

Okapi relies on specific versions of OpenMC, Nek, and MOOSE. A branch created by merging paulromano/c-api into xgitlab/zernike is needed for access to C-API routines to interface with C++ code and for interacting with functional expansion talleis, respectively. The branch of MOOSE required has my temporary construction and deconstruction classes that will be replaced by the Functional Expansion MOOSE module in the future. The branch of MOON must not have duplicate versions of classes in this temporary MOOSE branch. This will be cleaned up in the future..
