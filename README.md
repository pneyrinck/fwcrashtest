# fwcrashtest
This demonstrates a kernel panic crash bug on M1-based Macs when using Firewire.

1. Make sure the Mac has a firewire port. You can use a Apple manufactured adapter, for example.
2. Run this command line utiltiy and a panic will happen.

I have identified that a kernel panic happens related to the Firewire function CreatePseudoAddressSpace(..). This utility simply creates the address space and then releases it. The panic happens when it is released. 
