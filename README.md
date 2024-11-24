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
Results are as expected, quicker speeds start to flatline starting around 10 processes
## Result Analysis

