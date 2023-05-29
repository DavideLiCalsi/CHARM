# CHARM
Counters Help Against Roving Malware

**CHARM** is a novel technique to enable interruptible Remote Attestation on low-end microcontrollers. This repo stores the codebase that was used to experimentally validate our approach.

## CHARM in a nuthsell
Many vendors include Hardware Performance Counters (HPC) in their devices. They often consist of few hardware registers that are incremented whenever some specific event occurs. Despite being useful for debugging, we claim that it is possible to use them to capture a **trace** of the microcontroller's activity. This trace can later be used to determine whether some malicious relocations occurred at attestation time. By feeding this information to a well-trained **Binary Classifier** on the Verifier's side we can detect relocations with high accuracy.

## 
