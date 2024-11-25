# Jadyn Osborne Lab 11 : 11/25/2024
# System Programming Lab 11 Multiprocessing
## Implementation
Only existing files were edited, all major changes occurred within `mandel.c`.
To implement parallel processing, the parent forks within a for loop according to the number 
of processes provided by the user (default:1).
Each process works independently and creates the images based on each processes'
calculated start and end times. (Certain processes will contain extra remainder calculations).
All `mandel*.jpg` images have numbered file names dependent on what image they are generating
within their given start and end range. Lastly, all files are numbered in correct order, so
compilation of the jpgs using `ffmpeg` will work as it increments through by decimal value.
## Graph of runtime results
![Lab11Runtime](https://github.com/user-attachments/assets/96e335d5-3ebb-465f-b276-381a51e14e99)
## Result Analysis
Results are as expected, quicker speeds start to flatline starting around 10 processes which is
the total number of cores in my computer model (2 performance & 8 efficiency). As the number of 
processes being created continues, the proportion of gains in speed between each additional
process follows a downward trend. This is due to the nature of overhead when creating parallel
processes, and when not increasing your calculation size, in this case 50 images, the overhead
starts to make more of a negative impact on performance gain as you use more cores. If they were
many more images, say 1000, using all 10 cores would likely prove very beneficial, whereas using
10 cores to generate only 2 images could prove more time consuming than using less. Taking longer
to finish executing code even though a computer is using more processes in parallel can occur
as the code is too short / not intensive to warrant the overhead and constant switching back and 
forth between cores if they are not needed.

