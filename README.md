# NBS2CMD
A simple tool for converting .nbs (Minecraft Note Block Studio) files to .mcfunction to generate note block songs. Designed for redberd小红's ingame music box (bilibili.com/video/BV13L411378B).

Drag and drop .nbs/.nbp files (multiple at once supported) onto the executable. A .mcfunction file with the same name will be created for each .nbs file; one .mcfunction file will be created for all .nbp files. Please refer to https://minecraft.fandom.com/wiki/Function_(Java_Edition) for how to use .mcfunction files. 

A maximum of 32 tracks (or "layer") is supported for each .nbs file; each .nbp files should only contain one track. Due to the "8 game tick" restriction in the music box hardware design, only one pitch can exist every 8 gt (or a bar of 4 "blocks" in NBS) for each track. If a track does not meet the above requirement, or is "locked" in NBS, it will NOT be translated. 

The names of chests the player will receive follow the layer names in NBS. Thus it is suggested that the layers are named for distinction. Minecraft also has some rules for function names. Check the function name if Minecraft won't load the files. 
