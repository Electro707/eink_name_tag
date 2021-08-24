# STM32 Firwamre Overview

## WORK-IN-PROGRESS


## EEPROM Organization

The EEPROM is organized so that the first 32 bytes are used for information like the the number of frames available. The rest of the EEPROM is used to store a frame. Each frame has 44 sections, with each section beign 64 bytes long. 

|Offset|Data|
|---|---|
|0|Info Data *(TODO: Document this)*|
|32|Frame 1's Section 0|
|96|Frame 1's Section 1|
|160|Frame 1's Section 2|
|...|...|
|2784|Frame 1's Section 43|
|2848|Frame 1's Section 44|
|2912|Frame 2's Section 0|
|2976|Frame 2's Section 1|
|...|...|
