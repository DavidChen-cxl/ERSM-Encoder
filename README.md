# NBS2CMD
A simple tool for converting .nbs (Minecraft Note Block Studio) files to .mcfunction to generate note block songs. Designed for redberd小红's ingame music box (bilibili.com/video/BV13L411378B).

.nbs文件到.mcfunction转换器，用于为redberd小红设计的编码红石音乐机生成指令，机器说明及存档见bilibili.com/video/BV13L411378B.


## How To Use  使用方法
Drag and drop .nbs/.nbp files (multiple at once supported) onto the executable. A .mcfunction file with the same name will be created for each .nbs file; one .mcfunction file will be created for all .nbp files. Please refer to https://minecraft.fandom.com/wiki/Function_(Java_Edition) for how to use .mcfunction files. 

拖拽.nbs或.nbp文件至可执行文件，将会为每个.nbs文件生成同名.mcfunction，并将所有.nbp文件整合为一个.mcfunction，可以同时输入多个文件. 关于mc function的用法，详见minecraft wiki函数页面https://minecraft.fandom.com/zh/wiki/%E5%87%BD%E6%95%B0%EF%BC%88Java%E7%89%88%EF%BC%89.


## Input Requirements  输入要求
A maximum of 32 non-empty tracks (or "layer") is supported for each .nbs file; each .nbp files should only contain one track. Due to the "8 game tick" restriction in the music box hardware design, only one pitch can exist every 8 gt (or a bar of 4 "blocks" in NBS) for each track. If a track does not meet the above requirement, or is "locked" in NBS, it will NOT be translated. 

输入的.nbs文件最多支持32条非空音轨，.nbp文件仅支持单轨. 由于编码音乐机本身的限制，每条音轨每8游戏刻（即NBS中每个4格小节）只能有一种音高. 不满足上述限制条件的音轨，或是NBS中“锁住”的音轨将不会被翻译.


## File Naming  命名规则
The names of chests the player will receive follow the layer names in NBS. Thus it is suggested that the layers are named for distinction. Minecraft also has some rules for function names. Check the function name if Minecraft won't load the files. 

指令生成的箱子名称即为NBS中各音轨名称，为方便辨别，建议在NBS中为有用的音轨命名. 不过注意，minecraft对函数和命名空间名称要求较为严格，建议全程使用无空格无特殊符号的英文小写字母命名音轨和文件. 
