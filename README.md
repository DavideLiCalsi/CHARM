# CHARM
Counters Help Against Roving Malware

**CHARM** is a novel technique to enable interruptible Remote Attestation on low-end microcontrollers. This repo stores the codebase that was used to experimentally validate our approach.

## CHARM in a nuthsell
Many vendors include Hardware Performance Counters (HPC) in their devices. They often consist of few hardware registers that are incremented whenever some specific event occurs. Despite being useful for debugging, we claim that it is possible to use them to capture a **trace** of the microcontroller's activity. This trace can later be used to determine whether some **malicious relocations** occurred at attestation time. By feeding this information to a well-trained **Binary Classifier** on the Verifier's side we can detect relocations with high accuracy.

## Datasets
There are 16 datasets in total. You can find them in the ```dataset``` folder. Each of them corresponds to one combination of two parameters, namely **entropy level** (how unpredictable the benign applications running on your device are) and **activity level** (how many operations per unit of time the bening applications execute). Each dataset is a csv file labelled by the date in which it was acquired, and containts approximately 10k records.

## Models
We tested three Binary Classifiers, namely Logistic Regression (LR), Decision Trees (DT) and Support Vector Machine (SVM). The ```Code/models``` folders contains Python classes implementing these classifiers. A dedicated notebook contains code that trains and tests each model on all the datasets, to obtain the results that we report in our manuscript.

## Plot Generation
The test scores for each model and each preprocessing technique were gathered in the ```PlotGenerationKit/Results```. The ```PlotGenerationKit/Plots.ipynb``` notebook reads and analyzes those scores and generates the plots that you can find in our manuscript.
