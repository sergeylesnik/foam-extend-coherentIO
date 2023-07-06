# Coherent I/O format for OpenFOAM
A snapshot of the development of the novel OpenFOAM I/O format "coherent". It
is implemented in reduced version of foam-extend 4.1 (mainly foam and
finiteVolume libraries).

The work on the coherent format has been carried out by

Sergey Lesnik, Wikki GmbH

Gregor Weiss, HLRS University of Stuttgart

Henrik Rusche, Wikki GmbH (original idea and architecture)

# Installation
For the reduced version the same steps apply as for foam-extend-4.1, see
https://openfoamwiki.net/index.php/Installation/Linux/foam-extend-4.1.
Additionally, libarchive is needed for successful compilation (not required for
the coherent format). On Ubuntu 20.04, it can be installed via

`sudo apt install libarchive-dev`

ADIOS2 is the currently used transport layer (managing writing data to storage)
for the coherent format. It will be downloaded and compiled during the
installation enabling the tools like "bpls" needed for viewing the binary data
stored in the .bp format.

# Usage
Check out the cavity3D case in the tutorials folder. An Allrun script is
provided.

# Acknowledgment
This application has been developed as part of the exaFOAM Project
https://www.exafoam.eu, which has received funding from the European
High-Performance Computing Joint Undertaking (JU) under grant agreement No
956416. The JU receives support from the European Union's Horizon 2020 research
and innovation programme and France, Germany, Italy, Croatia, Spain, Greece,
and Portugal.
