//===---------------------------------------------------------------------===//
// AAP Simulator
//===---------------------------------------------------------------------===//

This directory forms the basis of a simulator based on the AAP backend. As
the backend is in preparation for upstream, simulator development is being
carried out in the lib/Target/AAPSimulator directory, to make patch generation
easier. It is intended in the future that at some point this will be moved
into the AAP Target directory, under a Simulator subdirectory.

There is also the corresponding tools/aap-run which is a objdump derivative
which uses the AAP Simulator library. The split between the tool and the
library is such that it should be possible to write a gdbserver using just the
AAP Simulator Library once complete.
